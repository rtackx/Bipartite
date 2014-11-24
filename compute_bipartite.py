#! /usr/bin/python
import datetime, getopt, sys, os
from modules.calcModule import *

def usage():
	usage = "Usage : compute_bipartite.py -f GRAPH_FILE [OPTION] \n\n"
	usage += "Option\tLong option\tDescription\n"
	usage += "-h\t--help\t\tDisplay this message\n"
	usage += "-f\t--file\t\tRequiered a graph file <[MANDATORY]>\n"
	usage += "-r\t--random\t\tDraw edges randomly following the fixed degree distribution\n"
	usage += "-t\t--type\t\tIndicate if the data file contains identifiers (usually the node ID) at the beginning of each line <Default 0[NO], 1[YES]>\n"
	usage += "-l\t--limit\t\tMaximum number of proximity nodes (in order to avoid memory overloading) <Default 100k>\n"
	print usage	

def main(argv):
	try:
		opts, args = getopt.getopt(argv, "hf:t:l:r", ["help", "file=", "type=", "limit=", "random"])
	except getopt.GetoptError, e:
		usage()
		print "Error >>> %s" % str(e)
		sys.exit(2)
	
	# VARIABLES
	datafile = ""
	datatype = 0
	# limit for size of neighbors
	limit = 100000
	random = 0

	for opt, arg in opts:
		if opt in ("-h", "--help"):
			usage()
			sys.exit(1)
		elif opt in ("-f", "--file"):
			datafile = arg
		elif opt in ("-t", "--type"):
			datatype = int(arg)
		elif opt in ("-l", "--limit"):
			limit = int(arg)
		elif opt in ("-r", "--random"):
			random = 1

	if not os.path.isfile(datafile):
		usage()
		print "Data file '%s' doesn't exist" % datafile 
		sys.exit(1)		
	if datatype not in (0, 1):
		usage()
		print "Bad type..."
		sys.exit(1)
	
	date_begin = datetime.datetime.now()
	print "====== Started at : %s ======" % date_begin

	graph_name = datafile.split("/")
	graph_name = "_".join(graph_name)
	graph_directory = "dir_"
	graph_directory += graph_name
	if random:
		graph_directory += "_random"	

	compute_bipartite(graph_directory, datafile, datatype, random, limit)	
	
	date_end = datetime.datetime.now()
	print "====== Time elapsed : %s ======" % (date_end - date_begin)

if __name__  == "__main__":	
	main(sys.argv[1:])
