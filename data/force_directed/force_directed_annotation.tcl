proc objPressed { view plot id } {
  echo "$view $plot $id"

  set model [get_charts_data -plot $plot -name model]

  set objs [get_charts_data -plot $plot -object $id -name connected]

  echo "Connected"

  foreach obj $objs {
    echo "$obj"
  }
}

proc plotAnnotationSlot { viewId plotId id } {
  puts "plotAnnotationSlot: $viewId, $plotId, $id"

  set text [get_charts_property -annotation $id -name text.string]
  puts "text: $text"
}

set model [load_charts_model -csv data/sankey.csv -comment_header \
 -column_type {{{0 name_pair}}}]

set plot [create_charts_plot -model $model -type force_directed \
  -columns {{link 0} {value 1}} -title "Force Directed Budget"]

connect_charts_signal -plot $plot -from objIdPressed -to objPressed

set button [create_charts_button_annotation -plot $plot -id button \
              -position {5 5 V} -text "Button"]

connect_charts_signal -plot $plot -from annotationIdPressed -to plotAnnotationSlot