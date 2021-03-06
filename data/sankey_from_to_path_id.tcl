# sankey link model
#set model [load_charts_model -csv data/sankey_from_to_path_id_1.csv -first_line_header]
set model [load_charts_model -csv data/sankey_big_path_id.csv -first_line_header]

#set plot [create_charts_plot -model $model -type sankey \
# -columns {{from From} {to To} {value Value} {depth Depth} {attributes Attributes}}]
set plot [create_charts_plot -model $model -type sankey \
 -columns {{from From} {to To} {value Value} {depth Depth} {pathId Path}}]

set_charts_property -plot $plot -name plotBox.clip -value 0
set_charts_property -plot $plot -name dataBox.clip -value 0

set axis [create_charts_axis_annotation -plot $plot \
  -start -1 -end 1 -position -1 -direction horizontal]

set_charts_property -annotation $axis -name valueStart     -value 1
set_charts_property -annotation $axis -name valueEnd       -value 4
set_charts_property -annotation $axis -name majorIncrement -value 1
