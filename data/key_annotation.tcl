set plot [create_charts_plot -type empty -title "Key Annotation" \
 -xmin -1.0 -xmax 1.0 -ymin -1.0 -ymax 1.0]

set annotation [create_charts_key_annotation -plot $plot]

add_charts_key_item -annotation $annotation -text Red   -color red
add_charts_key_item -annotation $annotation -text Green -color green
add_charts_key_item -annotation $annotation -text Blue  -color blue