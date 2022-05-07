<?php
  if (count ($argv) < 2){
    print("Must define which detail to build\n");
    exit(1);
  }
?>
(test_panel:layer:basicui
(test_panel:type:vertical
(test_panel:backpanel:true
(test_panel:tint:0.1 0.1 0.1
(test_panel:margin:0.02
(test_panel:margin-top:0.1
(test_panel:spacing:0.02
(test_panel:minheight:2

(test_panel:align-items-horizontal:left   # left/center/right
(test_panel:align-items-vertical:up     # up/center/down
(test_panel:border-size:0.004
(test_panel:border-color:0.3 0.3 0.3
(test_panel:position:-0.8 -0.1 0   # hackey to hardcode this position, but whatever!

)title:layer:basicui
)title:scale:0.008 0.02 0.008
)title:tint:1 1 2


<?php 
  function createElement($name, $attr, $more_attr){
    foreach ($attr as $key => $value){
      echo ($name . ":" . $key . ":" . $value . "\n");
    }
    foreach ($more_attr as $key => $value){
      echo ($name . ":" . $key . ":" . $value . "\n");
    }
  }

  $target_type = $argv[1];

  $mappingPerType = [
    "object_details" => [
      "title" => "Object Details",
      "items" => [
        ["label" => "Current Object", "value" => "platform"],
        ["label" => "position", "value" => "- 0 0 0"],
        ["label" => "scale", "value" => "- 1 1 1"],
      ]
    ],
    "world_state" => [
      "title" => "World State",
      "items" => [
        ["label" => "bloom", "value" => "enabled"],
        ["label" => "color", "value" => "- 1 1 1"],
      ],
    ],
  ];

  $detailType = $mappingPerType[$target_type];
  echo (")title:value:" . $detailType["title"] . "\n");
  
  $test_panel_elements = [")title"];
  $default_text_style = [
    "layer" => "basicui", 
    "scale" => "0.004 0.01 0.004",
  ];
  $extra_key_attrs = [
   // "tint" => "2 2 2",
  ];
  $extra_value_attrs = [
    //"tint" => "1 1 1",
  ];
  $default_key = array_merge($default_text_style, $extra_key_attrs);
  $default_value = array_merge($default_text_style, $extra_value_attrs);

  $default_keyvalueLayout = [
    "layer" => "basicui",
    "type" => "horizontal",
    "backpanel" => "true",
    "tint" => "0.05 0.05 0.05",  # doesn't show up since z ordering
    "margin" => "0.02",
    "spacing" => "0.02",
    "minwidth" => "0.4",
  ];

  $keyvaluePairs = $detailType["items"];
  for ($i = 0; $i < count($keyvaluePairs); $i++){
    $keyname = ")key_" . $i;
    createElement($keyname, $default_key, [ "value" => $keyvaluePairs[$i]["label"] ]);

    $valuename = ")value_" . $i;
    createElement($valuename, $default_value, [ "value" => $keyvaluePairs[$i]["value"] ]);

    $keyvalueLayout = "(keyval_" . $i;
    createElement($keyvalueLayout, $default_keyvalueLayout, [ "elements" => $keyname . "," . $valuename ]);

    echo ("\n");


    array_push($test_panel_elements, $keyvalueLayout);
  }

  $test_panel_elements =  array_reverse($test_panel_elements);

  echo ("(test_panel:elements:" . implode(",", $test_panel_elements) . "\n");

?>
