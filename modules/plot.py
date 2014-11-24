# Force matplotlib to not use any Xwindows backend
import matplotlib
matplotlib.use('Agg')
import pylab
import numpy as np
import os, operator
import math, copy, itertools

class Plot:
	def __init__(self, plot_file, title, xlabel, ylabel, xlog, ylog, legend = [], xlimit = [], ylimit = [], formatter = False):
		self.plot_file = plot_file + ".png"

		self.title = title
		self.xlabel = xlabel
		self.ylabel = ylabel
		self.xlog = xlog
		self.ylog = ylog
		self.legend = legend
		self.xlimit = xlimit
		self.ylimit = ylimit
		self.formatter = formatter

		###############################
		#	PYLAB FEATURES
		###############################
		self.figure = pylab.figure(figsize=(12,10), dpi=80)
		self.fct_formatter = matplotlib.ticker.FuncFormatter(self.__to_percent)

		pylab.xlabel(self.xlabel)
		pylab.ylabel(self.ylabel)
		pylab.title(self.title)

		if self.formatter:
			pylab.gca().yaxis.set_major_formatter(self.fct_formatter)
		if self.xlog:
			pylab.xscale("log")
		if self.ylog:
			pylab.yscale("log")
		if self.xlimit:
			pylab.xlim(self.xlimit)
		if self.ylimit:
			pylab.ylim(self.ylimit)		

		font = {'family' : 'sans-serif',
				'style'  : 'normal',
        		'weight' : 'bold',
        		'size'   : 15}
		matplotlib.rc('font', **font)
		###############################

	def __to_percent(self, y, position):
	    s = str(100 * y)

	    if pylab.rcParams['text.usetex'] == True:
	        return s + r'$\%$'
	    else:
	        return s + '%'
	
	def plot_single(self, data, plot_parameters):
		metric_data = zip(*sorted(data.iteritems(), key=operator.itemgetter(0), reverse=False))
		val1 = metric_data[0]
		val2 = metric_data[1]

		pylab.plot(val1, val2, **plot_parameters)
		
		if self.legend:
			pylab.legend(self.legend, loc="best", prop={"size":31})

		self.figure.savefig(self.plot_file)
		pylab.clf()

	def plot_multiple(self, list_data, list_plot_parameters):
		i = 0
		for data in list_data:
			metric_data = zip(*sorted(data.iteritems(), key=operator.itemgetter(0), reverse=False))
			val1 = metric_data[0]
			val2 = metric_data[1]

			pylab.plot(val1, val2, **list_plot_parameters[i])
			i += 1
		
		if self.legend:
			pylab.legend(self.legend, loc="best", prop={"size":13})

		self.figure.savefig(self.plot_file)
		pylab.clf()
