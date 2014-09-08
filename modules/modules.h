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

typedef struct Graph
{
    unsigned int* nodes;
    // number of nodes, and also number of nodes elements
    unsigned int n_size;
    unsigned int n_top;
    unsigned int n_bot;
    // number of edges, and also number of list_neighbors elements
    unsigned int m;
    unsigned int* list_neighbors;
    float density;
    float global_cc_top;
    float global_cc_bot;
    float global_rc_top;
    float global_rc_bot;
} GRAPH;

typedef struct Proximity
{
    int n;
    unsigned int* neighbors;
} PROXIMITY;

typedef struct Node
{
    bool is_top;
    unsigned int degree;
    unsigned int degree_gt1;
    unsigned int* p_neighbors;
    float dispersion;
    float monopole; 
    float avg_size_community;
    float cc;
    float cc_min;
    float cc_max;
    float rc;
    PROXIMITY prox;
} NODES;