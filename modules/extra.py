import time, string, random, os
import ConfigParser
import modules.graph_bipartite
#import gexf

def get_plotting_configurations():	
	config = ConfigParser.ConfigParser()
	config.read("plot_metrics.ini")

	metrics = []
	for m in config.items("metrics"):
		if m[1] == '1':
			metrics.append(m[0])

	correlations = []
	for m in config.items("correlations"):
		if m[1] == '1':
			correlations.append(m[0])

	metrics_log = []
	for m in config.items("metrics_log"):
		if m[1] == '1':
			metrics_log.append(m[0])

	return metrics, correlations, metrics_log

def timeit(method):
    def timed(*args, **kw):    	
		ts = time.time()
		result = method(*args, **kw)
		te = time.time()

		print '%r (%r, %r) %2.9f sec' % \
              (method.__name__, args, kw, te-ts)
		return result

    return timed

def id_generator(size=6, chars=string.ascii_uppercase + string.digits):
	return ''.join(random.choice(chars) for _ in range(size))

def detect_community(G):
	list_coeff_degree = {}
	list_max_coeff_degree = {}
	list_min_coeff_degree = {}
	
	for id_node in G.nodes:
		list_coeff_degree[id_node] = []
		list_max_coeff_degree[id_node] = 0.0
		list_min_coeff_degree[id_node] = 1.0
		for id_neighbour in G.nodes[id_node].list_neighbours:
			calc = (1 / float(G.nodes[id_node].degree) + 1 / float(G.nodes[id_neighbour].degree)) * (1 / float(G.nodes[id_neighbour].degree))
			list_coeff_degree[id_node].append((id_neighbour, calc))

			if calc > list_max_coeff_degree[id_node]:
				list_max_coeff_degree[id_node] = calc
			if calc < list_min_coeff_degree[id_node]:
				list_min_coeff_degree[id_node] = calc

	print list_coeff_degree
	print list_max_coeff_degree
	print list_min_coeff_degree

	community = {}

	for id_node in G.nodes:
		community[id_node] = []
		for id_neighbour in G.nodes[id_node].list_neighbours:
			if list_max_coeff_degree[id_node] >= list_max_coeff_degree[id_neighbour] and list_max_coeff_degree[id_node] <= list_max_coeff_degree[id_neighbour]:
				community[id_node].append(id_neighbour)

	print community

def num(s):
    try:
        return int(s)
    except ValueError:
        return float(s)
