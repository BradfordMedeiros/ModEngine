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

  function includeTemplate($file, $rootElementName, $i, $data,  $unique_control_id){
    $default_text_style = [
      "layer" => "basicui", 
      "scale" => "0.004 0.01 0.004",
    ];
    $extra_key_attrs = [];
    $extra_value_attrs = [ "details-editabletext" => "true" ];
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

    include $file;
  }

  $target_type = $argv[1];


  $vec3Type = [
    [ "type" => "float", "name" => "x", "value" => ".-3.4" ], 
    [ "type" => "float", "name" => "y",  "value" => "..345" ], 
    [ "type" => "slider", "name" => "z",  "value" => ".4.34" ]
  ];
  $vec4Type = [
    [ "type" => "float", "name" => "x", "value" => ".-3.4" ], 
    [ "type" => "float", "name" => "y",  "value" => "..345" ], 
    [ "type" => "float", "name" => "z",  "value" => ".4.34" ],
    [ "type" => "float", "name" => "w",  "value" => ".4.34" ]
  ];

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
          "type" => "numeric",
          "data" => [
            "key" => "position", 
            "value" => $vec3Type,
          ],
        ],
        [ "type" => "label", 
          "data" => [
            "key" => "Mesh", 
            "value" => [
              "binding" => "mesh",
            ],
          ]
        ],
        [ "type" => "label", 
          "data" => [
            "key" => "Look at Target", 
            "value" => [
              "binding" => "lookat",
            ],
          ]
        ],
        [ "type" => "label", 
          "data" => [
            "key" => "Render Layer", 
            "value" => [
              "binding" => "layer",
            ],
          ]
        ],
        /*[
          "type" => "options",
          "data" => [
            "key" => "light-type", 
            "value" => "placeholder value",
            "selector" => "lighttype-selector",
            "options" => [
              [ "label" => "directional", "selector-index" => "dir" ],
              [ "label" => "spotlight", "selector-index" => "spot" ], 
              [ "label" => "point", "selector-index" => "point" ],
            ],
          ],
        ],
        [texture
          "type" => "options",
          "data" => [
            "key" => "light-type", 
            "value" => "placeholder value",
            "selector" => "lighttype-selector",
            "options" => [
              [ "label" => "directional", "selector-index" => "dir" ],
              [ "label" => "spotlight", "selector-index" => "spot" ], 
              [ "label" => "point", "selector-index" => "point" ],
            ],
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "enable physics", 
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "scale", 
            "value" => $vec3Type,
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "rotation", 
            "value" => $vec4Type,
          ],
        ],
        [
          "type" => "list",
          "data" => [
            "key" => "Object Types",
            "values" => [
              ["image" => "./res/scenes/editor/dock/images/camera.png", "action" => "create-camera" ],
              ["image" => "./res/scenes/editor/dock/images/light.png", "action" => "create-light" ],
              ["image" => "./res/scenes/editor/dock/images/sound.png", "action" => "create-sound" ],
            ],
          ],
        ],*/
      ]
    ],
    "world_state" => [
      "title" => "World State",
      "items" => [
      /*  [
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
        ]*/
      ],
    ],
      "object_tools" => [
      "title" => "Object Tools",
      "items" => [
         [
          "type" => "label",
          "data" => [
            "key" => "Manipulator Tools", 
            "value" => ""
          ],
        ],
        [
          "type" => "list",
          "data" => [
            "key" => "Transform Mode",
            "values" => [
              ["image" => "./res/scenes/editor/dock/images/camera.png", "action" => "create-camera" ],
              ["image" => "./res/scenes/editor/dock/images/light.png", "action" => "create-light" ],
              ["image" => "./res/scenes/editor/dock/images/sound.png", "action" => "create-sound" ],
            ],
          ],
        ],
      ],
    ],
  ];

  $detailType = $mappingPerType[$target_type];
  echo (")title:value:" . $detailType["title"] . "\n");
  
  $test_panel_elements = [")title"];

  $keyvaluePairs = $detailType["items"];

  $typeToTemplate = [
    "label" => "./dock/details_textfield.php",
    "options" => "./dock/details_options.php",
    "checkbox" => "./dock/details_checkbox.php",
    "list" => "./dock/details_list.php",
    "numeric" => "./dock/details_numeric.php",
  ];


  for ($i = 0; $i < count($keyvaluePairs); $i++){
    $type = $keyvaluePairs[$i]["type"];
    $data = $keyvaluePairs[$i]["data"];
    $unique_control_id = "key_" . $i;

    $rootElementName = "(" . $unique_control_id;
    $templateFile = $typeToTemplate[$type];
    includeTemplate($templateFile, $rootElementName, $i, $data, $unique_control_id);
    array_push($test_panel_elements, $rootElementName);

    echo ("\n");
  }

  $test_panel_elements =  array_reverse($test_panel_elements);
  echo ("(test_panel:elements:" . implode(",", $test_panel_elements) . "\n");
?>

