#include "bipartite.h"

/* ~~~~~~~ ******* BIPARTITE CLASS ******* ~~~~~~~ */

Bipartite::Bipartite(const char* filename, int type, bool random) : filename(filename), type(type), random(random)
{
	n_size = 0;
    n_top = 0;
    n_bot = 0;
    m = 0;
    duplicate_edge = 0;
    nodes = NULL;
    nodes_feature = NULL;
    list_neighbors = NULL;
}

Bipartite::~Bipartite()
{
	for(int i=0; i<n_size; i++)
    {
        if(nodes_feature[i].prox.neighbors != NULL)
            free(nodes_feature[i].prox.neighbors);
    }

    if(nodes != NULL)
        free(nodes);
    
    if(list_neighbors != NULL)
        free(list_neighbors);
}

void Bipartite::run(unsigned int limit)
{
    printf("\t\t[ Loading bipartite graph from '%s'... ]\n", filename);
    load_graphfile();
    if(random)
    {
        printf("\t\t[ Creating random bipartite graph (CM)... ]\n");
        configuration_model();
    }
    sort_neighbours();
    density = 1.0 * (m) / (n_top * n_bot);

    display_informations();
    
    printf("\t\t[ Computing proximity of each node (limit %i)... ]\n", limit);
    load_proximity(limit);

    printf("\t\t[ Computing metrics... ]\n", limit);
    compute_metrics();
    compute_global_metrics();
}

void Bipartite::load_graphfile()
{    
    static const int BUFFER_SIZE = 1024*1024;

    FILE* fd = fopen(filename, "r");
    if(!fd)
        handle_error("open");

    char line[BUFFER_SIZE];
    char* tmp;
    unsigned int index_node_top, index_node_bot, y, id_node;
    unsigned int* pt;
    unsigned int* list_neighbors_top;
    std::map<unsigned int, std::vector<unsigned int> > list_neighbors_bot;

    index_node_top = 0;
    
    fseek(fd, 0, SEEK_END);
    size_t t = ftell(fd) * sizeof(char);
    rewind(fd);
    unsigned int progression = 0;

    while(fgets(line, BUFFER_SIZE, fd))
    {                
        progression += strlen(line);
        printf("\rReading percentage %.3f%%", (progression / (1.0 * t)) * 100.0);
        fflush(stdout);
        
        tmp = strtok(line, " \t\n");
        //nodes = (unsigned int*) realloc(nodes, sizeof(unsigned int) * (n_top + 1));        
        //nodes_feature = (NodeFeature*) realloc(nodes_feature, sizeof(NodeFeature) * (n_top + 1));
        nodes = PyMem_Resize(nodes, unsigned int, n_top + 1);
        nodes_feature = PyMem_Resize(nodes_feature, NodeFeature, n_top + 1);

        if(type == 1)
        {
            id_node = atoi(tmp);
            tmp = strtok(NULL, " \t\n");
        }
        else
            id_node = index_node_top;
        nodes[index_node_top] = id_node;
        list_index_id_nodes_top.insert(std::pair<unsigned int, unsigned int>(id_node, index_node_top));

        list_neighbors_top = NULL;
        y = 0;
        while (tmp != NULL)
        {
            id_node = atoi(tmp);

            //list_neighbors_top = (unsigned int*) realloc(list_neighbors_top, sizeof(unsigned int) * (y + 1));
            list_neighbors_top = PyMem_Resize(list_neighbors_top, unsigned int, y + 1);

            list_neighbors_top[y] = id_node;
            list_neighbors_bot[id_node].push_back(nodes[index_node_top]);
            //std::sort(list_neighbors_bot[id_node].begin(), list_neighbors_bot[id_node].end());

            tmp = strtok(NULL, " \t\n");

            y++;
        }

        //list_neighbors = (unsigned int*) realloc(list_neighbors, sizeof(unsigned int) * (m + y));
        list_neighbors = PyMem_Resize(list_neighbors, unsigned int, m + y);
        memcpy(list_neighbors + m, list_neighbors_top, sizeof(unsigned int) * y);

        nodes_feature[index_node_top].degree = y;
        nodes_feature[index_node_top].is_top = true;
        nodes_feature[index_node_top].prox.n = 0;
        nodes_feature[index_node_top].prox.neighbors = NULL;

        n_top++;
        index_node_top++;
        m += y;
    }
    printf("\n");
    fclose(fd);

    n_size = n_top + list_neighbors_bot.size();
    n_bot = list_neighbors_bot.size();
    index_node_bot = index_node_top;
    
    //nodes = (unsigned int*) realloc(nodes, sizeof(unsigned int) * (n_size));
    //nodes_feature = (NodeFeature*) realloc(nodes, sizeof(NodeFeature) * (n_size));
    nodes = PyMem_Resize(nodes, unsigned int, n_size);
    nodes_feature = PyMem_Resize(nodes_feature, NodeFeature, n_size);

    size_t size;
    std::map<unsigned int, std::vector<unsigned int> >::iterator it;
    for(it = list_neighbors_bot.begin(); it != list_neighbors_bot.end(); ++it)
    {
        size = it->second.size();        
        id_node = it->first;

        //list_neighbors = (unsigned int*) realloc(list_neighbors, sizeof(unsigned int) * (m + size));
        list_neighbors = PyMem_Resize(list_neighbors, unsigned int, m + size);

        std::copy(it->second.begin(), it->second.end(), list_neighbors + m);
        
        nodes_feature[index_node_bot].degree = size;
        nodes_feature[index_node_bot].is_top = false;
        nodes_feature[index_node_bot].prox.n = 0;
        nodes_feature[index_node_bot].prox.neighbors = NULL;
        
        nodes[index_node_bot] = id_node;

        list_index_id_nodes_bot.insert(std::pair<unsigned int, unsigned int>(id_node, index_node_bot));

        m += size;
        index_node_bot++;
    }

    unsigned int s = 0;
    for(int i=0; i<n_size; i++)
    {
        nodes_feature[i].p_neighbors = list_neighbors + s;
        s += nodes_feature[i].degree;
    }

    /* Advise the kernel of our access pattern.  */
    //posix_fadvise(fd, 0, 0, 1);  // FDADVICE_SEQUENTIAL
}

void Bipartite::configuration_model()
{
    int count_top, count_bot, v, u, tmp, number_edges;
    number_edges = m / 2;

    unsigned int* list_repeated_nodes_top = (unsigned int*) malloc(sizeof(unsigned int) * number_edges);
    unsigned int* list_repeated_nodes_bot = (unsigned int*) malloc(sizeof(unsigned int) * number_edges);
    unsigned int* tmp_array;
    
    count_top = count_bot = 0;
    for(int i=0; i<n_size; i++)
    {
        tmp_array = (unsigned int*) malloc(sizeof(unsigned int) * nodes_feature[i].degree);

        for(int j=0; j<nodes_feature[i].degree; j++)
            tmp_array[j] = i;

        if(nodes_feature[i].is_top)
        {
            memcpy(list_repeated_nodes_top + count_top, tmp_array, sizeof(unsigned int) * nodes_feature[i].degree);
            count_top += nodes_feature[i].degree;
        }
        else
        {
            memcpy(list_repeated_nodes_bot + count_bot, tmp_array, sizeof(unsigned int) * nodes_feature[i].degree);
            count_bot += nodes_feature[i].degree;
        }

        
        nodes_feature[i].degree = 0;
        nodes_feature[i].p_neighbors = NULL;
    }

    srand(time(NULL));
    while(number_edges > 1)
    {        
        u = rand() % (number_edges - 1) + 0;
        v = rand() % (number_edges - 1) + 0;

        // swap  values of top
        tmp = list_repeated_nodes_top[number_edges - 1];
        list_repeated_nodes_top[number_edges - 1] = list_repeated_nodes_top[u];
        list_repeated_nodes_top[u] = tmp;
        
        // swap  values of bot
        tmp = list_repeated_nodes_bot[number_edges - 1];
        list_repeated_nodes_bot[number_edges - 1] = list_repeated_nodes_bot[v];
        list_repeated_nodes_bot[v] = tmp;

        number_edges--;        
    }    

    number_edges = m / 2;    
    unsigned new_m = 0;
    unsigned duplicated = 0;
    bool exist;
    for(int i=0; i<number_edges; i++)
    {
        exist = false;
        for(int y=0; y<nodes_feature[list_repeated_nodes_top[i]].degree; y++)
        {
            if(nodes_feature[list_repeated_nodes_top[i]].p_neighbors[y] == nodes[list_repeated_nodes_bot[i]])
            {
                exist = true;
                duplicated++;
                break;
            }
        }

        if(!exist)
        {            
            nodes_feature[list_repeated_nodes_top[i]].p_neighbors = (unsigned int*) realloc(nodes_feature[list_repeated_nodes_top[i]].p_neighbors, sizeof(unsigned int) * nodes_feature[list_repeated_nodes_top[i]].degree + 1);
            nodes_feature[list_repeated_nodes_top[i]].p_neighbors[nodes_feature[list_repeated_nodes_top[i]].degree] = nodes[list_repeated_nodes_bot[i]];
            nodes_feature[list_repeated_nodes_top[i]].degree++;
            new_m++;
        }
        
    }

    for(int i=0; i<number_edges; i++)
    {
        exist = false;
        for(int y=0; y<nodes_feature[list_repeated_nodes_bot[i]].degree; y++)
        {
            if(nodes_feature[list_repeated_nodes_bot[i]].p_neighbors[y] == list_repeated_nodes_top[i])
            {
                exist = true;
                break;
            }
        }

        if(!exist)
        {
            nodes_feature[list_repeated_nodes_top[i]].p_neighbors = (unsigned int*) realloc(nodes_feature[list_repeated_nodes_bot[i]].p_neighbors, sizeof(unsigned int) * nodes_feature[list_repeated_nodes_bot[i]].degree + 1);
            nodes_feature[list_repeated_nodes_bot[i]].p_neighbors[nodes_feature[list_repeated_nodes_bot[i]].degree] = nodes[list_repeated_nodes_top[i]];
            nodes_feature[list_repeated_nodes_bot[i]].degree++;
            new_m++;
        }
        
    }

    printf("Number of duplicated edges : %i\n", duplicated);
    m = new_m;
    duplicate_edge = duplicated;
}

void Bipartite::sort_neighbours()
{
    for(int i=0; i<n_size; i++)
        std::sort(nodes_feature[i].p_neighbors, nodes_feature[i].p_neighbors + nodes_feature[i].degree);
}

void Bipartite::load_proximity(unsigned int limit)
{
    char buf[1024];
    unsigned int nb_large_proximity, mem, nb_2big_proximity, memory_consumption;
    
    nb_2big_proximity = 0;

    for(unsigned int i=0; i<n_size; i++)
    {       
        printf("\rProceeding percentage %.3f%% (memory consumption : %s) ", (i / (1.0 * (n_size - 1))) * 100.0, readable_fs(memory_consumption, buf));
        fflush(stdout);
        mem = node_proximity(i, limit);
        memory_consumption += mem;
        
        if(mem == -1)
            nb_2big_proximity++;
        
    }
    printf("\n");
    printf("- Number of nodes having more than %i neighbors at distance 2 : %i\n", limit, nb_2big_proximity);
}

unsigned int Bipartite::node_proximity(unsigned int& index_node, unsigned int& limit)
{
    std::set<unsigned int> set_neighbours;
    unsigned int count_degree_gt1 = 0, index_neighbor;

    for(int y=0; y<nodes_feature[index_node].degree; y++)
    {
        if(nodes_feature[index_node].is_top)
            index_neighbor = list_index_id_nodes_bot.at(nodes_feature[index_node].p_neighbors[y]);
        else
            index_neighbor = list_index_id_nodes_top.at(nodes_feature[index_node].p_neighbors[y]);

        if(nodes_feature[index_neighbor].degree > 1)
            count_degree_gt1++;

        if(set_neighbours.size() >= limit)
            continue;
        else
        {
            for(int j=0; j<nodes_feature[index_neighbor].degree; j++)
                set_neighbours.insert(nodes_feature[index_neighbor].p_neighbors[j]);                
        }
    }

    set_neighbours.erase(nodes[index_node]);
    nodes_feature[index_node].prox.n = set_neighbours.size();
    // Nodes with degree greater than 1 (1-degree depreciated)
    nodes_feature[index_node].degree_gt1 = count_degree_gt1;
    
    if(set_neighbours.size() >= limit)
    {
        nodes_feature[index_node].prox.n = -1;
        return -1;
    }
    
    nodes_feature[index_node].prox.neighbors = (unsigned int*) malloc(sizeof(unsigned int) * set_neighbours.size());
    std::copy(set_neighbours.begin(), set_neighbours.end(), nodes_feature[index_node].prox.neighbors);

    return sizeof(unsigned int) * set_neighbours.size();
}

void Bipartite::save_projections(Repository* rep)
{    
    unsigned int index_neighbor;
    std::set<std::pair<unsigned int, unsigned int> > set_proximity;
    char* filename_projection_top;
    char* filename_projection_bot;    

    /* ** ** ** *** * TOP PROJECTION * ** ** ** *** * */  
    filename_projection_top = rep->build_filename(rep->root, "/projection_top");
    FILE* fd_project_top = fopen(filename_projection_top, "w");

    for(int i=0; i<n_top; i++)
    {
        for(int y=0; y<nodes_feature[i].degree; y++)
        {
            index_neighbor = list_index_id_nodes_bot.at(nodes_feature[i].p_neighbors[y]);

            for(int k=0; k<nodes_feature[index_neighbor].degree; k++)
            {                
                if(nodes[i] != nodes_feature[index_neighbor].p_neighbors[k])
                    set_proximity.insert(std::make_pair(nodes[i], nodes_feature[index_neighbor].p_neighbors[k]));
            }
        }

        for(std::set<std::pair<unsigned int, unsigned int> >::iterator it = set_proximity.begin(); it != set_proximity.end(); ++it)
            fprintf(fd_project_top, "%i %i\n", (*it).first, (*it).second);

        set_proximity.clear();
    }
    fclose(fd_project_top);
    free(filename_projection_top);

    /* ** ** ** *** * BOT PROJECTION * ** ** ** *** * */
    filename_projection_bot = rep->build_filename(rep->root, "/projection_bot");
    FILE* fd_project_bot = fopen(filename_projection_bot, "w");

    for(int i=n_top; i<n_size; i++)
    {
        for(int y=0; y<nodes_feature[i].degree; y++)
        {
            index_neighbor = list_index_id_nodes_top.at(nodes_feature[i].p_neighbors[y]);
            
            for(int k=0; k<nodes_feature[index_neighbor].degree; k++)
            {
                if(nodes[i] != nodes_feature[index_neighbor].p_neighbors[k])
                    set_proximity.insert(std::make_pair(nodes[i], nodes_feature[index_neighbor].p_neighbors[k]));
            }
        }

        for(std::set<std::pair<unsigned int, unsigned int> >::iterator it = set_proximity.begin(); it != set_proximity.end(); ++it)
            fprintf(fd_project_bot, "%i %i\n", (*it).first, (*it).second);

        set_proximity.clear();
    }
    fclose(fd_project_bot);
    free(filename_projection_bot);
}

void Bipartite::display_informations() const
{
    char buf[1024];

    printf("**** INFORMATIONS *****\n");
    printf("** \tNUMBER NODES : %s\n", readable_nb(n_size, buf));
    printf("** \tNUMBER NODES TOP : %s\n", readable_nb(n_top, buf));
    printf("** \tNUMBER NODES BOT : %s\n", readable_nb(n_bot, buf));
    printf("** \tDENSITY : %f\n\n", density);
}


void Bipartite::compute_global_metrics()
{
    float cumul_cc_top, cumul_cc_bot, cumul_rc_top, cumul_rc_bot;
    cumul_cc_top = cumul_cc_bot = cumul_rc_top = cumul_rc_bot = 0.0;

    for(int i=0; i<n_top; i++)
    {
        if(nodes_feature[i].cc >= 0)
            cumul_cc_top += nodes_feature[i].cc;
        if(nodes_feature[i].rc >= 0)
            cumul_rc_top += nodes_feature[i].rc;
    }
    for(int i=n_top; i<n_size; i++)
    {
        if(nodes_feature[i].cc >= 0)
            cumul_cc_bot += nodes_feature[i].cc;
        if(nodes_feature[i].rc >= 0)
            cumul_rc_bot += nodes_feature[i].rc;
    }

    global_cc_top = 1.0 * cumul_cc_top / n_top;
    global_cc_bot = 1.0 * cumul_cc_bot / n_bot;
    global_rc_top = 1.0 * cumul_rc_top / n_top;
    global_rc_bot = 1.0 * cumul_rc_bot / n_bot;
}

void Bipartite::compute_metrics()
{
    unsigned int index_neighbor, count_dispersion, n_proximity, count_degree_community;
    float v, v_min, v_max;
    std::vector<unsigned int> s1, buf_s1, s2;
    std::set<unsigned int> s3;

    for(unsigned int i=0; i<n_size; i++)
    {
        printf("\rProceeding percentage %.3f%%", (i / (1.0 * (n_size - 1))) * 100.0);
        fflush(stdout);

        // Initialisation of metric values          
        nodes_feature[i].cc = -1.0;
        nodes_feature[i].cc_min = -1.0;
        nodes_feature[i].cc_max = -1.0;
        nodes_feature[i].rc = -1.0;
        nodes_feature[i].avg_size_community = 0.0;
        nodes_feature[i].monopole = 0.0;
        nodes_feature[i].dispersion = 0.0;

        if(nodes_feature[i].degree == 0)
            continue;

        // ############## COMPUTATION MONOPOLE ############## //
        nodes_feature[i].monopole = 1.0 * (nodes_feature[i].degree - nodes_feature[i].degree_gt1) / (nodes_feature[i].degree);

        n_proximity = nodes_feature[i].prox.n;
        if(n_proximity < 1)
        {
            if(n_proximity == -1)
                compute_metrics_without_proximity(i);

            continue;
        }

        count_dispersion = 0;
        count_degree_community = 0;
        v = v_min = v_max = 0.0;

        for(int y=0; y<nodes_feature[i].degree; y++)
        {
            if(nodes_feature[i].is_top)
                index_neighbor = list_index_id_nodes_bot.at(nodes_feature[i].p_neighbors[y]);
            else
                index_neighbor = list_index_id_nodes_top.at(nodes_feature[i].p_neighbors[y]);

            count_dispersion += (nodes_feature[index_neighbor].degree - 1);
        }
        
        //s1(nodes[i].p_neighbors, nodes[i].p_neighbors + nodes[i].degree);
        s1.resize(nodes_feature[i].degree);
        buf_s1.resize(nodes_feature[i].degree);
        memcpy(&s1[0], nodes_feature[i].p_neighbors, sizeof(unsigned int) * nodes_feature[i].degree);
        memcpy(&buf_s1[0], nodes_feature[i].p_neighbors, sizeof(unsigned int) * nodes_feature[i].degree);
                    
        for(int y=0; y<n_proximity; y++)
        {
            if(nodes_feature[i].is_top)
                index_neighbor = list_index_id_nodes_top.at(nodes_feature[i].prox.neighbors[y]);
            else
                index_neighbor = list_index_id_nodes_bot.at(nodes_feature[i].prox.neighbors[y]);

            count_degree_community += nodes_feature[index_neighbor].degree;

            s2.resize(nodes_feature[index_neighbor].degree);
            memcpy(&s2[0], nodes_feature[index_neighbor].p_neighbors, sizeof(unsigned int) * nodes_feature[index_neighbor].degree);

            // INTERSECTION (JACKARD)
            std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), inserter(s3, s3.begin()));

            v += 1.0 * (s3.size()) / (nodes_feature[i].degree + nodes_feature[index_neighbor].degree - s3.size());
            v_min += 1.0 * (s3.size()) / (min(nodes_feature[i].degree, nodes_feature[index_neighbor].degree));
            v_max += 1.0 * (s3.size()) / (max(nodes_feature[i].degree, nodes_feature[index_neighbor].degree));

            if(!buf_s1.empty() && s3.size() > 1)
            {
                for(std::vector<unsigned int>::iterator it = buf_s1.begin(); it != buf_s1.end(); ++it)
                {
                    if(s3.find(*it) != s3.end())
                    {
                        buf_s1.erase(it);
                        --it;                        
                    }
                }
            }

            s2.clear();
            s3.clear();
        }
        
        // ############## COMPUTATION DISPERSION ############## //
        if(count_dispersion > 0)
            nodes_feature[i].dispersion = 1.0 * (n_proximity) / (count_dispersion);            

        // ############## COMPUTATION CC ############## //
        nodes_feature[i].cc = 1.0 * (v) / (n_proximity);
        nodes_feature[i].cc_min = 1.0 * (v_min) / (n_proximity);
        nodes_feature[i].cc_max = 1.0 * (v_max) / (n_proximity);

        // ############## COMPUTATION RC ############## //
        if(nodes_feature[i].degree == 1)
            nodes_feature[i].rc = 0.0;
        else
            nodes_feature[i].rc = 1.0 * (2 * (s1.size() - buf_s1.size())) / (nodes_feature[i].degree * (nodes_feature[i].degree - 1));

        // ############## COMPUTATION AVG SIZE COMMUNITY ############## //
        if(nodes_feature[i].is_top)
            nodes_feature[i].avg_size_community = 1.0 * (count_degree_community / n_proximity);
        else
            nodes_feature[i].avg_size_community = 1.0 * (count_dispersion + nodes_feature[i].degree) / nodes_feature[i].degree;            

        s1.clear();
        buf_s1.clear();

    }
    printf("\n");
}

void Bipartite::compute_metrics_without_proximity(unsigned int& index_node)
{
    unsigned int index_neighbor, index_h, count_dispersion, n_proximity, count_degree_community;
    float v, v_min, v_max;
    std::vector<unsigned int> s1, buf_s1, s2;
    std::set<unsigned int> s3, proximity;

    count_dispersion = 0;
    count_degree_community = 0;
    v = v_min = v_max = 0.0;

    s1.resize(nodes_feature[index_node].degree);
    buf_s1.resize(nodes_feature[index_node].degree);
    memcpy(&s1[0], nodes_feature[index_node].p_neighbors, sizeof(unsigned int) * nodes_feature[index_node].degree);
    memcpy(&buf_s1[0], nodes_feature[index_node].p_neighbors, sizeof(unsigned int) * nodes_feature[index_node].degree);

    for(int h=0; h<nodes_feature[index_node].degree; h++)
    {
        if(nodes_feature[index_node].is_top)
            index_h = list_index_id_nodes_bot.at(nodes_feature[index_node].p_neighbors[h]);
        else
            index_h = list_index_id_nodes_top.at(nodes_feature[index_node].p_neighbors[h]);

        count_dispersion += (nodes_feature[index_h].degree - 1);

        for(int j=0; j<nodes_feature[index_h].degree; j++)
        {
            if(nodes_feature[index_h].is_top)
                index_neighbor = list_index_id_nodes_bot.at(nodes_feature[index_h].p_neighbors[j]);
            else
                index_neighbor = list_index_id_nodes_top.at(nodes_feature[index_h].p_neighbors[j]);

            if(index_node == index_neighbor)
                continue;                
            if(proximity.size() > 0 && proximity.find(index_neighbor) != proximity.end())
                continue;
            proximity.insert(index_neighbor);
            
            count_degree_community += nodes_feature[index_neighbor].degree;

            s2.resize(nodes_feature[index_neighbor].degree);
            memcpy(&s2[0], nodes_feature[index_neighbor].p_neighbors, sizeof(unsigned int) * nodes_feature[index_neighbor].degree);
                
            // INTERSECTION (JACKARD)
            std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), inserter(s3, s3.begin()));

            v += 1.0 * (s3.size()) / (nodes_feature[index_node].degree + nodes_feature[index_neighbor].degree - s3.size());
            v_min += 1.0 * (s3.size()) / (min(nodes_feature[index_node].degree, nodes_feature[index_neighbor].degree));
            v_max += 1.0 * (s3.size()) / (max(nodes_feature[index_node].degree, nodes_feature[index_neighbor].degree));

            if(!buf_s1.empty() && s3.size() > 1)
            {
                for(std::vector<unsigned int>::iterator it = buf_s1.begin(); it != buf_s1.end(); ++it)
                {
                    if(s3.find(*it) != s3.end())
                    {
                        buf_s1.erase(it);
                        --it;
                    }
                }
            }

            s2.clear();
            s3.clear();
        }

        n_proximity = proximity.size();
        if(n_proximity == 0)
            continue;

        // ############## COMPUTATION AVG SIZE COMMUNITY ############## //
        if(nodes_feature[index_node].is_top)
            nodes_feature[index_node].avg_size_community = 1.0 * (count_degree_community / n_proximity);
        else
            nodes_feature[index_node].avg_size_community = 1.0 * (count_dispersion + nodes_feature[index_node].degree) / nodes_feature[index_node].degree;


        // ############## COMPUTATION DISPERSION ############## //
        if(count_dispersion > 0)
            nodes_feature[index_node].dispersion = 1.0 * (n_proximity) / (count_dispersion);            

        // ############## COMPUTATION CC ############## //
        nodes_feature[index_node].cc = 1.0 * (v) / (n_proximity);
        nodes_feature[index_node].cc_min = 1.0 * (v_min) / (n_proximity);
        nodes_feature[index_node].cc_max = 1.0 * (v_max) / (n_proximity);

        // ############## COMPUTATION RC ############## //
        if(nodes_feature[index_node].degree == 1)
            nodes_feature[index_node].rc = 0.0;
        else
            nodes_feature[index_node].rc = 1.0 * (2 * (s1.size() - buf_s1.size())) / (nodes_feature[index_node].degree * (nodes_feature[index_node].degree - 1));

        nodes_feature[index_node].prox.n = n_proximity;
    }
}

void Bipartite::save_bipartite(const char* filegraph)
{
    FILE* fd_graph = fopen(filegraph, "w");

    fprintf(fd_graph, "#IDENTIFIER : %i\n", type);
    if(random)
        fprintf(fd_graph, "#RANDOM : yes\n");
    else
        fprintf(fd_graph, "#RANDOM : no\n");

    for(int i=0; i<n_top; i++)
    {
        if(type)
            fprintf(fd_graph, "%i ", nodes[i]);
        for(int y=0; y<nodes_feature[i].degree; y++)
            fprintf(fd_graph, "%i ", nodes_feature[i].p_neighbors[y]);
        fprintf(fd_graph, "\n");
    }

    fclose(fd_graph);
}

void Bipartite::save_nodes(Repository* rep)
{
    char* top;
    char* bot;
    top = rep->build_filename(rep->root, "/top.nodes");
    bot = rep->build_filename(rep->root, "/bot.nodes");

    FILE* fd_top = fopen(top, "w");
    FILE* fd_bot = fopen(bot, "w");

    for(int i=0; i<n_top; i++)
        fprintf(fd_top, "%i\n", nodes[i]);
    for(int i=n_top; i<n_size; i++)
        fprintf(fd_bot, "%i\n", nodes[i]);

    fclose(fd_top);
    fclose(fd_bot);

    free(top);
    free(bot);
}

void Bipartite::save_metrics(Repository* rep)
{
    FILE* buf;
    FILE* fd_top;
    FILE* fd_bot;

    for(int i=0; i<n_metrics * 2; i++)
    {
        fd_top = rep->list_files_metric[i];
        fd_bot = rep->list_files_metric[i + 1];

        switch(i)
        {
            case 0: // CC

                for(int y=0; y<n_size; y++)
                {
                    buf = fd_bot;
                    if(nodes_feature[y].is_top)
                        buf = fd_top;
                        
                    fprintf(buf, "%i %f\n", nodes[y], nodes_feature[y].cc);
                }
                break;

            case 2: // RC

                for(int y=0; y<n_size; y++)
                {
                    buf = fd_bot;
                    if(nodes_feature[y].is_top)
                        buf = fd_top;

                    fprintf(buf, "%i %f\n", nodes[y], nodes_feature[y].rc);
                }
                break;

            case 4: // DEGREE

                for(int y=0; y<n_size; y++)
                {
                    buf = fd_bot;
                    if(nodes_feature[y].is_top)
                        buf = fd_top;
                    fprintf(buf, "%i %i\n", nodes[y], nodes_feature[y].degree);
                }
                break;

            case 6: // CC_MIN

                for(int y=0; y<n_size; y++)
                {
                    buf = fd_bot;
                    if(nodes_feature[y].is_top)
                        buf = fd_top;
                    fprintf(buf, "%i %f\n", nodes[y], nodes_feature[y].cc_min);
                }
                break;

            case 8: // CC_MAX

                for(int y=0; y<n_size; y++)
                {
                    buf = fd_bot;
                    if(nodes_feature[y].is_top)
                        buf = fd_top;
                    fprintf(buf, "%i %f\n", nodes[y], nodes_feature[y].cc_max);
                }
                break;

            case 10: // AVG_SIZE_COM
                for(int y=0; y<n_size; y++)
                {
                    buf = fd_bot;
                    if(nodes_feature[y].is_top)
                        buf = fd_top;
                    fprintf(buf, "%i %f\n", nodes[y], nodes_feature[y].avg_size_community);
                }
                break;

            case 12: // DISPERSION

                for(int y=0; y<n_size; y++)
                {
                    buf = fd_bot;
                    if(nodes_feature[y].is_top)
                        buf = fd_top;
                    fprintf(buf, "%i %f\n", nodes[y], nodes_feature[y].dispersion);
                }
                break;

            case 14: // MONOPOLE

                for(int y=0; y<n_size; y++)
                {
                    buf = fd_bot;
                    if(nodes_feature[y].is_top)
                        buf = fd_top;
                    fprintf(buf, "%i %f\n", nodes[y], nodes_feature[y].monopole);
                }
                break;
            case 16: // SIZE_NEIGHBORHOOD

                for(int y=0; y<n_size; y++)
                {
                    buf = fd_bot;
                    if(nodes_feature[y].is_top)
                        buf = fd_top;
                    fprintf(buf, "%i %i\n", nodes[y], nodes_feature[y].prox.n);
                }
                break;
            default:;
        }

        i++;
    }
}

void Bipartite::save_global_metrics(Repository* rep)
{
    char buf[1024];
    char* informations = rep->build_filename(rep->root, "/global.info");
    FILE* fd = fopen(informations, "w");

    fprintf(fd, "\t###############################\n");
    fprintf(fd, "\t########## INFORMATIONS ##########\n");
    fprintf(fd, "\t-----------------------------------------------------------------------------\n");
    fprintf(fd, "\tNUMBER OF NODES : %s \n", readable_nb(n_size, buf));
    fprintf(fd, "\tNUMBER OF NODES TOP : %s (%i)\n", readable_nb(n_top, buf), n_top);
    fprintf(fd, "\tNUMBER OF NODES BOT : %s (%i)\n", readable_nb(n_bot, buf), n_bot);
    fprintf(fd, "\t-----------------------------------------------------------------------------\n");
    fprintf(fd, "\tNUMBER OF LINKS : %s \n", readable_nb(m, buf));
    fprintf(fd, "\tGRAPH DENSITY : %.4f \n", density);
    fprintf(fd, "\t-----------------------------------------------------------------------------\n");
    fprintf(fd, "\tGLOBAL CC TOP : %.4f \n", global_cc_top);
    fprintf(fd, "\tGLOBAL CC BOT : %.4f \n", global_cc_bot);
    fprintf(fd, "\tGLOBAL RC TOP : %.4f \n", global_rc_top);
    fprintf(fd, "\tGLOBAL RC BOT : %.4f \n", global_rc_bot);
    fprintf(fd, "\t-----------------------------------------------------------------------------\n");    
    fprintf(fd, "\tDUPLICATED EDGES : %i \n", duplicate_edge);
    fprintf(fd, "\t###############################\n");

    fclose(fd);
    free(informations);
}