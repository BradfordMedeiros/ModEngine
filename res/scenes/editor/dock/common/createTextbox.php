<?php

  // binding 
  // 1. [ "value" => "literal value to display"] or 
  // or 
  // 2. [ "value" => [ "binding"= > "binding to create" ]]

  function createTextbox($holdername, $valuename, $readonly, $type, $binding, $styles){
    // details-editable-type to sponsor the type behavior
    if ($type != NULL){
      if ($type!= "number" && $type != "positive-number"){  // should actually sponsor
        print("invalid numeric type");
        exit(1);
      }
    }


    $default_key = $styles["default_key"];
    $default_value = $styles["default_value"];

    $style = [];
    if (is_string($binding["value"])){
      $style["value"] = $binding["value"];
    }else{
      $style["value"] = "";
      $style["details-binding"] = $binding["value"]["binding"];
    }
    if (!$readonly){
      $style["details-editabletext"] = "true";
      $style["wrapamount"] = "10"; # size limit
      $style["wraptype"] = "char";
      $style["maxheight"] = "1"; 
    }
    createElement($valuename, $default_value, $style);

    $editableStyle = [
      "backpanel" => "true", 
      "tint" => "0 0 0 1", 
      "minwidth" => "0.2",
      "align-items-horizontal" => "center",
      "details-reselect" => $valuename,
 
      #"border-size" => "0.004", // should be the border size when selected
    ];
 
    $defaultStyle = [ 
      "min-spacing" => "0.08", 
      "margin" => "0.02", 
      "elements" => $valuename,
    ];
 
    $combinedStyle = $defaultStyle;
    if (!$readonly){
      $combinedStyle = array_merge($editableStyle, $defaultStyle);
    }
 
    createElement($holdername, $default_key, $combinedStyle);
  }
 
?> 