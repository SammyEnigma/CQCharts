# Adjacency From/To
set model [load_charts_model -csv data/dot/dot_fsm.csv -first_line_header]

set plot [create_charts_plot -model $model -type adjacency \
  -columns {{from 0} {to 1} {attributes 2}} -title "adjacency from/to"]
