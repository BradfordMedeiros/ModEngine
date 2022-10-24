<?php
  
  include_once "common/createTextbox.php";
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

  $holdername = "(" . $unique_control_id . "_textfield_holder_" . $i;
  $valuename = ")value_" . $i;

  $readonly = false;
  if (array_key_exists("readonly", $data)){
    $readonly = $data["readonly"];
  }
  $type = NULL;
  if (array_key_exists("type", $data["value"])){
    $type = $data["value"]["type"];
  }

  $valueFromSelection = false;
  if (array_key_exists("valueFromSelection", $data["value"]) && $data["value"]["valueFromSelection"]){
    $valueFromSelection = true;
  }

  createTextbox($holdername, $valuename, $readonly, $type, $data["value"], $valueFromSelection, $styles);

  createElement($rootElementName, $default_rootLayout, [ "elements" => $keyname . "," . $holdername ]);
?>