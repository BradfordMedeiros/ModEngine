

(test_panel:layer:basicui
(test_panel:type:vertical
(test_panel:backpanel:true
(test_panel:tint:0.1 0.1 0.1
(test_panel:margin:0.02
(test_panel:spacing:0.02
(test_panel:minwidth:1
(test_panel:minheight:2

)title:value:Object Details
)title:layer:basicui
)title:scale:0.01 0.01 0.01


<?php 
  
  /*(keyval0:layer:basicui
(keyval0:type:horizontal
(keyval0:backpanel:true
(keyval0:tint:0.1 0.1 0.1
(keyval0:margin:0.02
(keyval0:spacing:0.02
(keyval0:elements:)key0,)val0

)key0:value:key 0
)key0:layer:basicui
)key0:scale:0.01 0.01 0.01

)val0:value:value 0
)val0:layer:basicui
)val0:scale:0.01 0.01 0.01
*/
  function createElement($name, $attr, $more_attr){
    foreach ($attr as $key => $value){
      echo ($name . ":" . $key . ":" . $value . "\n");
    }
    foreach ($more_attr as $key => $value){
      echo ($name . ":" . $key . ":" . $value . "\n");
    }
  }

  $test_panel_elements = [")title"];
  $default_keyvalue =  [
    "layer" => "basicui", 
    "scale" => "0.01 0.01 0.01",
  ];

  $default_keyvalueLayout = [
    "layer" => "basicui",
    "type" => "horizontal",
    "backpanel" => "true",
    "tint" => "0.8 0.1 0.1",  # doesn't show up since z ordering
    "margin" => "0.02",
    "spacing" => "0.02",
  ];
   
  $keyvaluePairs = [
    ["label" => "Current Object", "value" => "platform"],
    ["label" => "position", "value" => "- 0 0 0"],
    ["label" => "scale", "value" => "- 1 1 1"],
  ];

  for ($i = 0; $i < count($keyvaluePairs); $i++){
    $keyname = ")key_" . $i;
    createElement($keyname, $default_keyvalue, [ "value" => $keyvaluePairs[$i]["label"] ]);

    $valuename = ")value_" . $i;
    createElement($valuename, $default_keyvalue, [ "value" => $keyvaluePairs[$i]["value"] ]);

    $keyvalueLayout = "(keyval_" . $i;
    createElement($keyvalueLayout, $default_keyvalueLayout, [ "elements" => $keyname . "," . $valuename ]);

    echo ("\n");


    array_push($test_panel_elements, $keyvalueLayout);
  }

  $test_panel_elements =  array_reverse($test_panel_elements);

  echo ("(test_panel:elements:" . implode(",", $test_panel_elements) . "\n");

?>

