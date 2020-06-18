set theme3 [create_charts_palette -theme theme3]

set_charts_palette -theme $theme3 -name desc -value "Theme 3"

set palette [create_charts_palette -palette my_red_green]

set_charts_palette -palette my_red_green -name defined_colors -value [list {0 red} {1 green}]

set colors [get_charts_palette -palette my_red_green -name defined_colors]

set_charts_palette -theme theme3 -name palettes -value [list my_red_green]

set model [load_charts_model -csv data/gaussian.txt -comment_header]

set plot [create_charts_plot -model $model -type distribution \
  -columns {{value 0}} -title "distribution"]

set view [get_charts_data -plot $plot -name view]

set_charts_property -view $view -name theme.name -value theme3

set_charts_property -plot $plot -name coloring.xStops         -value "0.0"
set_charts_property -plot $plot -name coloring.type           -value X_VALUE
set_charts_property -plot $plot -name coloring.defaultPalette -value my_red_green
