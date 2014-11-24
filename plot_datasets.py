#! /usr/bin/python

import sys, os, getopt
from modules.plot import Plot
import modules.extra, imp
brewer2mpl = imp.load_source('brewer2mpl', 'lib/brewer2mpl/brewer2mpl.py')

### GLOBAL VARIABLES ###
parts = ["top", "bot"]
paired = False
metrics = []
correlations = []
metrics_log = []
list_markers = ["o", "v", "D", "x", "*", "h", "4"]

## COLOR SETS ##
colorset = []
colorset_paired = []

########################

def usage():
	usage = "Usage : plot_metrics.py -D MAIN_DIRECTORY\n\n"
	usage += "Option\tLong option\tDescription\n"
	usage += "-h\t--help\t\tDisplay this message\n"
	usage += "-D\t--directories\t\tMain directory containing other directories of dataset results <[MANDATORY]>\n"
	usage += "-d\t--paired\t\tPlot an other main directory with -D option\n"
	print usage	

def set_colors(n):	
	global colorset, colorset_paired
	colorset = brewer2mpl.get_map("Set1", "qualitative", n).mpl_colors
	colorset_paired = brewer2mpl.get_map("Paired", "qualitative", n * 2, reverse=True).mpl_colors

	# switch color to get clearer color for paired dataset	
	for i in range(0, n):
		t = colorset_paired[i + 1]
		colorset_paired[i + 1] = colorset_paired[i]
		colorset_paired[i] = t
		i += 1

def create_directories(main_directory):
	global paired
	if paired:
		directory_dest = main_directory + "/multiple_plots_paired"
	else:
		directory_dest = main_directory + "/multiple_plots"

	if not os.path.isdir(directory_dest):
		os.mkdir(directory_dest)
	directory_dest_metrics = directory_dest + "/metrics"
	if not os.path.isdir(directory_dest_metrics):
		os.mkdir(directory_dest_metrics)
	if not os.path.isdir(directory_dest_metrics + "/top"):
		os.mkdir(directory_dest_metrics + "/top")
	if not os.path.isdir(directory_dest_metrics + "/bot"):
		os.mkdir(directory_dest_metrics + "/bot")

	directory_dest_correlations = directory_dest + "/correlations"
	if not os.path.isdir(directory_dest_correlations):
		os.mkdir(directory_dest_correlations)
	if not os.path.isdir(directory_dest_correlations + "/top"):
		os.mkdir(directory_dest_correlations + "/top")
	if not os.path.isdir(directory_dest_correlations + "/bot"):
		os.mkdir(directory_dest_correlations + "/bot")

	return directory_dest_metrics, directory_dest_correlations

def plot_metrics(main_directory, list_directories, directory_dest_metrics, main_directory_paired = "", list_directories_paired = []):
	print "\t\t[ Plotting metrics... ]"	

	for metric in metrics:
		for j in range(0, 2):
			m = metric + "_" + parts[j]

			xlog = False
			ylog = False
			if metric in metrics_log:
				xlog = True
				ylog = True

			print "\tPlotting %s ..." % m
						
			xlabel = parts[j].title() + " " + metric
			
			########################
			# PLOT DISTRIBUTION
			########################
			list_plot_parameters = []
			list_data = []
			list_legend = []
			
			i = 0
			ci = 0
			for directory in list_directories:
				data_distribution = {}
				with open(main_directory + "/" + directory + "/metrics/" + parts[j] + "/" + m + "/" + m + "_distribution.data", 'r') as file:
					for line in file:
						line = line.replace("\n", "").split()
						data_distribution[modules.extra.num(line[0])] = modules.extra.num(line[1])
				list_data.append(data_distribution)
				if paired:
					list_plot_parameters.append(dict(marker=list_markers[i], markersize=12, linewidth=0, alpha=0.5, color=colorset_paired[ci]))
				else:
					list_plot_parameters.append(dict(marker=list_markers[i], markersize=12, linewidth=0, alpha=0.5, color=colorset[i]))
				list_legend.append(directory)
				
				if paired:
					data_distribution = {}
					with open(main_directory_paired + "/" + list_directories_paired[i] + "/metrics/" + parts[j] + "/" + m + "/" + m + "_distribution.data", 'r') as file:
						for line in file:
							line = line.replace("\n", "").split()
							data_distribution[modules.extra.num(line[0])] = modules.extra.num(line[1])
					list_data.append(data_distribution)
					list_plot_parameters.append(dict(marker=list_markers[i], markersize=12, linewidth=0, alpha=0.5, color=colorset_paired[ci + 1]))
					list_legend.append(list_directories_paired[i])
				
				i += 1
				ci += 2
			
			title = ""
			ylabel = ""
			plot_file = directory_dest_metrics + "/" + parts[j] + "/distribution_" + m

			P = Plot(plot_file, title, xlabel, ylabel, xlog, ylog, list_legend)
			P.plot_multiple(list_data, list_plot_parameters)
			########################

			########################
			# PLOT CDF
			########################
			list_plot_parameters = []
			list_data = []
			list_legend = []
			
			i = 0
			ci = 0
			for directory in list_directories:
				data_reverse_cdf = {}
				with open(main_directory + "/" + directory + "/metrics/" + parts[j] + "/" + m + "/" + m + "_reverse_cdf.data", 'r') as file:
					for line in file:
						line = line.replace("\n", "").split()
						data_reverse_cdf[modules.extra.num(line[0])] = modules.extra.num(line[1])
				list_data.append(data_reverse_cdf)
				if paired:				
					list_plot_parameters.append(dict(linestyle="-", marker=list_markers[i], markersize=14, linewidth=2.5, alpha=0.5, markeredgecolor=colorset_paired[ci], color=colorset_paired[ci]))
				else:
					list_plot_parameters.append(dict(linestyle="-", marker=list_markers[i], markersize=14, linewidth=2.5, alpha=0.5, markeredgecolor=colorset[i], color=colorset[i]))
				list_legend.append(directory)				

				if paired:
					data_reverse_cdf = {}
					with open(main_directory_paired + "/" + list_directories_paired[i] + "/metrics/" + parts[j] + "/" + m + "/" + m + "_reverse_cdf.data", 'r') as file:
						for line in file:
							line = line.replace("\n", "").split()
							data_reverse_cdf[modules.extra.num(line[0])] = modules.extra.num(line[1])
					list_data.append(data_reverse_cdf)
					list_plot_parameters.append(dict(linestyle="-", marker=list_markers[i], markersize=14, linewidth=2.5, alpha=0.5, markeredgecolor=colorset_paired[ci + 1], color=colorset_paired[ci + 1]))
					list_legend.append(list_directories_paired[i])

				i += 1
				ci += 2

			title = ""
			ylabel = ""
			plot_file = directory_dest_metrics + "/" + parts[j] + "/reverse_cdf_" + m

			P = Plot(plot_file, title, xlabel, ylabel, False, False, list_legend, formatter=True)
			P.plot_multiple(list_data, list_plot_parameters)
			########################

def plot_correlations(main_directory, list_directories, directory_dest_correlations, main_directory_paired = "", list_directories_paired = []):
	print "\t\t[ Plotting correlations... ]"

	for correlation in correlations:
		metrics = correlation.split("-")
		for j in range(0, 2):
			correlation = metrics[0] + "_" + parts[j] + "-" + metrics[1] + "_" + parts[j]

			xlog = False
			ylog = False
			if metrics[0] in metrics_log:
				xlog = True			
			if metrics[1] in metrics_log:
				ylog = True

			print "\tPlotting %s ..." % correlation

			xlabel = parts[j].title() + " " + metrics[0]
			ylabel = parts[j].title() + " " + metrics[1]

			list_plot_parameters = []
			list_data = []
			list_data_avg_curve = []
			list_plot_parameters_avg_curve = []
			list_legend = []
			highest = 0.0

			i = 0
			ci = 0
			for directory in list_directories:
				maximum = 0.0
				data_scatter = {}
				with open(main_directory + "/" + directory + "/correlations/" + parts[j] + "/" + correlation + "/" + correlation + "_scatter.data", 'r') as file:
					for line in file:
						line = line.replace("\n", "").split()
						data_scatter[modules.extra.num(line[0])] = modules.extra.num(line[1])
				maximum = max(data_scatter.values())
				if highest < maximum:
					highest = maximum				
				if paired:
					list_plot_parameters.append(dict(marker=list_markers[i], markersize=8, linewidth=0, alpha=0.5, color=colorset_paired[ci]))
				else:
					list_plot_parameters.append(dict(marker=list_markers[i], markersize=8, linewidth=0, alpha=0.5, color=colorset[i]))
				list_data.append(data_scatter)
								
				data_avg_curve = {}
				with open(main_directory + "/" + directory + "/correlations/" + parts[j] + "/" + correlation + "/" + correlation + "_avg_curve.data", 'r') as file:
					for line in file:
						line = line.replace("\n", "").split()
						data_avg_curve[modules.extra.num(line[0])] = modules.extra.num(line[1])
				if paired:					
					list_plot_parameters_avg_curve.append(dict(linestyle="--", marker=list_markers[i], markersize=13, linewidth=2.5, color=colorset_paired[ci]))
				else:
					list_plot_parameters_avg_curve.append(dict(linestyle="--", marker=list_markers[i], markersize=13, linewidth=2.5, color=colorset[i]))
				
				if data_avg_curve:
					list_data_avg_curve.append(data_avg_curve)
				else:
						print "[Impossible to retrieve average curve data from '%s']" % directory

				list_legend.append(directory)

				if paired:
					maximum = 0.0
					data_scatter = {}
					with open(main_directory_paired + "/" + list_directories_paired[i] + "/correlations/" + parts[j] + "/" + correlation + "/" + correlation + "_scatter.data", 'r') as file:
						for line in file:
							line = line.replace("\n", "").split()
							data_scatter[modules.extra.num(line[0])] = modules.extra.num(line[1])
					maximum = max(data_scatter.values())
					if highest < maximum:
						highest = maximum
					list_plot_parameters.append(dict(marker=list_markers[i], markersize=8, linewidth=0, alpha=0.5, color=colorset_paired[ci + 1]))
					list_data.append(data_scatter)

					data_avg_curve = {}
					with open(main_directory_paired + "/" + list_directories_paired[i] + "/correlations/" + parts[j] + "/" + correlation + "/" + correlation + "_avg_curve.data", 'r') as file:
						for line in file:
							line = line.replace("\n", "").split()
							data_avg_curve[modules.extra.num(line[0])] = modules.extra(line[1])
					list_plot_parameters_avg_curve.append(dict(linestyle="--", marker=list_markers[i], markersize=13, linewidth=2.5, color=colorset_paired[ci + 1]))
					
					if data_avg_curve:
						list_data_avg_curve.append(data_avg_curve)
					else:
						print "[Impossible to retrieve average curve data from %s]" % list_directories_paired[i]

					list_legend.append(list_directories_paired[i])
					
				i += 1
				ci += 2

			title = ""
			plot_file = directory_dest_correlations + "/" + parts[j] + "/" + correlation

			list_data.extend(list_data_avg_curve)
			list_plot_parameters.extend(list_plot_parameters_avg_curve)
			
			P = Plot(plot_file, title, xlabel, ylabel, xlog, ylog, list_legend, ylimit=[0, highest])
			P.plot_multiple(list_data, list_plot_parameters)
				
def main(argv):
	try:
		opts, args = getopt.getopt(argv, "hD:xyp:", ["help", "directories=", "xlog", "ylog", "paired="])
	except getopt.GetoptError, e:
		usage()
		print "Error >>> %s" % str(e)
		sys.exit(2)

	main_directory = ""
	main_directory_paired = ""
	global paired

	for opt, arg in opts:
		if opt in ("-h", "--help"):
			usage()
		elif opt in ("-D", "--directories"):
			main_directory = arg
			if main_directory[len(main_directory) - 1] == "/":
				main_directory = main_directory[:len(main_directory) - 1]
		elif opt in ("-x", "--xlog"):
			xlog_main = True
		elif opt in ("-y", "--ylog"):
			ylog_main = True
		elif opt in ("-p", "--paired"):
			paired = True
			main_directory_paired = arg
			if main_directory_paired[len(main_directory_paired) - 1] == "/":
				main_directory_paired = main_directory_paired[:len(main_directory_paired) - 1]		
	
	list_directories = []
	if not os.path.isdir(main_directory):
		usage()
		print "Bad main directory location : '%s'" % main_directory
		sys.exit(2)	
	for directory in sorted(os.listdir(main_directory)):
		if "multiple_plots" not in directory:
			list_directories.append(directory)

	list_directories_paired = []
	if paired:
		if not os.path.isdir(main_directory_paired):
			usage()
			print "Bad paired directory location : '%s'" % main_directory_paired
			sys.exit(2)
		for directory in sorted(os.listdir(main_directory_paired)):
			if "multiple_plots" not in directory:
				list_directories_paired.append(directory)
	
	directory_dest_metrics, directory_dest_correlations = create_directories(main_directory)

	global metrics, correlations, metrics_log
	metrics, correlations, metrics_log = modules.extra.get_plotting_configurations()
	set_colors(len(list_directories))
	
	plot_metrics(main_directory, list_directories, directory_dest_metrics, main_directory_paired, list_directories_paired)
	plot_correlations(main_directory, list_directories, directory_dest_correlations, main_directory_paired, list_directories_paired)

if __name__  == "__main__":
	main(sys.argv[1:])
