import metric
import progressbar, os
import numpy as np

class Bipartite():
	################################################################
	#                   DEFINITIONS OF METRICS                     #
	################################################################

	def __init__(self, directory):
		self.directory = directory

		self.list_top_nodes = []
		self.list_bot_nodes = []		
		self.n_top = 0
		self.n_bot = 0
		self.n = 0

		# =========== METRICS ===========
		## \\\\\\\\\\\\ TOP PART //////////// ##
		self.degree_top = {}
		self.cc_top = {}
		self.cc_min_top = {}
		self.cc_max_top = {}
		self.monopole_top = {}
		self.avg_size_community_top = {}
		self.dispersion_top = {}
		self.rc_top = {}

		self.list_metrics_top = {}
		self.list_correlations_top = {}

		## \\\\\\\\\\\\ BOT PART //////////// ##
		# =========== METRICS ===========
		self.degree_bot = {}
		self.cc_bot = {}
		self.cc_min_bot = {}
		self.cc_max_bot = {}
		self.monopole_bot = {}
		self.avg_size_community_bot = {}
		self.dispersion_bot = {}
		self.rc_bot = {}

		self.list_metrics_bot = {}
		self.list_correlations_bot = {}

		# EXTRA
		self.list_correlations_mixed = {}

	def open(self):
		# ////////////////////////////////////////////////////// #
		top_nodes_file = self.directory + "/top.nodes"
		bot_nodes_file = self.directory + "/bot.nodes"

		with open(top_nodes_file, 'r') as file:
			for line in file:
				self.list_top_nodes.append(line.replace("\n", ""))
		self.n_top = len(self.list_top_nodes)

		with open(bot_nodes_file, 'r') as file:
			for line in file:
				self.list_bot_nodes.append(line.replace("\n", ""))
		self.n_bot = len(self.list_bot_nodes)
		self.n = self.n_top + self.n_top

		# ////////////////////////////////////////////////////// #
		metrics = [self.degree_top, self.degree_bot, self.cc_top, self.cc_bot, 
				self.cc_min_top, self.cc_min_bot, self.cc_max_top, self.cc_max_bot,
				self.monopole_top, self.monopole_bot, self.avg_size_community_top, self.avg_size_community_bot,
				self.dispersion_top, self.dispersion_bot, self.rc_top, self.rc_bot]

		metrics_files = ["degree_top.data", "degree_bot.data", "cc_top.data", "cc_bot.data", 
					"cc_min_top.data", "cc_min_bot.data", "cc_max_top.data", "cc_max_bot.data",
					"monopole_top.data", "monopole_bot.data", "avg_size_community_top.data", "avg_size_community_bot.data",
					"dispersion_top.data", "dispersion_bot.data", "rc_top.data", "rc_bot.data"]

		i = 0
		while i < len(metrics):
			top_file = self.directory + "/data/top/" + metrics_files[i]
			with open(top_file, 'r') as file:
				for line in file:
					line = line.replace("\n", "").split()
					metrics[i][line[0]] = float(line[1])
			i += 1

			bot_file = self.directory + "/data/bot/" + metrics_files[i]
			with open(bot_file, 'r') as file:
				for line in file:
					line = line.replace("\n", "").split()
					metrics[i][line[0]] = float(line[1])
			i += 1

		self.degree_top = dict(map(lambda (k,v): (k, int(v)), self.degree_top.iteritems()))
		self.degree_bot = dict(map(lambda (k,v): (k, int(v)), self.degree_bot.iteritems()))

	def save_metrics(self):		
		directory_metric = self.directory + "/metrics"
		if not os.path.exists(directory_metric):
			os.mkdir(directory_metric)

		directory_metric_top = directory_metric + "/top"
		if not os.path.exists(directory_metric_top):
			os.mkdir(directory_metric_top)

		for metric in self.list_metrics_top.itervalues():
			d = directory_metric_top + "/" + metric.name
			if not os.path.exists(d):
				os.mkdir(d)

			metric.save(d)

			f = open(d + "/info", "w")
			f.write(str(metric))
			f.close()

		directory_metric_bot = directory_metric + "/bot"
		if not os.path.exists(directory_metric_bot):
			os.mkdir(directory_metric_bot)

		for metric in self.list_metrics_bot.itervalues():
			d = directory_metric_bot + "/" + metric.name
			if not os.path.exists(d):
				os.mkdir(d)

			metric.save(d)

			f = open(d + "/info", "w")
			f.write(str(metric))
			f.close()
	
	def create_directories(self):
		self.directory_correlation = self.directory + "/correlations"
		if not os.path.exists(self.directory_correlation):
			os.mkdir(self.directory_correlation)

		self.directory_correlation_top = self.directory_correlation + "/top"
		if not os.path.exists(self.directory_correlation_top):
			os.mkdir(self.directory_correlation_top)

		self.directory_correlation_bot = self.directory_correlation + "/bot"
		if not os.path.exists(self.directory_correlation_bot):
			os.mkdir(self.directory_correlation_bot)

	def save_correlation(self, correlation, part):
		if part == "top":			
			d = self.directory_correlation_top + "/" + correlation.name
		else:
			d = self.directory_correlation_bot + "/" + correlation.name
		
		if not os.path.exists(d):
			os.mkdir(d)

		correlation.save(d)
		self.save_informations(correlation, d)

	def save_informations(self, correlation, d):
		f = open(d + "/info", "w")
		f.write(str(correlation))
		f.close()

	def save_correlations(self):
		for correlation in self.list_correlations_top.itervalues():
			save_correlation(correlation, "top")

		for correlation in self.list_correlations_bot.itervalues():
			save_correlation(correlation, "bot")


	################################################################
	#                   		METRICS                            #
	################################################################

	def create_metrics(self):
		## \\\\\\\\\\\\ TOP PART //////////// ##
		self.list_metrics_top["degree_top"] = metric.Metric(self, "degree_top", self.degree_top)
		self.list_metrics_top["degree_top"].compile()
		self.list_metrics_top["cc_top"] = metric.Metric(self, "cc_top", self.cc_top)
		self.list_metrics_top["cc_top"].compile()
		self.list_metrics_top["cc_min_top"] = metric.Metric(self, "cc_min_top", self.cc_min_top)
		self.list_metrics_top["cc_min_top"].compile()
		self.list_metrics_top["cc_max_top"] = metric.Metric(self, "cc_max_top", self.cc_max_top)
		self.list_metrics_top["cc_max_top"].compile()
		self.list_metrics_top["monopole_top"] = metric.Metric(self, "monopole_top", self.monopole_top)
		self.list_metrics_top["monopole_top"].compile()
		self.list_metrics_top["avg_size_community_top"] = metric.Metric(self, "avg_size_community_top", self.avg_size_community_top)
		self.list_metrics_top["avg_size_community_top"].compile()
		self.list_metrics_top["dispersion_top"] = metric.Metric(self, "dispersion_top", self.dispersion_top)
		self.list_metrics_top["dispersion_top"].compile()
		self.list_metrics_top["rc_top"] = metric.Metric(self, "rc_top", self.rc_top)
		self.list_metrics_top["rc_top"].compile()

		## \\\\\\\\\\\\ BOT PART //////////// ##
		self.list_metrics_bot["degree_bot"] = metric.Metric(self, "degree_bot", self.degree_bot)
		self.list_metrics_bot["degree_bot"].compile()
		self.list_metrics_bot["cc_bot"] = metric.Metric(self, "cc_bot", self.cc_bot)
		self.list_metrics_bot["cc_bot"].compile()
		self.list_metrics_bot["cc_min_bot"] = metric.Metric(self, "cc_min_bot", self.cc_min_bot)
		self.list_metrics_bot["cc_min_bot"].compile()
		self.list_metrics_bot["cc_max_bot"] = metric.Metric(self, "cc_max_bot", self.cc_max_bot)
		self.list_metrics_bot["cc_max_bot"].compile()
		self.list_metrics_bot["monopole_bot"] = metric.Metric(self, "monopole_bot", self.monopole_bot)
		self.list_metrics_bot["monopole_bot"].compile()
		self.list_metrics_bot["avg_size_community_bot"] = metric.Metric(self, "avg_size_community_bot", self.avg_size_community_bot)
		self.list_metrics_bot["avg_size_community_bot"].compile()
		self.list_metrics_bot["dispersion_bot"] = metric.Metric(self, "dispersion_bot", self.dispersion_bot)
		self.list_metrics_bot["dispersion_bot"].compile()
		self.list_metrics_bot["rc_bot"] = metric.Metric(self, "rc_bot", self.rc_bot)
		self.list_metrics_bot["rc_bot"].compile()

	'''def __compute_rec_community(self):
		for id_top_node in self.list_top_nodes:
			# LOCAL REC
			local_rec_community = {}
			# GLOBAL REC (average)
			global_rec = 0.0
			
			neighbours = self.list_top_nodes[id_top_node]
			
			# INIT
			self.list_rec_community[id_top_node] = 0.0
			self.list_local_rec_community[id_top_node] = {}

			if self.list_nb_neighbours_community[id_top_node] > 0:
				for id_neighbour_top in self.list_neighbours_community[id_top_node]:
					local_rec_community[id_top_node] = -1

					l = filter(neighbours.__contains__, self.list_top_nodes[id_neighbour_top])
					local_rec_community[id_neighbour_top] = len(l) / float(self.list_top_nodes_degrees[id_top_node])
					global_rec += local_rec_community[id_neighbour_top]

				self.list_local_rec_community[id_top_node] = local_rec_community
				self.list_rec_community[id_top_node] = global_rec / float(self.list_nb_neighbours_community[id_top_node])'''

	################################################################
	#                   CORRELATION OF METRICS                     #
	################################################################

	def treat_some_correlations(self):
		#### DEGREE TOP ####
		self.list_correlations_top["degree_top-cc_top"] = metric.Correlation(self, self.list_metrics_top["degree_top"], self.list_metrics_top["cc_top"], "top")
		self.list_correlations_top["degree_top-cc_top"].compile()
		self.save_correlation(self.list_correlations_top["degree_top-cc_top"], "top")
		del self.list_correlations_top["degree_top-cc_top"]
		print "- Finished correlation 'degree_top-cc_top'"
		
		self.list_correlations_top["degree_top-rc_top"] = metric.Correlation(self, self.list_metrics_top["degree_top"], self.list_metrics_top["rc_top"], "top")
		self.list_correlations_top["degree_top-rc_top"].compile()
		self.save_correlation(self.list_correlations_top["degree_top-rc_top"], "top")
		del self.list_correlations_top["degree_top-rc_top"]
		print "- Finished correlation 'degree_top-rc_top'"

		self.list_correlations_top["degree_top-monopole_top"] = metric.Correlation(self, self.list_metrics_top["degree_top"], self.list_metrics_top["monopole_top"], "top")
		self.list_correlations_top["degree_top-monopole_top"].compile()
		self.save_correlation(self.list_correlations_top["degree_top-monopole_top"], "top")
		del self.list_correlations_top["degree_top-monopole_top"]
		print "- Finished correlation 'degree_top-monopole_top'"

		self.list_correlations_top["degree_top-avg_size_community_top"] = metric.Correlation(self, self.list_metrics_top["degree_top"], self.list_metrics_top["avg_size_community_top"], "top")
		self.list_correlations_top["degree_top-avg_size_community_top"].compile()
		self.save_correlation(self.list_correlations_top["degree_top-avg_size_community_top"], "top")
		del self.list_correlations_top["degree_top-avg_size_community_top"]
		print "- Finished correlation 'degree_top-avg_size_community_top'"

		self.list_correlations_top["degree_top-dispersion_top"] = metric.Correlation(self, self.list_metrics_top["degree_top"], self.list_metrics_top["dispersion_top"], "top")
		self.list_correlations_top["degree_top-dispersion_top"].compile()
		self.save_correlation(self.list_correlations_top["degree_top-dispersion_top"], "top")
		del self.list_correlations_top["degree_top-dispersion_top"]
		print "- Finished correlation 'degree_top-dispersion_top'"

		#### DEGREE BOT ####
		self.list_correlations_bot["degree_bot-cc_bot"] = metric.Correlation(self, self.list_metrics_bot["degree_bot"], self.list_metrics_bot["cc_bot"], "bot")
		self.list_correlations_bot["degree_bot-cc_bot"].compile()
		self.save_correlation(self.list_correlations_bot["degree_bot-cc_bot"], "bot")
		del self.list_correlations_bot["degree_bot-cc_bot"]
		print "- Finished correlation 'degree_bot-cc_bot'"

		self.list_correlations_bot["degree_bot-rc_bot"] = metric.Correlation(self, self.list_metrics_bot["degree_bot"], self.list_metrics_bot["rc_bot"], "bot")
		self.list_correlations_bot["degree_bot-rc_bot"].compile()
		self.save_correlation(self.list_correlations_bot["degree_bot-rc_bot"], "bot")
		del self.list_correlations_bot["degree_bot-rc_bot"]
		print "- Finished correlation 'degree_bot-rc_bot'"

		self.list_correlations_bot["degree_bot-monopole_bot"] = metric.Correlation(self, self.list_metrics_bot["degree_bot"], self.list_metrics_bot["monopole_bot"], "bot")
		self.list_correlations_bot["degree_bot-monopole_bot"].compile()
		self.save_correlation(self.list_correlations_bot["degree_bot-monopole_bot"], "bot")
		del self.list_correlations_bot["degree_bot-monopole_bot"]
		print "- Finished correlation 'degree_bot-monopole_bot'"

		self.list_correlations_bot["degree_bot-avg_size_community_bot"] = metric.Correlation(self, self.list_metrics_bot["degree_bot"], self.list_metrics_bot["avg_size_community_bot"], "bot")
		self.list_correlations_bot["degree_bot-avg_size_community_bot"].compile()
		self.save_correlation(self.list_correlations_bot["degree_bot-avg_size_community_bot"], "bot")
		del self.list_correlations_bot["degree_bot-avg_size_community_bot"]
		print "- Finished correlation 'degree_bot-avg_size_community_bot'"

		self.list_correlations_bot["degree_bot-dispersion_bot"] = metric.Correlation(self, self.list_metrics_bot["degree_bot"], self.list_metrics_bot["dispersion_bot"], "bot")
		self.list_correlations_bot["degree_bot-dispersion_bot"].compile()
		self.save_correlation(self.list_correlations_bot["degree_bot-dispersion_bot"], "bot")
		del self.list_correlations_bot["degree_bot-dispersion_bot"]
		print "- Finished correlation 'degree_bot-dispersion_bot'"

		#### CC TOP ####
		self.list_correlations_top["cc_top-rc_top"] = metric.Correlation(self, self.list_metrics_top["cc_top"], self.list_metrics_top["rc_top"], "top")
		self.list_correlations_top["cc_top-rc_top"].compile()
		self.save_correlation(self.list_correlations_top["cc_top-rc_top"], "top")
		del self.list_correlations_top["cc_top-rc_top"]
		print "- Finished correlation 'cc_top-rc_top'"

		#### CC BOT ####
		self.list_correlations_bot["cc_bot-rc_bot"] = metric.Correlation(self, self.list_metrics_bot["cc_bot"], self.list_metrics_bot["rc_bot"], "bot")
		self.list_correlations_bot["cc_bot-rc_bot"].compile()
		self.save_correlation(self.list_correlations_bot["cc_bot-rc_bot"], "bot")
		del self.list_correlations_bot["cc_bot-rc_bot"]
		print "- Finished correlation 'cc_bot-rc_bot'"

	def treat_all_correlations(self):
		i = 0
		size_bar = (len(self.list_metrics_top) * len(self.list_metrics_top)) - len(self.list_metrics_top)
		size_bar += (len(self.list_metrics_bot) * len(self.list_metrics_bot)) - len(self.list_metrics_bot)
		#size_bar += (len(self.list_metrics_top) * len(self.list_metrics_bot))

		widgets = [progressbar.Percentage(),
               ' ', progressbar.Bar(),
               ' ', progressbar.ETA()]
		pbar = progressbar.ProgressBar(widgets=widgets, maxval=size_bar)
		pbar.start()

		#COMPUTE TOP POSSIBLE CORRELATIONS
		for metric_top1 in self.list_metrics_top:
			for metric_top2 in self.list_metrics_top:
				if metric_top1 != metric_top2:					
					name_correlation = "%s-%s" % (metric_top1, metric_top2)
					self.list_correlations_top[name_correlation] = metric.Correlation(self, self.list_metrics_top[metric_top1], self.list_metrics_top[metric_top2], "top")
					self.list_correlations_top[name_correlation].compile()  
					print "- Finished correlation '%s'" % name_correlation

					# MIRRORING
					name_correlation = "%s-%s" % (metric_top2, metric_top1)
					self.list_correlations_top[name_correlation] = metric.Correlation(self, self.list_metrics_top[metric_top2], self.list_metrics_top[metric_top1], "top")
					self.list_correlations_top[name_correlation].compile()
					print "- Finished correlation '%s'" % name_correlation

					i += 1
					pbar.update(i)

				
		#COMPUTE BOT POSSIBLE CORRELATIONS						
		for metric_bot1 in self.list_metrics_bot:
			for metric_bot2 in self.list_metrics_bot:
				if metric_bot1 != metric_bot2:					
					name_correlation = "%s-%s" % (metric_bot1, metric_bot2)
					self.list_correlations_bot[name_correlation] = metric.Correlation(self, self.list_metrics_bot[metric_bot1], self.list_metrics_bot[metric_bot2], "bot")
					self.list_correlations_bot[name_correlation].compile()
					print "- Finished correlation '%s'" % name_correlation  

					# MIRRORING
					name_correlation = "%s-%s" % (metric_top2, metric_top1)
					self.list_correlations_bot[name_correlation] = metric.Correlation(self, self.list_metrics_bot[metric_bot2], self.list_metrics_bot[metric_bot1], "bot")
					self.list_correlations_bot[name_correlation].compile()
					print "- Finished correlation '%s'" % name_correlation

					i += 1
					pbar.update(i)

		#COMPUTE MIXES TOP-BOT/BOT-TOP POSSIBLE CORRELATIONS						
		'''for metric_top in self.list_metrics_top:
			for metric_bot in self.list_metrics_bot:
				name_correlation = "%s-%s" % (metric_top, metric_bot)
				self.list_correlations_mixed[name_correlation] = metric.Correlation(self, self.list_metrics_top[metric_top], self.list_metrics_bot[metric_bot], "top-bot", mixed=True)
				self.list_correlations_mixed[name_correlation].compile()  
				print "- Finished correlation '%s'" % name_correlation

				# MIRRORING
				name_correlation = "%s-%s" % (metric_top2, metric_top1)
				self.list_correlations_mixed[name_correlation] = metric.Correlation(self, self.list_metrics_bot[metric_bot], self.list_metrics_top[metric_top], "bot-top", mixed=True)
				self.list_correlations_mixed[name_correlation].compile()
				print "- Finished correlation '%s'" % name_correlation

				i += 1
				pbar.update(i)'''

		pbar.finish()

