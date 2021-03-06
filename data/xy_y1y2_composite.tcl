set model1 [load_charts_model -tsv data/multi_series.tsv -comment_header \
  -column_type {{time {format %Y%m%d}}}]

set plot1 [create_charts_plot -model $model1 -type xy -columns {{x 0} {y 1}} \
  -title "multiple y axis"]
set plot2 [create_charts_plot -model $model1 -type xy -columns {{x 0} {y 2}}]

group_charts_plots -composite -y1y2 [list $plot1 $plot2]
