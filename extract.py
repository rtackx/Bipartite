#! /usr/bin/python

import re

class extract_feature(object):
	# rule is a conditional python-oriented statement
	def __init__(self, directory, part, rules):
		self.directory = directory
		self.part = part
		self.rules = rules
		
		self.metrics = []
		self.operators = []
		self.nodes = set()
		self.__decompose_rules()
	
	# retrieve metrics and operators from rules
	def __decompose_rules(self):
		regex = re.compile("\((.+?)\s*([=<>]{1,2})\s*(.+?)\)")
		self.metrics = regex.findall(self.rules)
		regex = re.compile("\(.+\)\s*(.*)\s*")
		self.operators = regex.findall(self.rules)

	def proceed(self):
		list_nodes = []
		for metric in self.metrics:
			print "Proceeding for metric : %s" % (' '.join(metric))
			nodes = set()
			f = open(self.directory + "/data/" + self.part + "/" + metric[0] + "_" + self.part + ".data", "r")
			for line in f:
				line = line.replace("\n", "").split()
				exp = line[1] + " " +  metric[1] + " " + metric[2]

				if eval(exp):
					nodes.add(line[0])
			list_nodes.append(nodes)

		self.nodes = list_nodes[0]
		for i in range(1, len(list_nodes)):
			if self.operators[i-1] == "and":
				self.nodes = self.nodes.intersection(list_nodes[i])
			else:
				self.nodes = self.nodes.extend(list_nodes[i])

	def save_nodes(self):
		f = open(self.directory + "nodes_" + self.rules, "w")
		for node in self.nodes:
			f.write(node + "\n")
		f.close()

if __name__  == "__main__":
	directory = "Results/com-lj/dir_Datasets_com-lj.all.cmty.txt"
	E = extract_feature(directory, "top", "(cc == 1.0)")
	E.proceed()
	E.save_nodes()
