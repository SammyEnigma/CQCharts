proc plotSlot(viewId, plotId, rowId)
  print "plotSlot", viewId, plotId, rowIZZd

  get_data -view $viewId -plot $plotId -id $rowId -column 1

  print _rc
endproc

load -csv data/coverage.csv -first_line_header
modelId = _rc

set_model -ind $modelId -column_type "0#time:format=%m/%d/%Y,oformat=%F"

add_plot -type xy -columns "id=0,x=0,y=1"
plotId1 = _rc

get_view -current
viewId = _rc

add_plot -type xy -columns "id=0,x=0,y=2"
plotId2 = _rc

set_property -view $viewId -plot $plotId1 -name impulse.visible -value 1
set_property -view $viewId -plot $plotId1 -name impulse.color   -value palette
set_property -view $viewId -plot $plotId1 -name impulse.alpha   -value 0.5
set_property -view $viewId -plot $plotId1 -name impulse.width   -value 20px

set_property -view $viewId -plot $plotId2 -name invertY         -value 1
set_property -view $viewId -plot $plotId2 -name impulse.visible -value 1
set_property -view $viewId -plot $plotId2 -name impulse.color   -value palette#1
set_property -view $viewId -plot $plotId2 -name impulse.alpha   -value 0.5
set_property -view $viewId -plot $plotId2 -name impulse.width   -value 20px

group -y1y2 $plotId1 $plotId2

place -vertical $plotId1 $plotId2

connect -view $viewId -plot $plotId1 -from objIdPressed -to plotSlot