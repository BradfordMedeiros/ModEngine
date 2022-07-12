<?php
  $keyname = ")" . $unique_control_id . "_" . "numeric_mainlabel";
  createElement($keyname, $default_key, [  "value" => $data["key"] ]);
  $value = $data["value"];
  $numericElements = [ $keyname ];

  for ($i = 0; $i < count($value); $i++){
    $floatLabelElementName =  ")" . $unique_control_id . "_" . "numeric_" . $i . "_float";
    createElement($floatLabelElementName, $default_key, [ "value" => $value[$i]["name"] ]);
    $controlType = $value[$i]["type"];

    $managedElements = [ $floatLabelElementName ];
    $hasBinding = !is_string($value[$i]);
    $textValue = "";
    if (! $hasBinding){
      $textValue = $value[$i]["value"];
    }
    $attrValues = [ 
      "value" => $textValue, 
      "details-editabletext" => "true",
    ];
    if ($hasBinding){
      $attrValues["details-binding"] = $value[$i]["value"]["binding"];
      if (array_key_exists("binding-index", $value[$i]["value"])){
        $attrValues["details-binding-index"] = $value[$i]["value"]["binding-index"];
      }
    }

    if ($controlType == "float"){
      $floatValueElementName =  ")" . $unique_control_id . "_" . "numeric"  . $i . "_value";
      if (array_key_exists("type", $value[$i]["value"])){
        $type = $value[$i]["value"]["type"];
        if ($type != "number" && $type != "positive-number"){
          print("invalid numeric type");
          exit(1);
        }
        $attrValues["details-editable-type"] = $type;
      }else{
        $attrValues["details-editable-type"] = "number";
      }

      createElement($floatValueElementName, $default_value, $attrValues);
      array_push($managedElements, $floatValueElementName);
    }else if ($controlType == "slider"){
      $sliderElementName =  "_" . $unique_control_id . "_" . "numeric"  . $i . "_value";
      $attrValues["scale"] = "0.3 0.02 0.02";
      $attrValues["onslide"] = "details-editable-slide";
      $attrValues["backpaneltint"] = "0.3 0.3 0.3 1";
      createElement($sliderElementName, $default_value, $attrValues);
      array_push($managedElements, $sliderElementName);
    }else{
      print("Invalid control type");
      exit(1);
    }
    $floatElementName = "(" . $unique_control_id . "_" . "numeric_" . $i;
    createElement($floatElementName, $default_key, [ "min-spacing" => "0.08", "margin" => "0.02", "minwidth" => "0.44", "tint" => "0.2 0.2 0.2 1", "backpanel" => "true", "elements" => implode(",", $managedElements) ]);
    array_push($numericElements, $floatElementName);
  }

  $numericElements = array_reverse($numericElements);
  createElement($rootElementName, $default_rootLayout, [ "tint" => "0.3 0.3 0.3 1", "margin-top" => "0.02", "margin-left" => "0", "margin-right" => "0", "spacing" => "0.01", "elements" => implode(",", $numericElements), "type" => "vertical" ]);
?>

