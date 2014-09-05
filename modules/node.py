class Node:
	def __init__(self, id_node):
		self.id_node = id_node
		self.degree = 0
		self.list_neighbours = {}
		
	def add_neighbour(self, id_neighbour):
		if id_neighbour not in self.list_neighbours:			
			#self.list_neighbours.append(id_neighbour)
			self.list_neighbours[id_neighbour] = None
			self.degree += 1
		
	def __str__(self):
		return "Node %s [degree : %d]" % (self.id_node, self.degree)
