<?php
  $keyname = ")" . $unique_control_id . "_" . "numeric_mainlabel";
  createElement($keyname, $default_key, [  "value" => $data["key"] ]);
  $value = $data["value"];
  $numericElements = [ $keyname ];

  for ($x = 0; $x < count($value); $x++){
    $floatLabelElementName =  ")" . $unique_control_id . "_" . "numeric_" . $x . "_float";
    createElement($floatLabelElementName, $default_key, [ "value" => $value[$x]["name"] ]);
    $managedElements = [ $floatLabelElementName ];

    $controlType = $value[$x]["type"];
    if ($controlType == "float"){
      $editableType = NULL;
      if (array_key_exists("type", $value[$x]["value"])){
        $editableType = $value[$x]["value"]["type"];
      }
      $holdername = "(" . $unique_control_id . "_numeric_textfield_holder_" . $x;
      $valuename = ")numeric_text_" . $unique_control_id . "_" . $x;

      $readonly = false;
      if (array_key_exists("readonly", $value[$x]["value"])){
        $readonly = $value[$x]["value"]["readonly"];
      }
      createTextbox($holdername, $valuename, $readonly, $editableType, $value[$x]["value"], false, $styles);
      array_push($managedElements, $holdername);
    }else if ($controlType == "slider"){
      $hasBinding = !is_string($value[$x]);
      $textValue = "";
      if (! $hasBinding){
        $textValue = $value[$x]["value"];
      }
      $sliderElementName =  "_" . $unique_control_id . "_" . "numeric"  . $x . "_value";
      $attrValues = [ 
        "value" => $textValue, 
        "details-editabletext" => "true",
      ];
      if ($hasBinding){
        $attrValues["details-binding"] = $value[$x]["value"]["binding"];
        if (array_key_exists("binding-index", $value[$x]["value"])){
          $attrValues["details-binding-index"] = $value[$x]["value"]["binding-index"];
        }
      }
      $attrValues["scale"] = "0.3 0.02 0.02";
      $attrValues["onslide"] = "details-editable-slide";
      $attrValues["backpaneltint"] = "0.3 0.3 0.3 1";

      if (array_key_exists("min", $value[$x]["value"])){
        $attrValues["min"] = $value[$x]["value"]["min"];
      }
      if (array_key_exists("max", $value[$x]["value"])){
        $attrValues["max"] = $value[$x]["value"]["max"];
      }

      createElement($sliderElementName, $default_value, $attrValues);
      array_push($managedElements, $sliderElementName);
    }else{
      print("Invalid control type");
      exit(1);
    }
    $floatElementName = "(" . $unique_control_id . "_" . "numeric_" . $x;
    createElement($floatElementName, $default_key, [ "min-spacing" => "0.08", "margin" => "0.02", "minwidth" => "0.44", "tint" => "0.2 0.2 0.2 1", "backpanel" => "true", "elements" => implode(",", $managedElements), "position" => "0 0 " . $depth[29] ]);
    array_push($numericElements, $floatElementName);
  }

  $numericElements = array_reverse($numericElements);
  createElement($rootElementName, $default_rootLayout, [ "tint" => "0.3 0.3 0.3 1", "margin-top" => "0.02", "margin-left" => "0", "margin-right" => "0", "spacing" => "0.01", "elements" => implode(",", $numericElements), "type" => "vertical" ]);
?>

