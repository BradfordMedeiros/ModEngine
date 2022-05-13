<?php
  if (count ($argv) < 2){
    print("Must define which detail to build\n");
    exit(1);
  }
?>
(test_panel:layer:basicui
(test_panel:type:vertical
(test_panel:backpanel:true
(test_panel:tint:0.1 0.1 0.1 1
(test_panel:margin:0.02
(test_panel:margin-top:0.1
(test_panel:spacing:0.02
(test_panel:minheight:2

(test_panel:align-items-horizontal:left   # left/center/right
(test_panel:align-items-vertical:up     # up/center/down
(test_panel:border-size:0.004
(test_panel:border-color:0.3 0.3 0.3 1
(test_panel:position:-0.8 -0.1 0   # hackey to hardcode this position, but whatever!

)window_x:layer:basicui
)window_x:scale:0.008 0.016 0.008
)window_x:tint:0.8 0.8 0.8 1
)window_x:value:x
)window_x:position:0.18 0.95 0

(test_panel:child:(banner_title_background_right,(banner_title_background_left,)window_x
)banner_title_right:layer:basicui
)banner_title_right:scale:0.008 0.01 0.004
)banner_title_right:tint:2 2 2 1
)banner_title_right:value:Object Details
)banner_title_right:rotation:0 0 -1 -90
(banner_title_background_right:position:0.23 0 0
(banner_title_background_right:layer:basicui
(banner_title_background_right:backpanel:true
(banner_title_background_right:tint:0 0 0.8 1
(banner_title_background_right:minheight:2
(banner_title_background_right:minwidth:0.02
(banner_title_background_right:border-size:0.002
(banner_title_background_right:border-color:0 0 0
(banner_title_background_right:child:)banner_title_right

)banner_title_left:layer:basicui
)banner_title_left:scale:0.008 0.01 0.004
)banner_title_left:tint:2 2 2 1
)banner_title_left:value:Object Details
)banner_title_left:rotation:0 0 -1 -90
(banner_title_background_left:position:-0.23 0 0
(banner_title_background_left:layer:basicui
(banner_title_background_left:backpanel:true
(banner_title_background_left:tint:0 0 0.8 1
(banner_title_background_left:minheight:2
(banner_title_background_left:minwidth:0.02
(banner_title_background_left:border-size:0.002
(banner_title_background_left:border-color:0 0 0
(banner_title_background_left:child:)banner_title_left

)title:layer:basicui
)title:scale:0.008 0.02 0.008
)title:tint:1 1 2 1


<?php 
  function createElement($name, $attr, $more_attr){
    $combined = array_merge($attr, $more_attr);
    foreach ($combined as $key => $value){
      echo ($name . ":" . $key . ":" . $value . "\n");
    }
  }

  $target_type = $argv[1];

  # data sources ideas (not implemented)
  /*
      # data bindings should be based upon per pain writing a script to return key:value map
      # in that script i can then eg write  a sql or gameobj-attr functions or whatever

  */
  # types:
  # label: simple read-only display of text data. 
  # key: left side text display
  # value: right side text display
  # binding: 

  $mappingPerType = [
    "object_details" => [
      "title" => "Object Details",
      "items" => [
        [ "type" => "label", 
          "data" => [
            "key" => "Current Object", 
            "value" => [
              "binding" => "object_name",
            ],
          ]
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "position", 
            "value" => "- 0 0 0"
          ],
        ],
        [
          "type" => "options",
          "data" => [
            "key" => "light-type", 
            "value" => "placeholder value",
            "options" => [
              "directional", "spotlight", "point",
            ],
          ],
        ],
      ]
    ],
    "world_state" => [
      "title" => "World State",
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "bloom", 
            "value" => "enabled"
          ],
        ],
        [
          "type" => "label", 
          "data" => [
            "key" => "color", 
            "value" => "- 1 1 1"
          ]
        ]
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
  ];
  $extra_value_attrs = [
    "details-editabletext" => "true"
  ];
  $default_key = array_merge($default_text_style, $extra_key_attrs);
  $default_value = array_merge($default_text_style, $extra_value_attrs);

  $default_keyvalueLayout = [
    "layer" => "basicui",
    "type" => "horizontal",
    "backpanel" => "true",
    "tint" => "0.05 0.05 0.05 1",  # doesn't show up since z ordering
    "margin" => "0.02",
    "spacing" => "0.02",
    "minwidth" => "0.36",
  ];

  $keyvaluePairs = $detailType["items"];
  for ($i = 0; $i < count($keyvaluePairs); $i++){
    $type = $keyvaluePairs[$i]["type"];
    $data = $keyvaluePairs[$i]["data"];

    $unique_control_id = "key_" . $i;
    if ($type == "label"){
      $keyname = ")key_" . $i;
      createElement($keyname, $default_key, [ "value" => $data["key"] ]);

      $valuename = ")value_" . $i;
      if (is_string($data["value"])){
        createElement($valuename, $default_value, [ "value" =>  $data["value"] ]);
      }else{
        createElement($valuename, $default_value, [ "value" =>  "", "details-binding" => $data["value"]["binding"]] );
      }

      $rootElementName = "(" . $unique_control_id;
      createElement($rootElementName, $default_keyvalueLayout, [ "elements" => $keyname . "," . $valuename ]);
      array_push($test_panel_elements, $rootElementName);
    }else if ($type == "options"){
      $titleName = ")" . $unique_control_id . "_label";
      createElement($titleName, $default_text_style, [ "value" => "light-type" ]);

      $options = $data["options"];
      $optionElements = [];
      for ($optionIndex = 0; $optionIndex < count($options); $optionIndex++){
        $optionName = $options[$optionIndex];
        $optionElementName = ")" . $unique_control_id . "_" . "option_" . $optionIndex;
        createElement($optionElementName, $default_key, [ "value" => $optionName ]);
        array_push($optionElements, $optionElementName);
      }

      $optionsLayout = "(" . $unique_control_id . "_options";
      createElement($optionsLayout, $default_keyvalueLayout, [ "elements" => implode(",", $optionElements) ]);
      $rootElementName = "(" . $unique_control_id;
      createElement($rootElementName, $default_keyvalueLayout, [ 
        "tint" => "0 0 1 0.8", 
        "type" => "vertical", 
        "elements" => $optionsLayout . "," . $titleName  
      ]);
      array_push($test_panel_elements, $rootElementName);
    }else{
      print("Key value pairs: invalid type - " . $type . "\n");
      exit(1);
    }

    echo ("\n");


  }

  $test_panel_elements =  array_reverse($test_panel_elements);

  echo ("(test_panel:elements:" . implode(",", $test_panel_elements) . "\n");

/*


)key_2_option1:layer:basicui
)key_2_option1:scale:0.004 0.01 0.004
)key_2_option1:value:point

)key_2_option2:layer:basicui
)key_2_option2:scale:0.004 0.01 0.004
)key_2_option2:value:directional
*/
?>

