<?php
  /*
  For example, looks like:Z
  )key_0:layer:basicui
  )key_0:scale:0.004 0.01 0.004
  )key_0:value:Current Object
  )value_0:layer:basicui
  )value_0:scale:0.004 0.01 0.004
  )value_0:details-editabletext:true
  )value_0:value:
  )value_0:details-binding:object_name
  (key_0:layer:basicui
  (key_0:type:horizontal
  (key_0:backpanel:true
  (key_0:tint:0.05 0.05 0.05 1
  (key_0:margin:0.02
  (key_0:spacing:0.02
  (key_0:minwidth:0.36
  (key_0:elements:)key_0,)value_0*/
  $keyname = ")key_" . $i;
  createElement($keyname, $default_key, [ "value" => $data["key"] ]);


  $readonly = false;
  if (array_key_exists("readonly", $data)){
    $readonly = $data["readonly"];
  }

  $valuename = ")value_" . $i;



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
    } 
    createElement($valuename, $default_value, $style);
  }

  $holdername = "(" . $unique_control_id . "_textfield_holder_" . $i;

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
  createElement($rootElementName, $default_rootLayout, [ "elements" => $keyname . "," . $holdername ]);
?>