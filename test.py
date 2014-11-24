#! python

''' // CREATE PROJECTIONS

from modules.calcModule import *

simple_load_graph("Datasets/anime.etiquette", 1)
save_projection(".")
'''

import modules.drawing

'''D = modules.drawing.DrawGraph("projection_top")
D.draw()
D.to_gexf()'''

D = modules.drawing.DrawBipartite("Datasets/network_protocols.etiquette", 1)
#D.draw()
D.to_gexf()
