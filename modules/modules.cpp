#include "modules.h"

/* ~~~~~~~ ******* EXTRA FUNCTIONNALITIES ******* ~~~~~~~ */

int compare(const void* a, const void* b)
{
    return ( *(unsigned int*)a - *(unsigned int*)b );
}

void handle_error(const char* msg)
{
    perror(msg);
    exit(255);
}

char* readable_fs(double size, char *buf)
{
    int i = 0;
    
    while (size > 1024) {
        size /= 1024;
        i++;
    }
    sprintf(buf, "%.*f %s", i, size, units_fs[i]);
    return buf;
}

char* readable_nb(double size, char *buf)
{
    int i = 0;
    
    while (size > 1000) {
        size /= 1000;
        i++;
    }
    sprintf(buf, "%.*f %s", i, size, units_nb[i]);
    return buf;
}

unsigned int init_mem_consuption(unsigned int graph_size, unsigned int number_edges)
{
	unsigned int memory_consumption = 0;
    memory_consumption = sizeof(unsigned int) * number_edges;
    memory_consumption += sizeof(NodeFeature) * graph_size;
    memory_consumption += sizeof(std::map<int,int>) * graph_size;

    return memory_consumption;
}

// Estimate how much memory it requieres to compute metrics
void estimate_memory_consumption(unsigned int graph_size, unsigned int number_edges, unsigned int top_size, unsigned int bot_size)
{
    size_t size_nodes, size_neighbours, size_proximity, size_metrics, size_map, total;
    char buf[1024];

    size_neighbours = sizeof(unsigned int) * number_edges;
    size_nodes = sizeof(NodeFeature) * graph_size;
    size_map = (sizeof(std::map<unsigned int, unsigned int>) * top_size) + (sizeof(std::map<unsigned int, unsigned int>) * bot_size);
    total = size_neighbours + size_map + size_nodes;

    printf("**** MEMORY CONSUMPTION *****\n");
    printf("** \tSIZE OF NODE DATA : %s\n", readable_fs(size_nodes, buf));
    printf("** \tSIZE OF NEIGHBORS : %s\n", readable_fs(size_neighbours, buf));
    printf("** \tSIZE OF NODE MAP : %s\n", readable_fs(size_map, buf));
    printf("** \tTOTAL : %s\n", readable_fs(total, buf));
    //printf("** \t ]-- SIZE ESTIMATION OF PROXIMITY : %s --[ \n", readable_fs(size_proximity, buf));
}

/* *-------------------- CLASS : Repository -------------- */

Repository::Repository(const char* root) : root(root)
{
	mkdir(root, 0777);
	mkdir(build_filename(root, "/data"), 0777);
	dir_data_top = build_filename(root, "/data/top");
	dir_data_bot = build_filename(root, "/data/bot");
	mkdir(dir_data_top, 0777);
	mkdir(dir_data_bot, 0777);

	create_files_metric();
}

Repository::~Repository()
{
	for(int i=0; i<n_metrics * 2; i++)
        fclose(list_files_metric[i]);

    free(dir_data_top);
    free(dir_data_bot);
}

char* Repository::build_filename(const char* first, const char* second) const
{
	size_t size = strlen(first) + strlen(second) + 3;
    char* filename = (char*) malloc(sizeof(char) * size);
    snprintf(filename, size, "%s%s", first, second);

    return filename;
}

void Repository::create_files_metric()
{
	list_files_metric = (FILE**) malloc(sizeof(FILE*) * n_metrics * 2);
	char* filename_top;
	char* filename_bot;

	int j = 0;
	for(int i=0; i<n_metrics; i++)
    {
    	filename_top = build_filename(dir_data_top, "/");
    	filename_top = build_filename(filename_top, list_names_metric[i]);
    	filename_top = build_filename(filename_top, "_top.data");
    	filename_bot = build_filename(dir_data_bot, "/");
    	filename_bot = build_filename(filename_bot, list_names_metric[i]);
    	filename_bot = build_filename(filename_bot, "_bot.data");

        list_files_metric[j] = fopen(filename_top, "w");
        list_files_metric[j + 1] = fopen(filename_bot, "w");

        if(!list_files_metric[i] || !list_files_metric[i + 1])
            handle_error("open");

        free(filename_top);
    	free(filename_bot);

    	j += 2;
    }
}