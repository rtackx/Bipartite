###
# Script for Gephi using Jpython console
###

import java.awt
import operator

def f(x):
	a = 0.07
	b = 15
	return a * x * 2 + b

def select_nodes(str):
	top_nodes = {}
	for node in g.nodes:
		if node.name == str:
			top_nodes[node.Id] = node
			node.Label = ""
			node.size = 20

	return top_nodes

def color_nodes(nodes, color):
	for id, node in nodes.items():
		node.color = java.awt.Color(*color)

def degree_modify_size(nodes):
	for id, node in nodes.items():
		node.size = f(node.degree)

def write_label(filename, nodes):
	f = open(filename, "r")
	node_names = {}
	for line in f:
		line = line.replace("\n", "").split()
		node_names[line[0]] = line[1]

	for id, node in nodes.items():
		node.Label = node_names[id]

def highest_degree_nodes(nodes):
	degree = {}

	for id, node in nodes.items():
		degree[id] = node.degree

	sorted_degree = sorted(degree.items(), key=operator.itemgetter(1), reverse=True)
	hi_nodes = {}

	for i in range(0, 5):
		id = sorted_degree[i][0]
		hi_nodes[id] = nodes[id]
	
	return hi_nodes

def high_interest_nodes(nodes):
	pass

def proc():
	top_nodes = select_nodes("[TOP]")
	color_nodes(top_nodes, (114,107,255))
	degree_modify_size(top_nodes)
	bot_nodes = select_nodes("[BOT]")
	color_nodes(bot_nodes, (207, 234, 156))

	#write_label("/mnt/woolthorpe/wikidarko/Anime/AnimecategNames", highest_degree_nodes(top_nodes))
	write_label("/mnt/woolthorpe/wikidarko/Network_protocols/Network_protocolscategNames", highest_degree_nodes(top_nodes))
	


