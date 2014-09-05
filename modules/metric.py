import operator
import math, copy, os, sys
import graph
import numpy
try:
	from collections import OrderedDict
except ImportError:
	#import imp
	#OrderedDict = imp.load_source('OrderedDict', 'lib/ordereddict/ordereddict.py')
	sys.path.append('lib/ordereddict')
	from ordereddict import OrderedDict
from modules.calcMetric import *

class Metric(object):
	def __init__(self, graph, name_metric, data_metric):
		# graph attached to this metric
		self.graph = graph
		self.name = name_metric

		# have to be a dictionary
		self.data_metric = copy.copy(data_metric)
		self.size = 0
		self.size_below1 = 0

		self.variance = 0.0
		self.ecart_type = 0.0
		self.median = 0.0

		self.distribution = {}
		self.fraction_distribution = {}
		self.cumulative_distribution = []

	def compile(self):
		self.__clean()

		self.average = 0.0
		self.highest = 0.0
		self.lowest = self.size
		
		self.__compute_basic_stats()
		self.__compute_cumulative_distribution()
		self.__compute_more_stats()

	def __clean(self):
		data_to_delete = []

		for id_node in self.data_metric:
			if self.data_metric[id_node] == -1:
				data_to_delete.append(id_node)

		self.size_below1 = len(data_to_delete)

		for id_node in data_to_delete:
			del self.data_metric[id_node]

		self.size = len(self.data_metric)

	def __compute_basic_stats(self):
		for (id_node, value_metric) in self.data_metric.iteritems():
			if value_metric in self.distribution:
				self.distribution[value_metric] += 1
			else:
				self.distribution[value_metric] = 1

			if self.highest < value_metric:
				self.highest = value_metric
			if value_metric >= 0.0 and self.lowest > value_metric:
				self.lowest = value_metric

			self.average += value_metric
		
		for (value_metric, count) in self.distribution.iteritems():			
			self.fraction_distribution[value_metric] = count / float(self.size)

		sort_data_metric = sorted(self.data_metric.values(), reverse=False)
		
		## COMPUTE MEDIAN
		if len(sort_data_metric) > 2:
			index_middle = (len(sort_data_metric) / 2) + 1
			if len(sort_data_metric) % 2 == 0:
				self.median = (sort_data_metric[index_middle] + sort_data_metric[index_middle - 1]) / float(2)
			else:
				self.median = sort_data_metric[index_middle]
		else:
			self.median = 0.0

		#self.average = (2 * self.average) / float(self.size)
		self.average = self.average / float(self.size)

	def __compute_cumulative_distribution(self):
		sorted_distribution = sorted(self.distribution.iteritems(), key=operator.itemgetter(0), reverse=True)
		cumulative = 0
		
		for (value_metric, count) in sorted_distribution:			
			cumulative += count
			self.cumulative_distribution.append((value_metric, cumulative / float(self.size)))		

	def __compute_more_stats(self):
		for (id_node, value_metric) in self.data_metric.iteritems():
			self.variance += pow(value_metric - self.average, 2)
		self.variance /= float(self.size)
		self.ecart_type = math.sqrt(self.variance)

	def save(self, directory_data):
		f1 = open(directory_data + "/" + self.name + "_distribution" + ".data", 'w')
		for (value_metric, count) in self.distribution.iteritems():
			f1.write(str(value_metric) + ' ' + str(count) + '\n')
		f1.close()
		f1 = open(directory_data + "/" + self.name + "_reverse_cdf" + ".data", 'w')
		for (v1, v2) in self.cumulative_distribution:
			f1.write(str(v1) + ' ' + str(v2) + '\n')
		f1.close()

	def __str__(self):
		info = "#########] %s [#########\n" % self.name
		info += "\t # AVERAGE : %0.6f\n" % self.average
		info += "\t # MAX VALUE : %0.6f\n" % self.highest
		info += "\t # MIN VALUE : %0.6f\n" % self.lowest
		info += "\t # MEDIAN : %0.6f\n" % self.median
		info += "\t # VARIANCE : %0.6f\n" % self.variance
		info += "\t # ECART-TYPE : %0.6f\n" % self.ecart_type

		if 0 in self.distribution:
			info += "\n#### Number of value 0 : %d\n" % (self.distribution[0])
		if 1 in self.distribution:
			info += "\n#### Number of value 1 : %d\n" % (self.distribution[1])
		info += "\n#### Number of value -1 : %d\n" % (self.size_below1)

		info += "[#######################]\n\n"
		return info

	def normalization(self):		
		data_metric = {}

		for (id_node, value_metric) in self.data_metric.iteritems():
			data_metric[id_node] = (value_metric - self.lowest) / float(self.highest - self.lowest)

		metric_norm = Metric(self.graph, "%s_norm" % self.name, data_metric)
		metric_norm.compile()

		return metric_norm

class Correlation(object):
	def __init__(self, graph, metric1, metric2, bipartite = None, mixed = False):
		self.graph = graph
		self.metric1 = metric1
		self.metric2 = metric2
		self.name = "%s-%s" % (self.metric1.name, self.metric2.name)

		self.scatter = []
		self.linear_regression = []
		self.average_curve = []
		self.covariance = 0.0	
		self.correlation_coefficient = 0.0

		if mixed:
			parts = bipartite.split('-')
			if parts[0] == "top":
				nodes_part = self.graph.list_top_nodes
			else:
				nodes_part = self.graph.list_bot_nodes
                   
			data_metric_mixed = {}
			for id_node in nodes_part:
				data_metric_mixed[id_node] = 0.0
				avg_data_metric = 0.0

				for id_neighbour in nodes_part[id_node]:
					if id_neighbour in metric2.data_metric:
						avg_data_metric += metric2.data_metric[id_neighbour]

				if avg_data_metric != 0.0:
					data_metric_mixed[id_node] = avg_data_metric / float(len(nodes_part[id_node]))

				
			metric2 = Metric(graph, metric2.name, data_metric_mixed)
			metric2.compile()

		self.data_metric1, self.data_metric2 =  arrange_metrics(metric1.data_metric, metric2.data_metric)
		self.nodes = dict((key, None) for key in self.data_metric1.keys())

		#self.__arrange_data_metrics(metric1, metric2)

		self.x = numpy.array(self.data_metric1.values())
		self.y = numpy.array(self.data_metric2.values())
			
	def __arrange_data_metrics(self, metric1, metric2):
		self.nodes = {}
		self.data_metric1 = {}
		self.data_metric2 = {}		
		list_id_node_data2 = metric2.data_metric.keys()

		for id_node_data1 in metric1.data_metric.keys():
			for id_node_data2 in list_id_node_data2:
				if id_node_data1 == id_node_data2:
					self.nodes[id_node_data1] = None

					self.data_metric1[id_node_data1] = metric1.data_metric[id_node_data1]
					self.data_metric2[id_node_data2] = metric2.data_metric[id_node_data2]

					list_id_node_data2.remove(id_node_data2)
					break
			if not list_id_node_data2:
				break

	def compile(self):
		if len(self.nodes) == 0:
			print "[ERROR : BIJECTION BETWEEN THE METRICS]"
			return
		self.__compute_scatter()
		self.__comute_linear_regression()
		self.__compute_fitting_curve()
		self.__compute_average_curve()

	def __compute_scatter(self):
		for id_node in self.nodes:
			''' SCATTERPLOT '''
			self.scatter.append((self.data_metric1[id_node], self.data_metric2[id_node]))
			self.covariance += ((self.data_metric1[id_node] - self.metric1.average) * (self.data_metric2[id_node] - self.metric2.average))			
			
		self.covariance /= len(self.nodes)

		if self.metric1.ecart_type != 0.0 and self.metric2.ecart_type != 0.0:
			self.correlation_coefficient = (self.covariance / (self.metric1.ecart_type * self.metric2.ecart_type))

	def __comute_linear_regression(self):
		A = numpy.array([self.x, numpy.ones(len(self.x))])
		w = numpy.linalg.lstsq(A.T, self.y)[0]
		
		self.linear_regression.append(self.x)
		self.linear_regression.append(w[0] * self.x + w[1])

	# Curve fitting the data
	def __compute_fitting_curve(self):		
		z = numpy.polyfit(self.x, self.y, 2)
		p = numpy.poly1d(z)
		xp = numpy.linspace(self.metric1.lowest, self.metric1.highest, len(self.data_metric1) / 4)
		dict_average_curve = dict(zip(xp, p(xp)))

		self.fitting_curving = sorted(dict_average_curve.iteritems(), key=operator.itemgetter(0), reverse=False)

	def __compute_average_curve(self):
		data_metric1 = OrderedDict(sorted(self.data_metric1.iteritems(), key=operator.itemgetter(1)))

		### POWER SPACE ###
		nb_step = 25
		# minimal value for alpha
		amin = float(self.metric1.lowest)
		# maximal value for alpha
		amax = float(self.metric1.highest)

		ALPHA_TMP = numpy.zeros(nb_step)
		for i in range(nb_step):
			ALPHA_TMP[i] = amax * (amin / amax) ** (i / (nb_step - 1.0))
		ALPHA = []
		for i in range(nb_step):
			ALPHA.append(ALPHA_TMP[nb_step - (i + 1)])
		
		list_values_average = {}
		
		step = 0.0
		i = 0
		while step < self.metric1.highest:
			value_step = ALPHA[i]
			list_delete = []
			
			count = 0
			avg_value = 0.0

			for (key_metric1, value_metric1) in data_metric1.iteritems():
				if value_metric1 > step:
					break

				avg_value += self.data_metric2[key_metric1]
				count += 1
				list_delete.append(key_metric1)

			if count > 0:
				list_values_average[step] = avg_value / float(count)
				
				for id_node in list_delete:
					del data_metric1[id_node]
					
			step = ALPHA[i]
			i += 1

		self.average_curve = sorted(list_values_average.iteritems(), key=operator.itemgetter(0), reverse=True)

	def save(self, directory_data):
		f1 = open(directory_data + "/" + self.name + "_scatter.data", 'w')
		for (value_metric1, value_metric2) in self.scatter:
			f1.write("%0.6f %0.6f\n" % (value_metric1, value_metric2))
		f1.close()
		if len(self.linear_regression) > 0:		
			f1 = open(directory_data + "/" + self.name + "_linear_regression" + ".data", 'w')
			i = 0
			while i < len(self.linear_regression[0]):
				f1.write("%0.6f %0.6f\n" % (self.linear_regression[0][i], self.linear_regression[1][i]))
				i += 1
			f1.close()
		f1 = open(directory_data + "/" + self.name + "_avg_curve" + ".data", 'w')
		for (value_ac1, value_ac2) in self.average_curve:
			f1.write("%0.6f %0.6f\n" % (value_ac1, value_ac2))
		f1.close()
		f1 = open(directory_data + "/" + self.name + "_fitting_curve" + ".data", 'w')
		for (value_ac1, value_ac2) in self.fitting_curving:
			f1.write("%0.6f %0.6f\n" % (value_ac1, value_ac2))
		f1.close()

	def __str__(self):
		info = "Correlation between %s and %s\n" % (self.metric1.name, self.metric2.name)
		info += "Covariance : %0.6f\n" % (self.covariance)
		info += "Correlation coefficient : %0.6f" % (self.correlation_coefficient)
		info += "\n------\n"
		return info
