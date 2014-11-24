#ifndef MODULES_H
#define MODULES_H

#include <Python.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include <algorithm>
#include <iterator>
#include <vector>
#include <set>
#include <map>
#include <utility>
#include <cstring>
#include <string>
//#include <unordered_map>

//#define MAX(a,b) ((a) > (b) ? a : b)
//#define MIN(a,b) ((a) < (b) ? a : b)
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

enum FILE_METRICS { CC_TOP = 0, CC_BOT = 1, RC_TOP = 2, RC_BOT = 3,
                    DEGREE_TOP = 4, DEGREE_BOT = 5, CC_MIN_TOP = 6, CC_MIN_BOT = 7,
                    CC_MAX_TOP = 8, CC_MAX_BOT = 9, AVG_SIZE_COM_TOP = 10, AVG_SIZE_COM_BOT = 11,
                    DISPERSION_TOP = 12, DISPERSION_BOT = 13, MONOPOLE_TOP = 14, MONOPOLE_BOT = 15,
                    SIZE_NEIGHBORHOOD_TOP = 16, SIZE_NEIGHBORHOOD_BOT = 17 };

static const char* units_fs[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
static const char* units_nb[] = {"", "K", "M", "B"};
static const unsigned int n_metrics = 9;
static char const* list_names_metric[n_metrics] = {
    "cc", "rc", "degree", "cc_min", "cc_max", "avg_size_community", 
    "dispersion", "monopole", "size_neighborhood"};

/* ~~~~~~~ ******* EXTRA FUNCTIONNALITIES ******* ~~~~~~~ */
unsigned int init_mem_consuption(unsigned int, unsigned int);
int compare(const void* a, const void* b);
void handle_error(const char* msg);
char* readable_fs(double size, char *buf);
char* readable_nb(double size, char *buf);
/* ~~~~~~~ ************************************ ~~~~~~~ */

typedef struct Proximity
{
    int n;
    unsigned int* neighbors;
} PROXIMITY;

class NodeFeature
{
    public:
        bool is_top;
        unsigned int degree;
        unsigned int degree_gt1;
        unsigned int* p_neighbors;
        PROXIMITY prox;
        float dispersion;
        float monopole; 
        float avg_size_community;
        float cc;
        float cc_min;
        float cc_max;
        float rc;        
};

class Repository
{
    //static char const* extension = "data";
    public:     
        const char* root;
        char* dir_data_top;
        char* dir_data_bot;
        FILE** list_files_metric;            
        
        Repository(const char*);
        ~Repository();
        char* build_filename(const char*, const char*) const;
        void create_files_metric();
};

#endif