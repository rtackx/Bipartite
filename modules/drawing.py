import networkx as nx
import matplotlib, copy
matplotlib.use('Agg')
import matplotlib.pyplot as plt

import modules.graph

def find_multiple(number):
	i = 1
	reached = False
	while i < number:
		if i*i == number:
			return i
		elif i*i > number:
			if reached:
				return i
			else:
				i /= 2
				reached = True
		else:
			if reached:
				i += 1
			else:
				i *= 2

class DrawGraph():
	def __init__(self, filegraph):
		self.nxGraph = nx.Graph()
		self.filegraph = filegraph

		print "\t\t[ Loading graph from file '%s'... ]" % self.filegraph
		self.G = modules.graph.Graph(self.filegraph)
		self.G.load()
		self.__load_nodes()
		self.__load_edges()

	def draw(self):
		# A FAIRE
		#nx.draw_graphviz(self.nxGraph)
		
		### Draw grid view		
		nxGraph = self.__grid_layout()
		pos = nx.get_node_attributes(nxGraph, "posxy")

		print "\t\t[ Drawing nodes... ]"
		nx.draw_networkx_nodes(nxGraph, pos, with_labels=False, node_size = 10, node_color = 'g')		
		print "\t\t[ Drawing edges... ]"
		nx.draw_networkx_edges(nxGraph, pos, width=0.3, alpha=0.4)
		self.__save_draw()

	# Return Networkx graph with a grid layout
	def __grid_layout(self, w = 10):
		nxGraph_grid = copy.deepcopy(self.nxGraph)
		nx.set_node_attributes(nxGraph_grid, "posxy",  [])

		m = find_multiple(self.G.n)
		y_coord = 0

		for i, id_node in enumerate(nxGraph_grid.node):
			if i % m == 0:
				y_coord += w
				x_coord = 1
			else:
				x_coord += w

			nxGraph_grid.node[id_node]["posxy"] = [x_coord, y_coord]

		return nxGraph_grid
	
	def __load_nodes(self):
		for id_node in self.G.nodes:
			self.nxGraph.add_node(id_node)

	def __load_edges(self):
		G = copy.copy(self.G)

		for id_node in G.nodes:
			for id_neighbour in G.nodes[id_node].list_neighbours.keys():
				self.nxGraph.add_edge(id_node, id_neighbour)
				del G.nodes[id_node].list_neighbours[id_neighbour]
				del G.nodes[id_neighbour].list_neighbours[id_node]

	def __save_draw(self):
		print "\t\t[ Saving graph figure... ]"
		plt.axis('off')
		plt.savefig(self.filegraph + ".png", dpi=400)

	def to_gexf(self):
		print "\t\t[ Exporting to Gexf format... ]"
		nx.write_gexf(self.nxGraph, self.filegraph + ".gexf")

from modules.calcModule import *

class DrawBipartite():
	def __init__(self, filegraph, type):
		self.nxGraph = nx.Graph()
		self.filegraph = filegraph
		self.type = type

		print "\t\t[ Loading bipartite graph from file '%s'... ]" % self.filegraph
		simple_load_graph(self.filegraph, self.type)
		self.list_top_nodes = get_top_nodes()
		self.list_bot_nodes = get_bot_nodes()
		self.__load_nodes()
		self.__load_edges()

	def draw(self):
		# Draw classic bipartite view
		nxGraph = self.__classic_bipartite()
		pos = nx.get_node_attributes(nxGraph, "posxy")

		print "\t\t[ Drawing nodes... ]"		
		nx.draw_networkx_nodes(nxGraph, pos, self.list_top_nodes, node_size = 200, node_color = 'g')
		nx.draw_networkx_nodes(nxGraph, pos, self.list_bot_nodes, node_size = 100, node_color = 'r')		
		print "\t\t[ Drawing edges... ]"
		nx.draw_networkx_edges(nxGraph, pos, width=0.5, alpha=0.5)
		self.__save_draw()

	def __classic_bipartite(self):
		nxGraph_classic = copy.deepcopy(self.nxGraph)
		nx.set_node_attributes(nxGraph_classic, "posxy",  [])

		x_coord = 1.0
		y_coord = 10.0
		for id_top_node in self.list_top_nodes:
			nxGraph_classic.node[id_top_node]["posxy"] = [x_coord, y_coord]
			x_coord += 10

		x_coord = 1.0
		y_coord = 1.0
		for id_bot_node in self.list_bot_nodes:
			nxGraph_classic.node[id_bot_node]["posxy"] = [x_coord, y_coord]
			x_coord += 5

		return nxGraph_classic

	def __load_nodes(self):
		for id_top_node in self.list_top_nodes:	
			self.nxGraph.add_node(id_top_node, label="[TOP]")
		for id_bot_node in self.list_bot_nodes:			
			self.nxGraph.add_node(id_bot_node, label="[BOT]")
		
	def __load_edges(self):
		for index_top_node, id_top_node in enumerate(self.list_top_nodes):
			for id_bot_neighbour in get_neighbors(index_top_node):
				self.nxGraph.add_edge(id_top_node, id_bot_neighbour)

	def __save_draw(self):
		print "\t\t[ Saving graph figure... ]"
		plt.axis('off')
		plt.savefig(self.filegraph + ".png", dpi=400)

	def to_gexf(self):
		print "\t\t[ Exporting to Gexf format... ]"
		nx.write_gexf(self.nxGraph, self.filegraph + ".gexf")
