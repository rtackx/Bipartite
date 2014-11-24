#ifndef BIPARTITE_H
#define BIPARTITE_H

#include "modules.h"

class Bipartite
{
	private:
		/******************* ATTRIBUTES *******************/
		unsigned int* nodes;
		NodeFeature* nodes_feature;
		unsigned int* list_neighbors;		
		std::map<unsigned int, unsigned int> list_index_id_nodes_top;
		std::map<unsigned int, unsigned int> list_index_id_nodes_bot;		
		/**************************************************/
	    
	    /******************* FUNCTIONS ********************/
	    void load_graphfile();
	    void configuration_model();
	    void sort_neighbours();
	    void load_proximity(unsigned int);
	    unsigned int node_proximity(unsigned int&, unsigned int&);	    
	    void compute_metrics_without_proximity(unsigned int&);	    
	    /**************************************************/

		/******************* GLOBAL (AVG) METRICS *******************/
	    float global_cc_top;
	    float global_cc_bot;
	    float global_rc_top;
	    float global_rc_bot;
		/******************************************************/	    
	public:
		/******************* ATTRIBUTES *******************/
		const char* filename;
		const char* output_directory;
		// The first string at the beginning of each line contains an ID
		int type;
		bool random;

		// Number of nodes, and number of nodes sets too
		unsigned int n_size;
	    unsigned int n_top;
	    unsigned int n_bot;
	    // Number of edges, and also number of list_neighbors elements
	    unsigned int m;
	    float density;
    	unsigned int duplicate_edge;
	    
	    /**************************************************/
	    
	    /******************* FUNCTIONS ********************/
	    Bipartite(const char*, int, bool);
	    ~Bipartite();
	    void run(unsigned int);
	    void display_informations() const;
	    void compute_metrics();
	    void compute_global_metrics();
	    void save_bipartite(const char*);
	    void save_metrics(Repository*);
	    void save_nodes(Repository*);
	    void save_projections(Repository*);
	    void save_global_metrics(Repository*);
	    /**************************************************/
};

#endif
