# table column model
set model [load_charts_model -data data/chord-cities.data]

set plot [create_charts_plot -type adjacency -columns {{link 0} {group 1}}]
