#! /usr/bin/python

import sys, os, getopt
import modules.extra, imp
from modules.plot import Plot
brewer2mpl = imp.load_source('brewer2mpl', 'lib/brewer2mpl/brewer2mpl.py')

### GLOBAL VARIABLES ###
nodes_top = []
nodes_bot = []

parts = ["top", "bot"]
metrics = []
correlations = []
metrics_log = []
colorset = brewer2mpl.get_map('Set1', 'qualitative', 8).mpl_colors
list_markers = ["o", "v", "D", "x", "*", "h", "4"]
########################

def usage():
	usage = "Usage : plot_metrics.py -d BIPARTITE_DIRECTORY\n\n"
	usage += "Option\tLong option\tDescription\n"
	usage += "-h\t--help\t\tDisplay this message\n"
	usage += "-d\t--directory\t\tDirectory where metrics are saved <[MANDATORY]>\n"
	print usage

def create_directories(directory):
	directory_plot = directory + "/plots"
	if not os.path.isdir(directory_plot):
		os.mkdir(directory_plot)

	#### METRICS ####
	if not os.path.isdir(directory + "/plots/metrics"):
		os.mkdir(directory + "/plots/metrics")
	directory_plot_metrics_top = directory_plot + "/metrics/top"
	if not os.path.isdir(directory_plot_metrics_top):
		os.mkdir(directory_plot_metrics_top)
	directory_plot_metrics_bot = directory_plot + "/metrics/bot"
	if not os.path.isdir(directory_plot_metrics_bot):
		os.mkdir(directory_plot_metrics_bot)

	#### CORRELATIONS ####
	if not os.path.isdir(directory + "/plots/correlations"):
		os.mkdir(directory + "/plots/correlations")
	directory_plot_correlations_top = directory_plot + "/correlations/top"
	if not os.path.isdir(directory_plot_correlations_top):
		os.mkdir(directory_plot_correlations_top)
	directory_plot_correlations_bot = directory_plot + "/correlations/bot"
	if not os.path.isdir(directory_plot_correlations_bot):
		os.mkdir(directory_plot_correlations_bot)

	return directory_plot_metrics_top, directory_plot_metrics_bot, directory_plot_correlations_top, directory_plot_correlations_bot

def plot_metrics(directory_metrics, parts_directory_plot_metrics, legend):	
	print "\t\t[ Plotting metrics... ]"

	for metric in metrics:
		for i in range(0, 2):
			d = directory_metrics + "/" + parts[i] + "/" + metric + "_" + parts[i]

			print "\tPlotting %s ..." % d

			if not os.path.isdir(d):
				print "[%s directory not found]" % d
			else:
				xlog = False
				ylog = False
				if metric in metrics_log:
					xlog = True
					ylog = True				
				
				########################
				# PLOT DISTRIBUTION
				########################
				data_distribution = {}
				with open(d + "/" + metric + "_" + parts[i] + "_distribution.data", 'r') as file:
					for line in file:						
						line = line.replace("\n", "").split()
						data_distribution[modules.extra.num(line[0])] = modules.extra.num(line[1])
				
				title = ""
				xlabel = metric
				ylabel = ""
				plot_file = parts_directory_plot_metrics[i] + "/" + xlabel + "_distribution"
				plot_parameters = dict(marker="o", markersize=11, linewidth=0, alpha=0.5, color=colorset[0])

				if data_distribution:
					P = Plot(plot_file, title, xlabel, ylabel, xlog, ylog, [legend])
					P.plot_single(data_distribution, plot_parameters)
				else:
					print "[Impossible to plot because no distribution data]"

				########################

				########################
				# PLOT CDF
				########################
				data_reverse_cdf = {}
				with open(d + "/" + metric + "_" + parts[i] + "_reverse_cdf.data", 'r') as file:
					for line in file:
						line = line.replace("\n", "").split()
						data_reverse_cdf[modules.extra.num(line[0])] = modules.extra.num(line[1])
				
				title = ""
				xlabel = metric
				ylabel = ""
				plot_file = parts_directory_plot_metrics[i] + "/" + xlabel + "_reverse_cdf"
				plot_parameters = dict(linestyle="-", marker="s", markersize=8, linewidth=2.5, markeredgecolor=colorset[0], color=colorset[0])
				
				if data_reverse_cdf:
					P = Plot(plot_file, title, xlabel, ylabel, False, False, [legend], formatter=True)
					P.plot_single(data_reverse_cdf, plot_parameters)
				else:
					print "[Impossible to plot because no reverse CDF data]"
				########################

def plot_correlations(directory_correlations, parts_directory_plot_correlations, legend):
	print "\t\t[ Plotting correlations... ]"

	for correlation in correlations:
		metrics = correlation.split("-")
		for i in range(0, 2):
			correlation = metrics[0] + "_" + parts[i] + "-" + metrics[1] + "_" + parts[i]
			d = directory_correlations + "/" + parts[i] + "/" + correlation

			if not os.path.isdir(d):
				print "[%s directory not found]" % d
			else:
				xlog = False
				ylog = False
				if metrics[0] in metrics_log:
					xlog = True			
				if metrics[1] in metrics_log:
					ylog = True

				print "\tPlotting %s ..." % d
				
				list_plot_parameters = []
				list_data = []

				########################
				# SCATTER POINTS 
				########################
				data_scatter = {}
				with open(d + "/" + correlation + "_scatter.data", 'r') as file:
					for line in file:
						line = line.replace("\n", "").split()
						data_scatter[modules.extra.num(line[0])] = modules.extra.num(line[1])
				highest = max(data_scatter.values())
				list_plot_parameters.append(dict(marker="o", markersize=3, linewidth=0, alpha=0.5, color=colorset[0]))
				list_data.append(data_scatter)

				########################
				# CURVE FITTING (AVG)
				########################
				data_avg_curve = {}
				with open(d + "/" + correlation + "_avg_curve.data", 'r') as file:
					for line in file:
						line = line.replace("\n", "").split()
						data_avg_curve[modules.extra.num(line[0])] = modules.extra.num(line[1])
				list_plot_parameters.append(dict(linestyle="--", marker="s", markersize=8, linewidth=2.5, color=colorset[1]))
				list_data.append(data_avg_curve)

				'''data_linear = {}
				with open(d + "/" + correlation + "_linear_regression.data", 'r') as file:
					for line in file:
						line = line.replace("\n", "").split()
						if line[0] in nodes or not nodes:
							data_linear[num(line[0])] = num(line[1])
				list_plot_parameters.append(dict(linestyle="-", marker="o", markersize=6, linewidth=1.5, alpha=0.75))
				list_data.append(data_linear)'''

				'''data_fitting_curve = {}
				with open(d + "/" + correlation + "_fitting_curve.data", 'r') as file:
					for line in file:
						line = line.replace("\n", "").split()
						if line[0] in nodes or not nodes:
							data_fitting_curve[num(line[0])] = num(line[1])
				list_plot_parameters.append(dict(linestyle="-", marker="o", markersize=6, linewidth=1.5, alpha=0.75))
				list_data.append(data_fitting_curve)'''

				title = ""
				xlabel = metrics[0]
				ylabel = metrics[1]
				plot_file = parts_directory_plot_correlations[i] + "/" + correlation

				try:
					P = Plot(plot_file, title, xlabel, ylabel, True, True, [legend], ylimit=[0, highest])
					P.plot_multiple(list_data, list_plot_parameters)
				except Exception, e:
					print "Error during plotting process : %s" % e

def main(argv):
	try:
		opts, args = getopt.getopt(argv, "hd:", ["help", "directory="])
	except getopt.GetoptError, e:
		usage()
		print "Error >>> %s" % str(e)
		sys.exit(2)

	directory = ""
	node_file = ""
	part_node_file = ""

	for opt, arg in opts:
		if opt in ("-h", "--help"):
			usage()
		elif opt in ("-d", "--directory"):
			directory = arg
			if directory[len(directory) - 1] == "/":
				directory = directory[:len(directory) - 1]

	### CHECK IF DIRECTORIES ARE CORRECT 
	if not os.path.isdir(directory):
		usage()
		print "Bad directory location : '%s'" % directory
		sys.exit(2)
	directory_metrics = directory + "/metrics"
	if not os.path.isdir(directory_metrics):
		usage()
		print "Error metrics directory doesn't exist"
		sys.exit(2)
	directory_correlations = directory + "/correlations"
	if not os.path.isdir(directory_correlations):
		usage()
		print "Error correlations directory doesn't exist"
		sys.exit(2)

	#legend = directory.split("/")[0]
	legend = directory.split("/")
	legend = legend[len(legend)-1]

	### CREATE DIRECTORIES
	directory_plot_metrics_top, directory_plot_metrics_bot, directory_plot_correlations_top, directory_plot_correlations_bot = create_directories(directory)
	parts_directory_plot_metrics = [directory_plot_metrics_top, directory_plot_metrics_bot]
	parts_directory_plot_correlations = [directory_plot_correlations_top, directory_plot_correlations_bot]	

	global metrics, correlations, metrics_log
	metrics, correlations, metrics_log = modules.extra.get_plotting_configurations()

	plot_metrics(directory_metrics, parts_directory_plot_metrics, legend)
	plot_correlations(directory_correlations, parts_directory_plot_correlations, legend)

if __name__  == "__main__":
	main(sys.argv[1:])
