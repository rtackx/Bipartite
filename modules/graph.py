import os, copy
import modules.node
import modules.extra

class Graph(object):
	def __init__(self, graphfile):
		self.graphfile = graphfile
		self.nodes = {}
		
		self.n = 0
		self.m = 0
		self.density = 0.0
		
		self.list_local_degree = {}
		self.list_local_cc = {}
		self.list_local_rc = {}

		self.list_metrics = {}
		self.list_correlations = {}

	def compute_metrics(self):
		self.__compute_degree_informations()				
		self.__compute_cc_informations()
		self.__compute_rc_informations()

	def create_metrics(self):
		self.list_metrics["degree"] = metric.Metric(self, "degree", self.list_local_degree)
		self.list_metrics["degree"].compile()
		#self.list_metrics["degree_norm"] = self.list_metrics["degree"].normalization()

		self.list_metrics["cc"] = metric.Metric(self, "cc", self.list_local_cc)
		self.list_metrics["cc"].compile()

		self.list_metrics["rc"] = metric.Metric(self, "rc", self.list_local_rc)
		self.list_metrics["rc"].compile()

	def treat_correlations(self):
		for metric1 in self.list_metrics:
			for metric2 in self.list_metrics:
				if metric1 != metric2:			
					name_correlation = "%s-%s" % (metric1, metric2)
					self.list_correlations[name_correlation] = metric.Correlation(self, self.list_metrics[metric1], self.list_metrics[metric2])
					self.list_correlations[name_correlation].compile()
		
	def __compute_degree_informations(self):
		for id_node in self.nodes:
			degree = self.nodes[id_node].degree
			self.list_local_degree[id_node] = degree
			self.m += degree

		self.m /= 2
		self.average_degree = (2 * self.m) / float(self.n)
		self.density = (2 * self.m) / float(self.n * (self.n - 1))
	
	def __compute_cc_informations(self):
		for id_node in self.nodes:
			self.list_local_cc[id_node] = -1.0
			k = self.nodes[id_node].degree * (self.nodes[id_node].degree - 1)
			# ALTERNATIVE METHOD (deprecated degree 1 nodes)
			'''k = 0
			for id_neighbour in self.nodes[id_node].list_neighbours:
				if self.nodes[id_neighbour].degree > 1:
					k += 1
			k = k * (k - 1)	'''

			v = 0
			for id_neighbour1 in self.nodes[id_node].list_neighbours:
				for id_neighbour2 in self.nodes[id_node].list_neighbours:
					if id_neighbour1 != id_neighbour2:						
						if (id_neighbour2 in self.nodes[id_neighbour1].list_neighbours):
							v += 1

			if k != 0 and v != 0:
				self.list_local_cc[id_node] = v / float(k)
		
	def __compute_rc_informations(self):	
		for id_node in self.nodes:
			self.list_local_rc[id_node] = -1.0
			k = self.nodes[id_node].degree * (self.nodes[id_node].degree - 1)
			# ALTERNATIVE METHOD (deprecated degree 1 nodes)
			'''k = 0
			for id_neighbour in self.nodes[id_node].list_neighbours:
				if self.nodes[id_neighbour].degree > 1:
					k += 1
			k = k * (k - 1)	'''
			v = 0
			
			for id_neighbour1 in self.nodes[id_node].list_neighbours:
				for id_neighbour2 in self.nodes[id_node].list_neighbours:
					if id_neighbour2 != id_neighbour1:
						for id_neighbour_id1 in self.nodes[id_neighbour1].list_neighbours:
							for id_neighbour_id2 in self.nodes[id_neighbour2].list_neighbours:
								if id_neighbour_id1 != id_node and id_neighbour_id1 == id_neighbour_id2:
									v += 0.5
									break
							else:
								continue
							break
			
			if k != 0 and v != 0:
				self.list_local_rc[id_node] = (2*v) / float(k)
	
	def load(self):
		file = open(self.graphfile, 'r')

		# A AMERLIORER
		for line in file.read().splitlines():
			nodes_id = line.split()

			if nodes_id[0] not in self.nodes:
				self.nodes[nodes_id[0]] = modules.node.Node(nodes_id[0])

			for i in range(1, len(nodes_id)):			
				self.nodes[nodes_id[0]].add_neighbour(nodes_id[i])

		file.close()
		self.n = len(self.nodes)
	
	def informations(self):
		info = "\t\t#### [Statistics of graph (%s)] ####\n\n" % self.name
		info += "- GLOBAL STATS :\n\n"
		info += "\t# Size (n) = %d\n" % self.n
		info += "\t# Number of links (m) = %d\n" % self.m				
		info += "\t# Density = %0.6f\n" % self.density

		info += "\n- LOCAL METRICS :\n\n"
		for metric_name in self.list_metrics:
			info += str(self.list_metrics[metric_name])

		return info

	def informations_correlations(self):
		info = "\n\t ---- Correlations ---- \n"
		for correlation_name in self.list_correlations:
			info += str(self.list_correlations[correlation_name])

		return info

	def save_local_informations(self, filename):
		f = open(filename, 'w')

		com = "#ID\tDegree\tClustering coefficient\tRedundancy coefficient"
		if isinstance(self, graph_bipartite.Bipartite):
			com += "\tBipartite TOP(0)/BOT(1) part"
		com +="\n"
		f.write(com)

		for id_node in self.nodes:
			line = "%s\t%d\t%0.6f\t%0.6f" % (id_node, self.list_local_degree[id_node], self.list_local_cc[id_node], self.list_local_rc[id_node])
			if isinstance(self, graph_bipartite.Bipartite):
				if id_node in self.list_top_nodes:
					line += "\t0"
				else:
					line += "\t1"
			line +="\n"
			f.write(line)

		f.close()

	def save_metrics(self, directory_data):
		if not os.path.isdir(directory_data):
			os.mkdir(directory_data)

		for metric_name in self.list_metrics:
			self.list_metrics[metric_name].save(directory_data)

	def save_correlations(self, directory_data):
		directory_correlation = directory_data + "/correlations"
		if not os.path.isdir(directory_correlation):
			os.mkdir(directory_correlation)

		for correlation_name in self.list_correlations:
			self.list_correlations[correlation_name].save(directory_correlation)

	def __str__(self):		
		return self.informations()
