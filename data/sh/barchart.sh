# Bar Chart

# Tests:
#   Simple (Single set of bars)
#   Simple (Multiple bars per set)
#   Simple (Multiple bars per set grouped by row)
#   Stacked
#   Percent
#   Range

CQChartsTest \
 -csv data/multi_bar.csv -first_line_header \
 -type bar -columns "group=0,value=1" -column_type "1#integer" \
 -plot_title "bar chart" \
-and \
 -csv data/multi_bar.csv -first_line_header \
 -type bar -columns "name=0,value=1 2 3 4 5 6 7" \
 -plot_title "multiple bar chart" -column_type "1#integer" \
 -bool "horizontal=1" \
-and \
 -csv data/multi_bar.csv -first_line_header \
 -type bar -columns "group=0,value=1 2 3 4 5 6 7" \
 -plot_title "row grouping" -column_type "1#integer" \
 -properties "dataGrouping.rowGrouping=1" \
-and \
 -csv data/money_bar_stack_neg.csv -first_line_header \
 -type barchart -columns "group=0,value=1 2 3 4" \
 -bool "stacked=1,percent=1" \
 -plot_title "stack negative" \
-and \
 -csv data/temp_bar_range.csv -first_line_header \
 -type barchart -columns "group=0,value=2 3" \
 -bool "rangeBar=1,horizontal=1" \
 -plot_title "bar range" \
 -properties "dataLabel.visible=1,dataLabel.position=TOP_OUTSIDE" \
 -properties "key.visible=0,options.barMargin=12px,fill.color=palette:0.2"
