set plotId [create_charts_plot -type empty -xmin 0 -ymin 0 -xmax 100 -ymax 100]

set rect1 [create_charts_shape_annotation -plot $plotId -rectangle {{10 10} {20 20}}]

set_charts_property -annotation $rect1 -name shapeType -value CIRCLE

set image [create_charts_image_annotation -plot $plotId \
  -position {32 0 px} -svg "data/beer_bottle.svg"]

set_charts_property -annotation $image -name objRef -value [list $rect1 right]
