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

  $valuename = ")value_" . $i;
  if (is_string($data["value"])){
    createElement($valuename, $default_value, [ "value" =>  $data["value"], "details-editabletext" => "true"  ]);
  }else{
    createElement($valuename, $default_value, [ "value" =>  "", "details-binding" => $data["value"]["binding"], "details-editabletext" => "true"  ] );
  }
  createElement($rootElementName, $default_rootLayout, [ "elements" => $keyname . "," . $valuename ]);
?>