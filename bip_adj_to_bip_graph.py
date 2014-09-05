#!/usr/bin/python
import sys

adj_graph = sys.argv[1]
list_nodes = {}

with open(adj_graph, 'r') as f_adj_graph:
	for line in f_adj_graph:
		if "%" not in line:
			nodes = line[:-1].split(' ')

			if nodes[0] not in list_nodes:
				list_nodes[nodes[0]] = []
			list_nodes[nodes[0]].append(nodes[1])

with open(adj_graph + ".bipartite", 'w') as f_adj_graph_bip:
	for node in list_nodes:
		f_adj_graph_bip.write(str(node) + ' ' + ' '.join(list_nodes[node]) + '\n')
