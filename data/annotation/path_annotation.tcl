proc annotationSlot { viewId plotId id } {
  puts "annotationSlot: $viewId, $plotId, $id"

  set id [get_charts_property -annotation $id -name id]
  puts "id: $id"
}

set plot [create_charts_plot -type empty -xmin 0 -ymin 0 -xmax 100 -ymax 100]

set path1 [create_charts_path_annotation -plot $plot -id one \
  -path "M50,2a48,48 0 1 1 0,96a24 24 0 1 1 0-48a24 24 0 1 0 0-48"]

connect_charts_signal -plot $plot -from annotationPressed -to annotationSlot
