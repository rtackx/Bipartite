#include <Python.h>
#include "modules.h"

/* ** ** ** *** * VARIABLES / METRICS * ** ** ** *** * */
GRAPH graph;
NODES* nodes = NULL;
unsigned int n_list_proximity;
std::map<unsigned int, unsigned int> list_index_id_nodes_top;
std::map<unsigned int, unsigned int> list_index_id_nodes_bot;

/****** \ ... / ******/

unsigned int memory_consumption, nb_2big_proximity;
static const char* units_fs[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
static const char* units_nb[] = {"", "K", "M", "B"};

static const unsigned int n_metrics = 18;

enum FILE_METRICS { CC_TOP = 0, CC_BOT = 1, RC_TOP = 2, RC_BOT = 3,
                    DEGREE_TOP = 4, DEGREE_BOT = 5, CC_MIN_TOP = 6, CC_MIN_BOT = 7,
                    CC_MAX_TOP = 8, CC_MAX_BOT = 9, AVG_SIZE_COM_TOP = 10, AVG_SIZE_COM_BOT = 11,
                    DISPERSION_TOP = 12, DISPERSION_BOT = 13, MONOPOLE_TOP = 14, MONOPOLE_BOT = 15,
                    SIZE_NEIGHBORHOOD_TOP = 16, SIZE_NEIGHBORHOOD_BOT = 17 };

static char const* list_files[n_metrics] = {
        "cc_top.data", "cc_bot.data",
        "rc_top.data", "rc_bot.data",
        "degree_top.data", "degree_bot.data",
        "cc_min_top.data", "cc_min_bot.data",
        "cc_max_top.data", "cc_max_bot.data",
        "avg_size_community_top.data", "avg_size_community_bot.data",        
        "dispersion_top.data", "dispersion_bot.data",
        "monopole_top.data", "monopole_bot.data",
        "size_neighborhood_top.data", "size_neighborhood_bot.data"};

char const* path_directory;
char** list_files_path;
FILE** list_files_fd;
unsigned int duplicate_edge;

/* ** ** ** *** * FUNCTIONNALITIES * ** ** ** *** * */

int compare(const void* a, const void* b)
{
    return ( *(unsigned int*)a - *(unsigned int*)b );
}

void handle_error(const char* msg)
{
    perror(msg);
    exit(255);
}

char* readable_fs(double size, char *buf) {
    int i = 0;
    
    while (size > 1024) {
        size /= 1024;
        i++;
    }
    sprintf(buf, "%.*f %s", i, size, units_fs[i]);
    return buf;
}

char* readable_nb(double size, char *buf) {
    int i = 0;
    
    while (size > 1000) {
        size /= 1000;
        i++;
    }
    sprintf(buf, "%.*f %s", i, size, units_nb[i]);
    return buf;
}

static PyObject* py_get_informations(PyObject* self, PyObject* args)
{
    char buf[1024];

    printf("**** INFORMATIONS *****\n");
    printf("** \tNUMBER NODES : %s\n", readable_nb(graph.n_size, buf));
    printf("** \tNUMBER NODES TOP : %s\n", readable_nb(graph.n_top, buf));
    printf("** \tNUMBER NODES BOT : %s\n", readable_nb(graph.n_bot, buf));
    printf("** \tDENSITY : %f\n\n", graph.density);

    Py_RETURN_NONE;
}

static PyObject* py_estimate_memory_consumption(PyObject* self, PyObject* args)
{
    size_t size_nodes, size_neighbours, size_proximity, size_metrics, size_map, total;
    char buf[1024];

    size_neighbours = sizeof(unsigned int) * graph.m;
    size_nodes = sizeof(NODES) * graph.n_size;
    size_metrics = sizeof(float) * (7 * graph.n_size);
    size_map = (sizeof(std::map<unsigned int, unsigned int>) * graph.n_top) + (sizeof(std::map<unsigned int, unsigned int>) * graph.n_bot);
    // to define : size_proximity = 
    total = size_neighbours + size_metrics + size_map + size_nodes;

    printf("**** MEMORY CONSUMPTION *****\n");
    printf("** \tSIZE OF NODE DATA : %s\n", readable_fs(size_nodes, buf));
    printf("** \tSIZE OF NEIGHBORS : %s\n", readable_fs(size_neighbours, buf));
    printf("** \tSIZE OF METRICS : %s\n", readable_fs(size_metrics, buf));
    printf("** \tSIZE OF NODE MAP : %s\n", readable_fs(size_map, buf));
    printf("** \tTOTAL : %s\n", readable_fs(total, buf));
    //printf("** \t ]-- SIZE ESTIMATION OF PROXIMITY : %s --[ \n", readable_fs(size_proximity, buf));

    Py_RETURN_NONE;
}

void write_global_information(char const* path_directory)
{
    char buf[1024];
    size_t size_informations = strlen("informations") + strlen(path_directory) + 1;
    char* informations = (char*) malloc(sizeof(char) * size_informations);
    snprintf(informations, size_informations, "%s/informations", path_directory);

    FILE* fd = fopen(informations, "w");

    fprintf(fd, "\t###############################\n");
    fprintf(fd, "\t########## INFORMATIONS ##########\n");
    fprintf(fd, "\t-----------------------------------------------------------------------------\n");
    fprintf(fd, "\tNUMBER OF NODES : %s \n", readable_nb(graph.n_size, buf));
    fprintf(fd, "\tNUMBER OF NODES TOP : %s (%i)\n", readable_nb(graph.n_top, buf), graph.n_top);
    fprintf(fd, "\tNUMBER OF NODES BOT : %s (%i)\n", readable_nb(graph.n_bot, buf), graph.n_bot);
    fprintf(fd, "\t-----------------------------------------------------------------------------\n");
    fprintf(fd, "\tNUMBER OF LINKS : %s \n", readable_nb(graph.m, buf));
    fprintf(fd, "\tGRAPH DENSITY : %.4f \n", graph.density);
    fprintf(fd, "\t-----------------------------------------------------------------------------\n");
    fprintf(fd, "\tGLOBAL CC TOP : %.4f \n", graph.global_cc_top);
    fprintf(fd, "\tGLOBAL CC BOT : %.4f \n", graph.global_cc_bot);
    fprintf(fd, "\tGLOBAL RC TOP : %.4f \n", graph.global_rc_top);
    fprintf(fd, "\tGLOBAL RC BOT : %.4f \n", graph.global_rc_bot);
    fprintf(fd, "\t-----------------------------------------------------------------------------\n");    
    fprintf(fd, "\tDUPLICATED EDGES : %i \n", duplicate_edge);
    fprintf(fd, "\t###############################\n");

    fclose(fd);
    free(informations);
}

void write_nodes(char const* path_directory)
{
    size_t size_top, size_bot;
    size_top = strlen(path_directory) + strlen("top.nodes") + 2;
    size_bot = strlen(path_directory) + strlen("bot.nodes") + 2;
    char* top = (char*) malloc(sizeof(char) * size_top);
    char* bot = (char*) malloc(sizeof(char) * size_bot);

    snprintf(top, size_top, "%s/top.nodes", path_directory);
    snprintf(bot, size_bot, "%s/bot.nodes", path_directory);

    FILE* fd_top = fopen(top, "w");
    FILE* fd_bot = fopen(bot, "w");

    for(int i=0; i<graph.n_top; i++)
        fprintf(fd_top, "%i\n", graph.nodes[i]);
    for(int i=graph.n_top; i<graph.n_size; i++)
        fprintf(fd_bot, "%i\n", graph.nodes[i]);

    fclose(fd_top);
    fclose(fd_bot);

    free(top);
    free(bot);
}

void load_files_metrics(char const* path_directory)
{
    size_t size_metrics, size_top, size_bot;
    size_metrics = strlen("/data") + strlen(path_directory) + 2;
    size_top = strlen("/data/top") + strlen(path_directory) + 2;
    size_bot = strlen("/data/bot") + strlen(path_directory) + 2;

    char* metrics = (char*) malloc(sizeof(char) * size_metrics);
    char* top = (char*) malloc(sizeof(char) * size_top);
    char* bot = (char*) malloc(sizeof(char) * size_bot);

    snprintf(metrics, size_metrics, "%s/data", path_directory);
    snprintf(top, size_top, "%s/data/top", path_directory);
    snprintf(bot, size_bot, "%s/data/bot", path_directory);

    mkdir(path_directory, 0777);
    mkdir(metrics, 0777);
    mkdir(top, 0777);
    mkdir(bot, 0777);

    list_files_path = (char**) malloc(sizeof(char*) * n_metrics);
    list_files_fd = (FILE**) malloc(sizeof(FILE*) * n_metrics);

    char* filename_top = NULL;
    char* filename_bot = NULL;
    
    for(int i=0; i<n_metrics; i++)
    {
        size_top = strlen(top) + strlen(list_files[i]) + 2;
        size_bot = strlen(bot) + strlen(list_files[i + 1]) + 2;
        filename_top = (char*) malloc(sizeof(char) * size_top);
        filename_bot = (char*) malloc(sizeof(char) * size_bot);
        
        snprintf(filename_top, size_top, "%s/%s", top, list_files[i]);
        snprintf(filename_bot, size_bot, "%s/%s", bot, list_files[i + 1]);

        list_files_path[i] = filename_top;
        list_files_path[i + 1] = filename_bot;

        list_files_fd[i] = fopen(filename_top, "w");
        list_files_fd[i + 1] = fopen(filename_bot, "w");
        if(!list_files_fd[i] || !list_files_fd[i + 1])
            handle_error("open");

        i++;
    }
    
    free(metrics);
    free(top);
    free(bot);
}

void close_files_metrics()
{
    for(int i=0; i<n_metrics; i++)
    {
        fclose(list_files_fd[i]);
        fclose(list_files_fd[i + 1]);
        free(list_files_path[i]);
        free(list_files_path[i + 1]);

        i++;
    }

    free(list_files_fd);
    free(list_files_path);
}

void write_metrics()
{
    FILE* buf;
    FILE* fd_top;
    FILE* fd_bot;

    for(int i=0; i<n_metrics; i++)
    {
        fd_top = list_files_fd[i];
        fd_bot = list_files_fd[i + 1];

        switch(i)
        {
            case 0: // CC

                for(int y=0; y<graph.n_size; y++)
                {
                    buf = fd_bot;
                    if(nodes[y].is_top)
                        buf = fd_top;
                        
                    fprintf(buf, "%i %f\n", graph.nodes[y], nodes[y].cc);
                }
                break;

            case 2: // RC

                for(int y=0; y<graph.n_size; y++)
                {
                    buf = fd_bot;
                    if(nodes[y].is_top)
                        buf = fd_top;

                    fprintf(buf, "%i %f\n", graph.nodes[y], nodes[y].rc);
                }
                break;

            case 4: // DEGREE

                for(int y=0; y<graph.n_size; y++)
                {
                    buf = fd_bot;
                    if(nodes[y].is_top)
                        buf = fd_top;
                    fprintf(buf, "%i %i\n", graph.nodes[y], nodes[y].degree);
                }
                break;

            case 6: // CC_MIN

                for(int y=0; y<graph.n_size; y++)
                {
                    buf = fd_bot;
                    if(nodes[y].is_top)
                        buf = fd_top;
                    fprintf(buf, "%i %f\n", graph.nodes[y], nodes[y].cc_min);
                }
                break;

            case 8: // CC_MAX

                for(int y=0; y<graph.n_size; y++)
                {
                    buf = fd_bot;
                    if(nodes[y].is_top)
                        buf = fd_top;
                    fprintf(buf, "%i %f\n", graph.nodes[y], nodes[y].cc_max);
                }
                break;

            case 10: // AVG_SIZE_COM

                for(int y=0; y<graph.n_size; y++)
                {
                    buf = fd_bot;
                    if(nodes[y].is_top)
                        buf = fd_top;
                    fprintf(buf, "%i %f\n", graph.nodes[y], nodes[y].avg_size_community);
                }
                break;

            case 12: // DISPERSION

                for(int y=0; y<graph.n_size; y++)
                {
                    buf = fd_bot;
                    if(nodes[y].is_top)
                        buf = fd_top;
                    fprintf(buf, "%i %f\n", graph.nodes[y], nodes[y].dispersion);
                }
                break;

            case 14: // MONOPOLE

                for(int y=0; y<graph.n_size; y++)
                {
                    buf = fd_bot;
                    if(nodes[y].is_top)
                        buf = fd_top;
                    fprintf(buf, "%i %f\n", graph.nodes[y], nodes[y].monopole);
                }
                break;
            case 16: // SIZE_NEIGHBORHOOD

                for(int y=0; y<graph.n_size; y++)
                {
                    buf = fd_bot;
                    if(nodes[y].is_top)
                        buf = fd_top;
                    fprintf(buf, "%i %i\n", graph.nodes[y], nodes[y].prox.n);
                }
                break;
            default:;
        }

        i++;
    }
}

/* ** ** ** *** * PARSER / WORDCOUNTER / GRAPH BUILDING * ** ** ** *** * */
void random_graph_fixed_degree_distribution()
{
    int count_top, count_bot, v, u, tmp, m;
    m = graph.m / 2;

    unsigned int* list_repeated_nodes_top = PyMem_New(unsigned int, m);
    unsigned int* list_repeated_nodes_bot = PyMem_New(unsigned int, m);
    unsigned int* tmp_array;
    
    count_top = count_bot = 0;
    for(int i=0; i<graph.n_size; i++)
    {
        tmp_array = PyMem_New(unsigned int, nodes[i].degree);
        
        for(int j=0; j<nodes[i].degree; j++)
            tmp_array[j] = i;

        if(nodes[i].is_top)
        {
            memcpy(list_repeated_nodes_top + count_top, tmp_array, sizeof(unsigned int) * nodes[i].degree);
            count_top += nodes[i].degree;
        }
        else
        {
            memcpy(list_repeated_nodes_bot + count_bot, tmp_array, sizeof(unsigned int) * nodes[i].degree);
            count_bot += nodes[i].degree;
        }

        
        nodes[i].degree = 0;
        nodes[i].p_neighbors = NULL;
    }

    srand(time(NULL));
    while(m > 1)
    {        
        u = rand() % (m - 1) + 0;
        v = rand() % (m - 1) + 0;

        // swap  values of top
        tmp = list_repeated_nodes_top[m - 1];
        list_repeated_nodes_top[m - 1] = list_repeated_nodes_top[u];
        list_repeated_nodes_top[u] = tmp;

        // swap  values of bot
        tmp = list_repeated_nodes_bot[m - 1];
        list_repeated_nodes_bot[m - 1] = list_repeated_nodes_bot[v];
        list_repeated_nodes_bot[v] = tmp;

        m--;
    }

    m = graph.m / 2;
    unsigned new_m = 0;
    unsigned duplicated = 0;
    bool exist;
    for(int i=0; i<m; i++)
    {
        exist = false;
        for(int y=0; y<nodes[list_repeated_nodes_top[i]].degree; y++)
        {
            if(nodes[list_repeated_nodes_top[i]].p_neighbors[y] == graph.nodes[list_repeated_nodes_bot[i]])
            {
                exist = true;
                duplicated++;
                break;
            }
        }

        if(!exist)
        {
            nodes[list_repeated_nodes_top[i]].p_neighbors = PyMem_Resize(nodes[list_repeated_nodes_top[i]].p_neighbors, unsigned int, nodes[list_repeated_nodes_top[i]].degree + 1);
            nodes[list_repeated_nodes_top[i]].p_neighbors[nodes[list_repeated_nodes_top[i]].degree] = graph.nodes[list_repeated_nodes_bot[i]];
            nodes[list_repeated_nodes_top[i]].degree++;
            new_m++;
        }
    }

    for(int i=0; i<m; i++)
    {
        exist = false;
        for(int y=0; y<nodes[list_repeated_nodes_bot[i]].degree; y++)
        {
            if(nodes[list_repeated_nodes_bot[i]].p_neighbors[y] == list_repeated_nodes_top[i])
            {
                exist = true;
                break;
            }
        }

        if(!exist)
        {
            nodes[list_repeated_nodes_bot[i]].p_neighbors = PyMem_Resize(nodes[list_repeated_nodes_bot[i]].p_neighbors, unsigned int, nodes[list_repeated_nodes_bot[i]].degree + 1);
            nodes[list_repeated_nodes_bot[i]].p_neighbors[nodes[list_repeated_nodes_bot[i]].degree] = graph.nodes[list_repeated_nodes_top[i]];
            nodes[list_repeated_nodes_bot[i]].degree++;
            new_m++;
        }
    }

    printf("Number of duplicated edges : %i\n", duplicated);
    graph.m = new_m;
    duplicate_edge = duplicated;
}

void wc(const char* fname, int type)
{
    static const int BUFFER_SIZE = 1024*1024;
    FILE * fd = fopen(fname, "r");
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

        //graph.nodes = (unsigned int*) realloc(graph.nodes, sizeof(unsigned int) * (graph.n_top + 1));
        graph.nodes = PyMem_Resize(graph.nodes, unsigned int, graph.n_top + 1);
        //nodes = (NODES*) realloc(nodes, sizeof(NODES) * (graph.n_top + 1));
        nodes = PyMem_Resize(nodes, NODES, graph.n_top + 1);

        if(type == 1)
        {
            id_node = atoi(tmp);
            tmp = strtok(NULL, " \t\n");
        }
        else
            id_node = index_node_top;

        graph.nodes[index_node_top] = id_node;
        list_index_id_nodes_top.insert(std::pair<unsigned int, unsigned int>(id_node, index_node_top));

        list_neighbors_top = NULL;
        y = 0;
        while (tmp != NULL)
        {
            id_node = atoi(tmp);

            //list_neighbors_top = (unsigned int*) realloc(list_neighbors_top, sizeof(unsigned int) * (y+1));
            list_neighbors_top = PyMem_Resize(list_neighbors_top, unsigned int, y + 1);

            list_neighbors_top[y] = id_node;
            list_neighbors_bot[id_node].push_back(graph.nodes[index_node_top]);
            //std::sort(list_neighbors_bot[id_node].begin(), list_neighbors_bot[id_node].end());
            tmp = strtok(NULL, " \t\n");

            y++;
        }

        //graph.list_neighbors = (unsigned int*) realloc(graph.list_neighbors, sizeof(unsigned int) * (graph.m + y));
        graph.list_neighbors = PyMem_Resize(graph.list_neighbors, unsigned int, graph.m + y);
        memcpy(graph.list_neighbors + graph.m, list_neighbors_top, sizeof(unsigned int) * y);

        nodes[index_node_top].degree = y;
        nodes[index_node_top].is_top = true;
        nodes[index_node_top].prox.n = 0;
        nodes[index_node_top].prox.neighbors = NULL;

        graph.n_top++;
        index_node_top++;
        graph.m += y;
    }
    printf("\n");
    fclose(fd);

    graph.n_size = graph.n_top + list_neighbors_bot.size();
    graph.n_bot = list_neighbors_bot.size();
    index_node_bot = index_node_top;

    //nodes = (NODES*) realloc(nodes, sizeof(NODES) * (graph.n_size));
    //graph.nodes = (unsigned int*) realloc(graph.nodes, sizeof(unsigned int) * (graph.n_size));
    nodes = PyMem_Resize(nodes, NODES, graph.n_size);
    graph.nodes = PyMem_Resize(graph.nodes, unsigned int, graph.n_size);

    size_t size;
    std::map<unsigned int, std::vector<unsigned int> >::iterator it;
    for(it = list_neighbors_bot.begin(); it != list_neighbors_bot.end(); ++it)
    {
        size = it->second.size();
        id_node = it->first;

        //graph.list_neighbors = (unsigned int*) realloc(graph.list_neighbors, sizeof(unsigned int) * (graph.m + size));
        graph.list_neighbors = PyMem_Resize(graph.list_neighbors, unsigned int, graph.m + size);

        std::copy(it->second.begin(), it->second.end(), graph.list_neighbors + graph.m);
        nodes[index_node_bot].degree = size;
        nodes[index_node_bot].is_top = false;
        nodes[index_node_bot].prox.n = 0;
        nodes[index_node_bot].prox.neighbors = NULL;
        
        graph.nodes[index_node_bot] = id_node;

        list_index_id_nodes_bot.insert(std::pair<unsigned int, unsigned int>(id_node, index_node_bot));
        
        graph.m += size;
        index_node_bot++;
    }

    unsigned int s = 0;
    for(int i=0; i<graph.n_size; i++)
    {
        nodes[i].p_neighbors = graph.list_neighbors + s;
        s += nodes[i].degree;
    }

    /* Advise the kernel of our access pattern.  */
    //posix_fadvise(fd, 0, 0, 1);  // FDADVICE_SEQUENTIAL

}


/* ** ** ** *** * LOADING GRAPH * ** ** ** *** * */

void sort_neighbours()
{
    for(int i=0; i<graph.n_size; i++)
        std::sort(nodes[i].p_neighbors, nodes[i].p_neighbors + nodes[i].degree);
}

void init_mem_consuption()
{
    memory_consumption = sizeof(unsigned int) * graph.m;
    memory_consumption += sizeof(NODES) * graph.n_size;
    memory_consumption += sizeof(GRAPH);
    memory_consumption += sizeof(std::map<int,int>) * graph.n_size;
}

void save_graph(char const* filename, unsigned int type)
{
    FILE* fd_graph = fopen(filename, "w");

    for(int i=0; i<graph.n_top; i++)
    {
        if(type)
            fprintf(fd_graph, "%i ", graph.nodes[i]);
        for(int y=0; y<nodes[i].degree; y++)
            fprintf(fd_graph, "%i ", nodes[i].p_neighbors[y]);
        fprintf(fd_graph, "\n");
    }

    fclose(fd_graph);
}

static PyObject* py_load_graph(PyObject* self, PyObject* args)
{
    char const* filename;
    unsigned int type;
    int random;
    PyArg_ParseTuple(args, "ssii", &path_directory, &filename, &type, &random);

    graph.n_size = 0;
    graph.n_top = 0;
    graph.n_bot = 0;
    graph.m = 0;
    duplicate_edge = 0;
    graph.list_neighbors = NULL;

    load_files_metrics(path_directory);

    wc(filename, type);
    if(random)
    {
        printf("\t\t[ Creating random bipartite graph... ]\n");
        random_graph_fixed_degree_distribution();

        size_t size = strlen(path_directory) + strlen("/random_graph") + 2;
        char* save_graph_filename = (char*) malloc(sizeof(char) * size);    
        snprintf(save_graph_filename, size, "%s/random_graph", path_directory);

        printf("\t\t[ Saving random bipartite graph... ]\n");
        save_graph(save_graph_filename, type);
        free(save_graph_filename);
    }
    sort_neighbours();

    graph.density = 1.0 * (graph.m) / (graph.n_top * graph.n_bot);
    init_mem_consuption();

    Py_RETURN_NONE;
}

static PyObject* py_unload_graph(PyObject* self, PyObject* args)
{
    for(int i=0; i<graph.n_size; i++)
    {
        if(nodes[i].prox.neighbors != NULL)
            free(nodes[i].prox.neighbors);
    }

    if(graph.nodes != NULL)
        free(graph.nodes);
    
    if(graph.list_neighbors != NULL)
        free(graph.list_neighbors);
    
    if(nodes != NULL)
        free(nodes);

    Py_RETURN_NONE;
}

unsigned int load_neighbour_proximity(unsigned int& index_node, unsigned int& limit)
{
    std::set<unsigned int> set_neighbours;
    unsigned int count_degree_gt1 = 0, index_neighbor;

    for(int y=0; y<nodes[index_node].degree; y++)
    {
        if(nodes[index_node].is_top)
            index_neighbor = list_index_id_nodes_bot.at(nodes[index_node].p_neighbors[y]);
        else
            index_neighbor = list_index_id_nodes_top.at(nodes[index_node].p_neighbors[y]);

        if(nodes[index_neighbor].degree > 1)
            count_degree_gt1++;

        if(set_neighbours.size() >= limit)
            continue;
        else
        {
            for(int j=0; j<nodes[index_neighbor].degree; j++)
                set_neighbours.insert(nodes[index_neighbor].p_neighbors[j]);
        }
    }

    set_neighbours.erase(graph.nodes[index_node]);
    nodes[index_node].prox.n = set_neighbours.size();
    // Nodes with degree greater than 1 (1-degree depreciated)
    nodes[index_node].degree_gt1 = count_degree_gt1;

    if(set_neighbours.size() >= limit)
    {
        nodes[index_node].prox.n = -1;
        return -1;
    }

    nodes[index_node].prox.neighbors = PyMem_New(unsigned int, set_neighbours.size());
    std::copy(set_neighbours.begin(), set_neighbours.end(), nodes[index_node].prox.neighbors);

    return sizeof(unsigned int) * set_neighbours.size();
}

static PyObject* py_load_proximity(PyObject* self, PyObject* args)
{
    unsigned int limit;
    PyArg_ParseTuple(args, "i", &limit);

    char buf[1024];
    unsigned int nb_large_proximity, mem;

    nb_2big_proximity = 0;

    for(unsigned int i=0; i<graph.n_size; i++)
    {       
        printf("\rProceeding percentage %.3f%% (memory consumption : %s) ", (i / (1.0 * (graph.n_size - 1))) * 100.0, readable_fs(memory_consumption, buf));
        fflush(stdout);
        mem = load_neighbour_proximity(i, limit);
        memory_consumption += mem;
        
        if(mem == -1)
            nb_2big_proximity++;
        
    }
    printf("\n");
    printf("- Number of nodes having more than %i neighbors at distance 2 : %i\n", limit, nb_2big_proximity);

    Py_RETURN_NONE;
}

/* ** ** ** *** * COMPUTATIONS METRICS * ** ** ** *** * */

static PyObject* py_compute_isolation(PyObject* self, PyObject* args)
{
    unsigned int count, index_neighbor;

    count = 0;
    for(unsigned int i=0; i<graph.n_top; i++)
    {
        if(nodes[i].degree == 1)
        {
            index_neighbor = list_index_id_nodes_bot.at(nodes[i].p_neighbors[0]);
            if(nodes[index_neighbor].degree > 1)
                count++;
        }
    }
    printf("%i communautés à degré 1 avec voisin\n", count);

    count = 0;
    for(unsigned int i=graph.n_top; i<graph.n_size; i++)
    {
        if(nodes[i].degree == 1)
        {
            index_neighbor = list_index_id_nodes_top.at(nodes[i].p_neighbors[0]);
            if(nodes[index_neighbor].degree > 1)
                count++;
        }
    }
    printf("%i noeuds à degré 1 avec voisin\n", count);

    Py_RETURN_NONE;
}

void compute_global_metrics()
{
    float cumul_cc_top, cumul_cc_bot, cumul_rc_top, cumul_rc_bot;
    cumul_cc_top = cumul_cc_bot = cumul_rc_top = cumul_rc_bot = 0.0;

    for(int i=0; i<graph.n_top; i++)
    {
        if(nodes[i].cc != -1)
            cumul_cc_top += nodes[i].cc;
        if(nodes[i].rc != -1)
            cumul_rc_top += nodes[i].rc;
    }
    for(int i=graph.n_top; i<graph.n_size; i++)
    {
        if(nodes[i].cc != -1)
            cumul_cc_bot += nodes[i].cc;
        if(nodes[i].rc != -1)
            cumul_rc_bot += nodes[i].rc;
    }

    graph.global_cc_top = 1.0 * cumul_cc_top / graph.n_top;
    graph.global_cc_bot = 1.0 * cumul_cc_bot / graph.n_bot;
    graph.global_rc_top = 1.0 * cumul_rc_top / graph.n_top;
    graph.global_rc_bot = 1.0 * cumul_rc_bot / graph.n_bot;
}

void basic_compute_metrics(unsigned int& index_node)
{
    unsigned int index_neighbor, index_h, count_dispersion, size, count_degree_community;
    float v, v_min, v_max;
    std::vector<unsigned int> s1, buf_s1, s2;
    std::set<unsigned int> s3, proximity;

    count_dispersion = 0;
    count_degree_community = 0;
    v = v_min = v_max = 0.0;

    s1.resize(nodes[index_node].degree);
    buf_s1.resize(nodes[index_node].degree);
    memcpy(&s1[0], nodes[index_node].p_neighbors, sizeof(unsigned int) * nodes[index_node].degree);
    memcpy(&buf_s1[0], nodes[index_node].p_neighbors, sizeof(unsigned int) * nodes[index_node].degree);

    for(int h=0; h<nodes[index_node].degree; h++)
    {
        if(nodes[index_node].is_top)
            index_h = list_index_id_nodes_bot.at(nodes[index_node].p_neighbors[h]);
        else
            index_h = list_index_id_nodes_top.at(nodes[index_node].p_neighbors[h]);

        count_dispersion += (nodes[index_h].degree - 1);

        for(int j=0; j<nodes[index_h].degree; j++)
        {
            if(nodes[index_h].is_top)
                index_neighbor = list_index_id_nodes_bot.at(nodes[index_h].p_neighbors[j]);
            else
                index_neighbor = list_index_id_nodes_top.at(nodes[index_h].p_neighbors[j]);

            if(index_node == index_neighbor)
                continue;                
            if(proximity.size() > 0 && proximity.find(index_neighbor) != proximity.end())
                continue;
            proximity.insert(index_neighbor);
            
            count_degree_community += nodes[index_neighbor].degree;

            s2.resize(nodes[index_neighbor].degree);
            memcpy(&s2[0], nodes[index_neighbor].p_neighbors, sizeof(unsigned int) * nodes[index_neighbor].degree);
                
            // INTERSECTION (JACKARD)
            std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), inserter(s3, s3.begin()));

            v += 1.0 * (s3.size()) / (nodes[index_node].degree + nodes[index_neighbor].degree - s3.size());
            v_min += 1.0 * (s3.size()) / (min(nodes[index_node].degree, nodes[index_neighbor].degree));
            v_max += 1.0 * (s3.size()) / (max(nodes[index_node].degree, nodes[index_neighbor].degree));

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

        size = proximity.size();
        if(size == 0)
            continue;

        // ############## COMPUTATION AVG SIZE COMMUNITY ############## //
        if(nodes[index_node].is_top)
            nodes[index_node].avg_size_community = 1.0 * (count_degree_community / size);
        else
            nodes[index_node].avg_size_community = 1.0 * (count_dispersion + nodes[index_node].degree) / nodes[index_node].degree;


        // ############## COMPUTATION DISPERSION ############## //
        if(count_dispersion > 0)
            nodes[index_node].dispersion = 1.0 * (size) / (count_dispersion);            

        // ############## COMPUTATION CC ############## //
        nodes[index_node].cc = 1.0 * (v) / (size);
        nodes[index_node].cc_min = 1.0 * (v_min) / (size);
        nodes[index_node].cc_max = 1.0 * (v_max) / (size);

        // ############## COMPUTATION RC ############## //
        if(nodes[index_node].degree == 1)
            nodes[index_node].rc = 0.0;
        else
            nodes[index_node].rc = 1.0 * (2 * (s1.size() - buf_s1.size())) / (nodes[index_node].degree * (nodes[index_node].degree - 1));

        nodes[index_node].prox.n = size;
    }
}

void compute_metrics()
{
    unsigned int index_neighbor, count_dispersion, size, count_degree_community;
    float v, v_min, v_max;
    std::vector<unsigned int> s1, buf_s1, s2;
    std::set<unsigned int> s3;

    for(unsigned int i=0; i<graph.n_size; i++)
    {
        printf("\rProceeding percentage %.3f%%", (i / (1.0 * (graph.n_size - 1))) * 100.0);
        fflush(stdout);

        // Initialisation of metric values          
        nodes[i].cc = -1.0;
        nodes[i].cc_min = -1.0;
        nodes[i].cc_max = -1.0;
        nodes[i].rc = -1.0;
        nodes[i].avg_size_community = 0.0;
        nodes[i].monopole = 0.0;
        nodes[i].dispersion = 0.0;

        if(nodes[i].degree == 0)
            continue;

        // ############## COMPUTATION MONOPOLE ############## //
        nodes[i].monopole = 1.0 * (nodes[i].degree - nodes[i].degree_gt1) / (nodes[i].degree);

        size = nodes[i].prox.n;
        if(size == -1)
        {
            basic_compute_metrics(i);
            continue;
        }
        else if(size == 0)
            continue;

        count_dispersion = 0;
        count_degree_community = 0;
        v = v_min = v_max = 0.0;

        for(int y=0; y<nodes[i].degree; y++)
        {
            if(nodes[i].is_top)
                index_neighbor = list_index_id_nodes_bot.at(nodes[i].p_neighbors[y]);
            else
                index_neighbor = list_index_id_nodes_top.at(nodes[i].p_neighbors[y]);

            count_dispersion += (nodes[index_neighbor].degree - 1);
        }
        
        //s1(nodes[i].p_neighbors, nodes[i].p_neighbors + nodes[i].degree);
        s1.resize(nodes[i].degree);
        buf_s1.resize(nodes[i].degree);
        memcpy(&s1[0], nodes[i].p_neighbors, sizeof(unsigned int) * nodes[i].degree);
        memcpy(&buf_s1[0], nodes[i].p_neighbors, sizeof(unsigned int) * nodes[i].degree);
                    
        for(int y=0; y<size; y++)
        {
            if(nodes[i].is_top)
                index_neighbor = list_index_id_nodes_top.at(nodes[i].prox.neighbors[y]);
            else
                index_neighbor = list_index_id_nodes_bot.at(nodes[i].prox.neighbors[y]);

            count_degree_community += nodes[index_neighbor].degree;

            s2.resize(nodes[index_neighbor].degree);
            memcpy(&s2[0], nodes[index_neighbor].p_neighbors, sizeof(unsigned int) * nodes[index_neighbor].degree);

            // INTERSECTION (JACKARD)
            std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), inserter(s3, s3.begin()));

            v += 1.0 * (s3.size()) / (nodes[i].degree + nodes[index_neighbor].degree - s3.size());
            v_min += 1.0 * (s3.size()) / (min(nodes[i].degree, nodes[index_neighbor].degree));
            v_max += 1.0 * (s3.size()) / (max(nodes[i].degree, nodes[index_neighbor].degree));

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
            nodes[i].dispersion = 1.0 * (size) / (count_dispersion);
            

        // ############## COMPUTATION CC ############## //
        nodes[i].cc = 1.0 * (v) / (size);
        nodes[i].cc_min = 1.0 * (v_min) / (size);
        nodes[i].cc_max = 1.0 * (v_max) / (size);

        // ############## COMPUTATION RC ############## //
        if(nodes[i].degree == 1)
            nodes[i].rc = 0.0;
        else
            nodes[i].rc = 1.0 * (2 * (s1.size() - buf_s1.size())) / (nodes[i].degree * (nodes[i].degree - 1));

        // ############## COMPUTATION AVG SIZE COMMUNITY ############## //
        if(nodes[i].is_top)
            nodes[i].avg_size_community = 1.0 * (count_degree_community / size);
        else
            nodes[i].avg_size_community = 1.0 * (count_dispersion + nodes[i].degree) / nodes[i].degree;            

        s1.clear();
        buf_s1.clear();

    }
    printf("\n");
}

static PyObject* py_treat_compute_metrics(PyObject* self, PyObject* args)
{    
    compute_metrics();    
    compute_global_metrics();    

    Py_RETURN_NONE;
}

static PyObject* py_write_metrics(PyObject* self, PyObject* args)
{
    write_metrics();
    write_nodes(path_directory);
    write_global_information(path_directory);
    close_files_metrics();

    Py_RETURN_NONE;
}

/* ** ** ** *** * ACCESSORS * ** ** ** *** * */

static PyObject* py_get_node(PyObject* self, PyObject* args)
{
    unsigned int index_node;
    PyArg_ParseTuple(args, "i", &index_node);

    return Py_BuildValue("i", graph.nodes[index_node]);
}

static PyObject* py_get_neighbors(PyObject* self, PyObject* args)
{
    unsigned int index_node;
    PyArg_ParseTuple(args, "i", &index_node);

    if(index_node > graph.n_size)
    {
        PyErr_SetString(PyExc_RuntimeError, "test except");
        return NULL;
    }

    unsigned int size = nodes[index_node].degree;
    PyObject *lst = PyList_New(size);

    PyObject* buf_element;
    for(int i=0; i<size; i++)
    {
        PyObject* buf_element = Py_BuildValue("i", *(nodes[index_node].p_neighbors + i));
        PyList_SET_ITEM(lst, i, buf_element);
    }

    return lst;
}

static PyObject* py_get_proximity(PyObject* self, PyObject* args)
{
    unsigned int index_node, index_neighbor;
    PyArg_ParseTuple(args, "i", &index_node);

    unsigned int size = nodes[index_node].prox.n;
    PyObject *lst = PyList_New(size);

    for(int i=0; i<size; i++)
    {
        if(nodes[index_node].is_top)
            index_neighbor = list_index_id_nodes_top.at(nodes[index_node].prox.neighbors[i]);
        else
            index_neighbor = list_index_id_nodes_bot.at(nodes[index_node].prox.neighbors[i]);

        PyObject* buf_element = Py_BuildValue("i", index_neighbor);
        PyList_SET_ITEM(lst, i, buf_element);
    }

    return lst;
}

static PyObject* py_get_node_metrics(PyObject* self, PyObject* args)
{
    unsigned int index_node;
    PyArg_ParseTuple(args, "i", &index_node);

    PyObject *lst = PyList_New(9);

    PyObject* buf_element;
    buf_element = Py_BuildValue("i", nodes[index_node].degree);
    PyList_SET_ITEM(lst, 0, buf_element);
    buf_element = Py_BuildValue("i", nodes[index_node].degree_gt1);
    PyList_SET_ITEM(lst, 1, buf_element);
    buf_element = Py_BuildValue("f", nodes[index_node].dispersion);
    PyList_SET_ITEM(lst, 2, buf_element);
    buf_element = Py_BuildValue("f", nodes[index_node].monopole);
    PyList_SET_ITEM(lst, 3, buf_element);
    buf_element = Py_BuildValue("f", nodes[index_node].avg_size_community);
    PyList_SET_ITEM(lst, 4, buf_element);
    buf_element = Py_BuildValue("f", nodes[index_node].cc);
    PyList_SET_ITEM(lst, 5, buf_element);
    buf_element = Py_BuildValue("f", nodes[index_node].cc_min);
    PyList_SET_ITEM(lst, 6, buf_element);
    buf_element = Py_BuildValue("f", nodes[index_node].cc_max);
    PyList_SET_ITEM(lst, 7, buf_element);
    buf_element = Py_BuildValue("f", nodes[index_node].rc);
    PyList_SET_ITEM(lst, 8, buf_element);

    return lst;
}

// INITIALISATION DEFINITION PYTHON
static PyMethodDef calcModule_methods[] = {
    {"load_graph", py_load_graph, METH_VARARGS},    
    {"get_neighbors", py_get_neighbors, METH_VARARGS},
    {"get_node", py_get_node, METH_VARARGS},
    {"load_proximity", py_load_proximity, METH_VARARGS},
    {"get_proximity", py_get_proximity, METH_VARARGS},
    {"treat_compute_metrics", py_treat_compute_metrics, METH_VARARGS},
    {"get_node_metrics", py_get_node_metrics, METH_VARARGS},
    {"estimate_memory_consumption", py_estimate_memory_consumption, METH_VARARGS},
    {"get_informations", py_get_informations, METH_VARARGS},
    {"unload_graph", py_unload_graph, METH_VARARGS},
    {"compute_isolation", py_compute_isolation, METH_VARARGS},
    {"write_metrics", py_write_metrics, METH_VARARGS},    
	{NULL, NULL}
};

PyMODINIT_FUNC initcalcModule()
{
	(void) Py_InitModule("calcModule", calcModule_methods);
}

