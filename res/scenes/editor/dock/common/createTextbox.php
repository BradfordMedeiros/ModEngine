<?php

  function createTextbox($holdername, $valuename, $data, $styles){
    $default_key = $styles["default_key"];
    $default_value = $styles["default_value"];

    $readonly = false;
    if (array_key_exists("readonly", $data)){
      $readonly = $data["readonly"];
    }
 
    if (is_string($data["value"])){
      $style = [ 
        "value" =>  $data["value"], 
      ];
      if (!$readonly){
        $style["details-editabletext"] = "true";
      }
      createElement($valuename, $default_value, $style);
    }else{
      $style = [ 
        "value" =>  "", 
        "details-binding" => $data["value"]["binding"], 
 
      ];
      if (!$readonly){
        $style["details-editabletext"] = "true";
        $style["wrapamount"] = "10"; # size limit
        $style["wraptype"] = "char";
        $style["maxheight"] = "1";
      } 
      createElement($valuename, $default_value, $style);
    }
 
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