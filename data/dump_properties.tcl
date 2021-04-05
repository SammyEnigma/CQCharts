proc encode_text { text } {
  return [encode_charts_text -csv -text $text]
}

proc encode_hidden { hidden } {
  if {$hidden} {
    return "hidden"
  } else {
    return ""
  }
}

proc dump_properties { hidden } {
  set red  {[31m}
  set norm {[0m}

  set model [load_charts_model -csv data/boxplot.csv -first_line_header]

  set types [get_charts_data -name plot_types]

  set view_checked 0

  set annotations_checked 0

  set title_checked 0
  set axis_checked  0
  set key_checked   0

  if {$hidden} {
    echo "type,sub_type,property,data_type,value,is_hidden,description"
  } else {
    echo "type,sub_type,property,data_type,value,description"
  }

  foreach type $types {
    set view [create_charts_view]

    if {! $view_checked} {
      if {$hidden} {
        set properties [get_charts_data -view $view -name properties -hidden]
      } else {
        set properties [get_charts_data -view $view -name properties]
      }

      foreach property $properties {
        set value [get_charts_property -view $view -name $property -hidden]
        set desc  [encode_text [get_charts_property -view $view -name $property -hidden -data desc]]
        set dtype [get_charts_property -view $view -name $property -hidden -data user_type]
        set owner [get_charts_property -view $view -name $property -hidden -data owner]
        set hid   [encode_hidden [get_charts_property -view $view -name $property -hidden -data is_hidden]]

        if {$hidden} {
          echo "view,,$property,$dtype,$value,$hid,$desc"
        } else {
          echo "view,,$property,$dtype,$value,$desc"
        }

        if {"$desc" == ""} {
          echo "${red}No desc for : ${property}${norm}"
          exit
        }

        if {"$dtype" == ""} {
          echo "${red}No type for : ${property} ${desc}${norm}"
          exit
        }
      }

      set view_checked 1
    }

    set title_checked1 0
    set axis_checked1  0
    set key_checked1   0

    set plot [create_charts_plot -view $view -type $type]

    if {$hidden} {
      set properties [get_charts_data -plot $plot -name properties -hidden]
    } else {
      set properties [get_charts_data -plot $plot -name properties]
    }

    foreach property $properties {
      set value [get_charts_property -plot $plot -name $property -hidden]
      set desc  [encode_text [get_charts_property -plot $plot -name $property -hidden -data desc]]
      set dtype [get_charts_property -plot $plot -name $property -hidden -data user_type]
      set owner [get_charts_property -plot $plot -name $property -hidden -data owner]
      set hid   [encode_hidden [get_charts_property -plot $plot -name $property -hidden -data is_hidden]]

      if {$owner == "title"} {
        if {$title_checked == 1} {
          continue
        }

        set title_checked1 1
      }

      if {$owner == "axis"} {
        if {$axis_checked == 1} {
          continue
        }

        set axis_checked1 1
      }

      if {$owner == "key"} {
        if {$key_checked == 1} {
          continue
        }

        set key_checked1 1
      }

      if {$hidden} {
        echo "plot,$type,$property,$dtype,$value,$hid,$desc"
      } else {
        echo "plot,$type,$property,$dtype,$value,$desc"
      }

      if {"$desc" == ""} {
        echo "${red}No desc for : ${property}${norm}"
        exit
      }

      if {"$dtype" == ""} {
        echo "${red}No type for : ${property} ${desc}${norm}"
        exit
      }
    }

    if {$title_checked1 == 1} {
      set title_checked 1
    }

    if {$axis_checked1 == 1} {
      set axis_checked 1
    }

    if {$key_checked1 == 1} {
      set key_checked 1
    }

    if {! $annotations_checked} {
      set types [get_charts_data -name annotation_types]

      foreach type $types {
        set cmd "create_charts_${type}_annotation"

echo "$cmd"
        if       {$type == "polygon"} {
          set annotation [eval {$cmd -plot $plot -points {{0 0} {1 0} {1 1} {0 1}}}]
        } elseif {$type == "polyline"} {
          set annotation [eval {$cmd -plot $plot -points {{0 0} {1 1} {2 0}}}]
        } elseif {$type == "ellipse"} {
          set annotation [eval {$cmd -plot $plot -center {0 0} -rx 1 -ry 1}]
        } elseif {$type == "image"} {
          set annotation [eval {$cmd -plot $plot -svg data/path.svg}]
        } elseif {$type == "path"} {
          set annotation [eval {$cmd -plot $plot -path "M 0 0 L 10 10 L 20 10"}]
        } elseif {$type == "arrow"} {
          set annotation [eval {$cmd -plot $plot -start {0 0} -end {1 1}}]
        } elseif {$type == "arc"} {
          set annotation [eval {$cmd -plot $plot -start {0 0} -end {1 1}}]
        } elseif {$type == "pie_slice"} {
          set annotation [eval {$cmd -plot $plot -position {0 0} -outer_radius 1}]
        } elseif {$type == "axis"} {
          set annotation [eval {$cmd -plot $plot -start 0 -end 1}]
        } elseif {$type == "point"} {
          set annotation [eval {$cmd -plot $plot -position {0 0}}]
        } elseif {$type == "point_set"} {
          set annotation [eval {$cmd -plot $plot -values {{0 0} {1 1} {2 2}}}]
        } elseif {$type == "value_set"} {
          set annotation [eval {$cmd -plot $plot -values {0 1 2} -rectangle {0 0 1 1}}]
        } elseif {$type == "group"} {
          # TODO
        } else {
          set annotation [eval {$cmd -plot $plot}]
        }

        if {$hidden} {
          set properties [get_charts_data -annotation $annotation -name properties -hidden]
        } else {
          set properties [get_charts_data -annotation $annotation -name properties]
        }

        foreach property $properties {
          set value [get_charts_property -annotation $annotation -name $property -hidden]
          set desc  [encode_text [get_charts_property -annotation $annotation -name $property -hidden -data desc]]
          set dtype [get_charts_property -annotation $annotation -name $property -hidden -data user_type]
          set owner [get_charts_property -annotation $annotation -name $property -hidden -data owner]
          set hid   [encode_hidden [get_charts_property -annotation $annotation -name $property -hidden -data is_hidden]]

          if {$hidden} {
            echo "annotation,$type,$property,$dtype,$value,$hid,$desc"
          } else {
            echo "annotation,$type,$property,$dtype,$value,$desc"
          }

          if {"$desc" == ""} {
            echo "${red}No desc for : ${property}${norm}"
            exit
          }

          if {"$dtype" == ""} {
            echo "${red}No type for : ${property} ${desc}${norm}"
            exit
          }
        }
      }

      set annotations_checked 1
    }

    remove_charts_view -view $view
  }
}

dump_properties 1

exit
