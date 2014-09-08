#! /usr/bin/python

import sys, os, getopt
from modules.graph_bipartite import Bipartite

def usage():
	usage = "Usage : metrics_bipartite.py -d BIPARTITE_DIRECTORY \n\n"
	usage += "Option\tLong option\tDescription\n"
	usage += "-h\t--help\t\tDisplay this message\n"
	usage += "-d\t--directory\t\tDirectory where data are saved <[MANDATORY]>\n"
	print usage	

def main(argv):
	try:
		opts, args = getopt.getopt(argv, "hd:", ["help", "directory="])
	except getopt.GetoptError, e:
		usage()
		print "Error >>> %s" % str(e)
		sys.exit(2)

	directory = ""

	for opt, arg in opts:
		if opt in ("-h", "--help"):
			usage()
		elif opt in ("-d", "--directory"):
			directory = arg
			if directory[len(directory) - 1] == "/":
				directory = directory[:len(directory) - 1]
			
	if not os.path.isdir(directory):
		usage()
		print "Bad directory location : %s" % directory
		sys.exit(2)

	print "\t\t[ Loading data... ]"
	GB = Bipartite(directory)
	GB.open()
	print "\t\t[ Creating metrics... ]"
	GB.create_metrics()
	GB.save_metrics()
	
	print "\t\t[ Creating correlations... ]"
	GB.create_directories()
	GB.treat_some_correlations()
	
	#GB.treat_all_correlations()
	#GB.save_correlations()

if __name__  == "__main__":
	main(sys.argv[1:])
