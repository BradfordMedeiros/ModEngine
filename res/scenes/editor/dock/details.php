<?php
  if (count ($argv) < 2){
    print("Must define which detail to build\n");
    exit(1);
  }

  $zpos = -1;  # + or +- 0.2 for certain order of elements within

?>
(test_panel:layer:basicui
(test_panel:type:vertical
#(test_panel:margin:0.02
#(test_panel:spacing:0.02

(test_panel:position:-0.78 -0.097 <?php echo($zpos . "\n"); ?>
(test_panel:align-content:neg


<?php 
  function createElement($name, $attr, $more_attr){
    $combined = array_merge($attr, $more_attr);
    foreach ($combined as $key => $value){
      echo ($name . ":" . $key . ":" . $value . "\n");
    }
  }

  function includeTemplate($file, $rootElementName, $i, $data,  $unique_control_id, $zpos, $sqlBinding, $depth, $default_rootLayout ){
    $default_style = [ "layer" => "basicui" ];
    $default_text_style = [
      "layer" => "basicui", 
      "scale" => "0.004 0.01 0.004",
      "position" => "0 0 " . $depth[3],
    ];
    $default_key = $default_text_style;
    $default_value = array_merge($default_text_style, []);

    $styles = [
      "default_key" => $default_key,
      "default_value" => $default_value,
    ];
    
    $default_keyvalueLayout = [
      "layer" => "basicui",
      "type" => "horizontal",
      "backpanel" => "true",
      "tint" => "0.2 0.2 0.2 1",  
      "margin" => "0.04",
      "spacing" => "0.02",
      "minwidth" => "0.36",
      "position" => "0 0 " . $depth[2],
    ];

    if ($sqlBinding){
      $default_rootLayout["sql-binding"] = $sqlBinding["binding"];
      $default_rootLayout["sql-query"] = $sqlBinding["query"];
      if (array_key_exists("update", $sqlBinding)){
        $default_rootLayout["sql-update"] = $sqlBinding["update"];
      }
      if (array_key_exists("cast", $sqlBinding)){
        $default_rootLayout["sql-cast"] = $sqlBinding["cast"];
      }
    }
    include $file;
  }

  $target_type = $argv[1];


  $vec3Type = [
    [ "type" => "float", "name" => "x", "value" => ".-3.4" ], 
    //[ "type" => "float", "name" => "y",  "value" => "..345" ], 
    //[ "type" => "slider", "name" => "z",  "value" => ".4.34" ]
  ];
  $vec4Type = [
    [ "type" => "float", "name" => "x", "value" => ".-3.4" ], 
    [ "type" => "float", "name" => "y",  "value" => "..345" ], 
    [ "type" => "float", "name" => "z",  "value" => ".4.34" ],
    [ "type" => "float", "name" => "w",  "value" => ".4.34" ]
  ];

  $mappingPerType = [
    "not_implemented" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Panel is Not Yet Implenented",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding",
            ]
          ],
        ],
      ],
    ],
    "cameras" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Modify Camera Settings",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Create Camera",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ],
            "action" => "create-camera",
            "tint" => "1 1 0 1",
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "depth of field", 
            "value" => [
              "binding" => "gameobj:dof",  
              "binding-on" => "enabled",
              "binding-off" => "disabled",
            ],
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "blur distances for depth of field effect", 
            "value" => [
              [ 
                "type" => "slider", 
                "name" => "min blur", 
                "value" => [ 
                  "binding" => "gameobj:minblur", 
                  "min" => -100,  // what should min and max really be?d
                  "max" => 100,
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "maxblur", 
                "value" => [ 
                  "binding" => "gameobj:maxblur", 
                  "min" => -100,
                  "max" => 100,
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "amount", 
                "value" => [ 
                  "binding" => "gameobj:bluramount", 
                  "min" => 0,
                  "max" => 100,
                ]
              ]
            ]
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Depth of Field Target",
            "value" => [
              "valueFromSelection" => true,
              "binding" => "gameobj:target",
            ]
          ],
        ],
      ],
    ],
    "lights" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Modify Light Settings",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ],
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Create Light",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ],
            "action" => "create-light",
            "tint" => "1 1 0 1",
          ],
        ],
        [
          "type" => "options",
          "data" => [
            "key" => "type", 
            "options" => [
              [ "label" => "point", "binding" => "gameobj:type", "binding-on" => "point" ],
              [ "label" => "spotlight", "binding" => "gameobj:type", "binding-on" => "spotlight" ],
              [ "label" => "directional", "binding" => "gameobj:type", "binding-on" => "directional" ],
            ],
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "color", 
            "value" => [
              [ 
                "type" => "slider", 
                "name" => "red", 
                "value" => [ 
                  "binding" => "gameobj:color", 
                  "binding-index" =>  0,
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "green", 
                "value" => [ 
                  "binding" => "gameobj:color", 
                  "binding-index" =>  1,
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "blue", 
                "value" => [ 
                  "binding" => "gameobj:color", 
                  "binding-index" =>  2,
                ]
              ]
            ]
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "attenuation", 
            "value" => [
              [ 
                "type" => "slider", 
                "name" => "constant", 
                "value" => [ 
                  "binding" => "gameobj:attenuation", 
                  "binding-index" =>  0,
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "linear", 
                "value" => [ 
                  "binding" => "gameobj:attenuation", 
                  "binding-index" =>  1,
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "quadratic", 
                "value" => [ 
                  "binding" => "gameobj:attenuation", 
                  "binding-index" =>  2,
                ]
              ],
            ]
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "angle", 
            "value" => [
              [ 
                "type" => "slider", 
                "name" => "angle", 
                "value" => [ 
                  "binding" => "gameobj:angle", 
                  "min" => -1,
                  "max" => 1,
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "delta", 
                "value" => [ 
                  "binding" => "gameobj:angledelta", 
                  "min" => 0,
                  "max" => 1,
                ]
              ],
            ]
          ],
        ],
      ],
    ],
    "sound" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Modify Sound Settings",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "loop", 
            "value" => [
              "binding" => "gameobj:loop",  
              "binding-on" => "true",
              "binding-off" => "false",
            ],
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "volume", 
            "value" => [
              [ 
                "type" => "float", 
                "name" => "volume", 
                "value" => [ 
                  "binding" => "gameobj:volume", 
                  "type" => "number",
                ]
              ],
            ]
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Path to Clip",
            "readonly" => true,
            "value" => [
              "binding" => "gameobj:clip",
            ]
          ],
        ],
      ],
    ],
    "text" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Modify Text Settings",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Create Text",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ],
            "action" => "create-text",
            "tint" => "1 1 0 1",
          ],
        ],
        [ "type" => "label", 
          "data" => [
            "key" => "text", 
            "value" => [
              "binding" => "gameobj:value",
            ],
          ]
        ],
        [
          "type" => "options",
          "data" => [
            "key" => "align", 
            "options" => [
                [ "label" => "left", "binding" => "gameobj:align", "binding-on" => "left" ],
                [ "label" => "center", "binding" => "gameobj:align", "binding-on" => "center" ],
                [ "label" => "right", "binding" => "gameobj:align", "binding-on" => "right" ],
            ],
          ],
        ],
        [
          "type" => "options",
          "data" => [
            "key" => "wraptype", 
            "options" => [
                [ "label" => "none", "binding" => "gameobj:wraptype", "binding-on" => "none" ],
                [ "label" => "char", "binding" => "gameobj:wraptype", "binding-on" => "char" ],
            ],
          ],
        ],

        [
          "type" => "numeric",
          "data" => [
            "key" => "wrapamount", 
            "value" => [
              [ 
                "type" => "float", 
                "name" => "wrapamount", 
                "value" => [ 
                  "binding" => "gameobj:wrapamount", 
                  "type" => "number",
                ]
              ],
            ]
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "font",
            "value" => [
              "binding" => "gameobj:font",
            ]
          ],
        ],

        [
          "type" => "numeric",
          "data" => [
            "key" => "offset", 
            "value" => [
              [ 
                "type" => "float", 
                "name" => "offsetx", 
                "value" => [ 
                  "binding" => "gameobj:offsetx", 
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "offsety", 
                "value" => [ 
                  "binding" => "gameobj:offsety", 
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "offset", 
                "value" => [ 
                  "binding" => "gameobj:offset", 
                  "type" => "number",
                ]
              ],
            ]
          ],
        ],


      ],
    ],
    "object_details" => [
      "title" => "Object Details",
      "items" => [
        /*[ "type" => "label", 
          "sql" => [
            "binding" => "sql-person-name",
            "query" => "select name from people where class = warrior limit 1",
            "update" => 'update people set people.name = $VALUE where people.class = warrior',
            "cast" => "string", # string, number (vec not yet implemented)
          ],  
          "data" => [
            "key" => "Another Label", 
            "value" => [
              "binding" => "sql-person-name",
            ],
          ]
        ],*/
        [ "type" => "label", 
          "data" => [
            "key" => "Current Object", 
            "value" => [
              "binding" => "object_name",
              "type" => "number",
            ],
          ]
        ],
        /*[ "type" => "label", 
          "data" => [
            "key" => "Look at Target", 
            "value" => [
              "binding" => "lookat",
            ],
          ]
        ],
        [ "type" => "label", 
          "data" => [
            "key" => "No Data Source Test", 
            "value" => [
              "binding" => "nodata",
            ],
          ]
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "toggle particles", 
            "value" => [
              "binding" => "state",
              "binding-on" => "enabled",
              "binding-off" => "disabled",
            ],
          ],
        ],
        */

        /*[
          "type" => "checkbox",
          "sql" => [
            "binding" => "multiday-ticket",
            "query" => "select tickets.multiday from tickets where tickets.name = single",
            "update" => 'update tickets set tickets.multiday  = $VALUE where tickets.name = single',
            "cast" => "string",
          ],
          "data" => [
            "key" => "Is Multiday Ticket?", 
            "value" => [
              "binding" => "multiday-ticket",
              "binding-on" => "true",
              "binding-off" => "false",
            ],
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "enable physics", 
            "value" => [
              "binding" => "physics",
              "binding-on" => "enabled",
              "binding-off" => "disabled",
            ],
          ],
        ],
        
         [
          "type" => "options",
          "sql" => [
            "binding" => "sql-tree-type",
            "query" => "select tree_type from trees where growth_rate = medium limit 1",
            "update" => 'update trees set trees.tree_type  = $VALUE where trees.growth_rate = medium',
            "cast" => "string",
          ],  
          "data" => [
            "key" => "treetype", 
            "options" => [
              [ "label" => "big tree", "binding" => "sql-tree-type", "binding-on" => "big_tree" ],
              [ "label" => "medium tree", "binding" => "sql-tree-type", "binding-on" => "medium_tree" ],
              [ "label" => "small tree", "binding" => "sql-tree-type", "binding-on" => "small_tree" ],
   
            ],
          ],
        ],*/

        /*[
          "type" => "numeric",
          "data" => [
            "key" => "Gravity", 
            "value" => [
              [ 
                "type" => "slider", 
                "name" => "Up/Down", 
                "value" => [ 
                  "binding" => "physics_gravity", 
                  "binding-index" =>  0,
                ]
              ],
            ]
          ],
        ],*/
        /*[
          "type" => "checkbox",
          "data" => [
            "key" => "Toggle Red", 
            "value" => [
              "binding" => "tint",
              "binding-on" => "1 0 0 1",
              "binding-off" => "1 1 1 1",
            ]
          ],
        ],*/
        /*[
          "type" => "numeric",
          "sql" => [
            "binding" => "sql-trait-speed",
            "query" => 'select people.topspeed from people where people.name = $object_name',
            "update" => 'update people set people.topspeed = $VALUE where people.name = john',
            "cast" => "number",
          ], 
          "data" => [
            "key" => "Physics Tuning", 
            "value" => [
              [ 
                "type" => "float", 
                "name" => "physics max speed", 
                "value" => [ 
                  "binding" => "sql-trait-speed", 
                  "type" => "number",
                  "min" => 0,
                  "max" => 100,
                ],
              ],
            ]
          ],
        ],
        [
          "type" => "numeric",
          "sql" => [
            "binding" => "sql-trait-color",
            "query" => "select people.color from people where people.name = john",
            "update" => 'update people set people.color = $VALUE where people.name = john',
            #"update" => "select people.color from people where people.name = john",
            "cast" => "vec",
          ], 
          "data" => [
            "key" => "Vec4Demo", 
            "value" => [
              [ 
                "type" => "float", 
                "name" => "class-color", 
                "value" => [ 
                  "binding" => "sql-trait-color",
                  "binding-index" => 3, 
                  "type" => "number",
                  "min" => 0,
                  "max" => 100,
                ],
              ],
            ]
          ],
        ],*/
        [
          "type" => "numeric",
          "data" => [
            "key" => "Tint", 
            "value" => [
              [ 
                "type" => "slider", 
                "name" => "Red", 
                "value" => [ 
                  "binding" => "gameobj:tint", 
                  "binding-index" =>  0,
                  "type" => "positive-number",
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "Green", 
                "value" => [ 
                  "binding" => "gameobj:tint", 
                  "binding-index" =>  1,
                  "type" => "positive-number",
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "Blue", 
                "value" => [ 
                  "binding" => "gameobj:tint", 
                  "binding-index" =>  2,
                  "type" => "positive-number",
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "Alpha", 
                "value" => [ 
                  "binding" => "gameobj:tint", 
                  "binding-index" =>  3,
                  "type" => "positive-number",
                ]
              ],
            ]
          ],
        ],
        /*[ "type" => "label", 
          "data" => [
            "key" => "Mesh", 
            "value" => [
              "binding" => "mesh",
            ],
          ]
        ],/*
        [ "type" => "label", 
          "data" => [
            "key" => "Render Layer", 
            "value" => [
              "binding" => "layer",
            ],
          ]
        ],*/
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
        ],/*
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
    "scene_info" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Number of Scenes",
            "readonly" => true,
            "value" => [
              "binding" => "meta-numscenes",
            ]
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Object Id", 
            "value" => [
              "binding" => "gameobj:tint", 
              "binding-index" =>  0,
              "type" => "positive-integer",
            ]
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Scene Id", 
            "value" => [
              "binding" => "runtime-sceneid",
            ]
          ],
        ],
      ],
    ],
    "object_tools" => [
      "minheight" => false,
      "minwidth" => false,
      "hidex" => true,
      "vertical" => "center",
      "horizontal" => "center",
      "tint" => false,
      "items" => [
        [
          "type" => "list",
          "data" => [
            "key" => "",
            "values" => [
              [ 
                "image" => "./res/scenes/editor/dock/images/play.png", 
                "action" => "toggle-play-mode",
                "binding" => "play-mode-on", 
                "binding-on" => "on",
                "binding-off" => "off",
              ],
              [ "image" => "./res/scenes/editor/dock/images/pause.png", 
                "action" => "toggle-pause-mode",
                "binding" => "pause-mode-on", 
                "binding-on" => "on",
                "binding-off" => "off",
              ],
            ],
          ],
        ],
      ],
    ],
    "transform" => [
      "minheight" => false,
      "minwidth" => false,
      "hidex" => true,
      "vertical" => "center",
      "horizontal" => "center",
      "tint" => "0 0 0 0.3",
      "margin-top" => false,
      "spacing" => false,
      "items" => [
        [
          "type" => "list",
          "data" => [
            "key" => "",
            "values" => [
              [ 
                "image" => "./res/scenes/editor/dock/images/transform.png", 
                "action" => "set-transform-mode",
                "size" => "0.02 0.05 0.02",
              ],
              [ "image" => "./res/scenes/editor/dock/images/scale.png", 
                "action" => "set-scale-mode",
                "size" => "0.02 0.05 0.02",
              ],
              [ "image" => "./res/scenes/editor/dock/images/rotate.png", 
                "action" => "set-rotate-mode",
                "size" => "0.02 0.05 0.02",
              ],
              [ "image" => "./res/scenes/editor/dock/images/x.png", 
                "action" => "set-axis-x",
                "size" => "0.02 0.05 0.02",
              ],
              [ "image" => "./res/scenes/editor/dock/images/y.png", 
                "action" => "set-axis-y",
                "size" => "0.02 0.05 0.02",
              ],
              [ "image" => "./res/scenes/editor/dock/images/z.png", 
                "action" => "set-axis-z",
                "size" => "0.02 0.05 0.02",
              ],
              [ "image" => "./res/scenes/editor/dock/images/copy.png", 
                "action" => "copy-object",
                "size" => "0.02 0.05 0.02",
              ],
            ],
          ],
        ],
      ],
    ],
    "scenegraph" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Scenegraph",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
        [
          "type" => "list",
          "data" => [
            "key" => "",
            "mode" => "oneof",
            "values" => [
              ["image" => "gentexture-scenegraph", "action" => "set-rotate-mode", "size" => "0.42 1 0.16" ],
            ],
            "type" => "vertical",
          ],
        ],
      ],
    ],
    "performance" => [
      "title" => "Performance",
      "items" => [
        [
          "type" => "list",
          "data" => [
            "key" => "",
            "values" => [
              ["image" => "graphs-testplot", "action" => "set-rotate-mode", "size" => "0.42 -1.2 0.16" ],
            ],
            "type" => "vertical",
          ],
        ],
      ],
    ],
    "world" => [
      "items" => [
        [
          "type" => "numeric",
          "data" => [
            "key" => "ambient amount", 
            "value" => [
              [ 
                "type" => "slider", 
                "name" => "red", 
                "value" => [ 
                  "binding" => "world:light:amount", 
                  "binding-index" => 0,
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "green", 
                "value" => [ 
                  "binding" => "world:light:amount", 
                  "binding-index" => 1,
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "blue", 
                "value" => [ 
                  "binding" => "world:light:amount", 
                  "binding-index" => 2,
                ]
              ],

            ]
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "enable skybox", 
            "value" => [
              "binding" => "world:skybox:enable",  
              "binding-on" => "true",
              "binding-off" => "false",
            ],
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "Skybox Color", 
            "value" => [
              [ 
                "type" => "slider", 
                "name" => "Red", 
                "value" => [ 
                  "binding" => "world:skybox:color",   
                  "binding-index" =>  0,
                  "type" => "positive-number",
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "Green", 
                "value" => [ 
                  "binding" => "world:skybox:color",
                  "binding-index" =>  1,
                  "type" => "positive-number",
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "Blue", 
                "value" => [ 
                  "binding" => "world:skybox:color",
                  "binding-index" =>  2,
                  "type" => "positive-number",
                ]
              ],
            ]
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "enable fog", 
            "value" => [
              "binding" => "world:fog:enabled",  
              "binding-on" => "true",
              "binding-off" => "false",
            ],
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "Fog Color", 
            "value" => [
              [ 
                "type" => "slider", 
                "name" => "Red", 
                "value" => [ 
                  "binding" => "world:fog:color",   // bindings are wrong 
                  "binding-index" =>  0,
                  "type" => "positive-number",
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "Green", 
                "value" => [ 
                  "binding" => "world:fog:color", 
                  "binding-index" =>  1,
                  "type" => "positive-number",
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "Blue", 
                "value" => [ 
                  "binding" => "world:fog:color", 
                  "binding-index" =>  2,
                  "type" => "positive-number",
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "mincutoff", 
                "value" => [ 
                  "binding" => "world:fog:mincutoff",   // bindings are wrong 
                  "min" => 0,
                  "max" => 1.1
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "maxcutoff", 
                "value" => [ 
                  "binding" => "world:fog:maxcutoff",   // bindings are wrong 
                  "min" => 0,
                  "max" => 1.1
                ]
              ],
            ]
          ],
        ],
      ],
    ],
    "rendering" => [
      "items" => [
        [
          "type" => "checkbox",
          "data" => [
            "key" => "enable diffuse", 
            "value" => [
              "binding" => "world:diffuse:enabled",  
              "binding-on" => "true",
              "binding-off" => "false",
            ],
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "enable specular", 
            "value" => [
              "binding" => "world:specular:enabled",  
              "binding-on" => "true",
              "binding-off" => "false",
            ],
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "bloom", 
            "value" => [
              [ 
                "type" => "slider", 
                "name" => "amount", 
                "value" => [ 
                  "binding" => "world:bloom:amount", 
                  "min" => 0,  
                  "max" => 5,
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "blur amount", 
                "value" => [ 
                  "binding" => "world:bloomblur:amount", 
                  "min" => 0, 
                  "max" => 1,
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "blur thresholder", 
                "value" => [ 
                  "binding" => "world:bloom:threshold", 
                  "min" => 0, 
                  "max" => 5,
                ]
              ],
            ]
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "enable bloom", 
            "value" => [
              "binding" => "world:bloom:enabled",  
              "binding-on" => "true",
              "binding-off" => "false",
            ],
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "enable exposure", 
            "value" => [
              "binding" => "world:exposure:enabled",  
              "binding-on" => "true",
              "binding-off" => "false",
            ],
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "enable gamma", 
            "value" => [
              "binding" => "world:gamma:enabled",  
              "binding-on" => "true",
              "binding-off" => "false",
            ],
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "Color correction", 
            "value" => [
              [ 
                "type" => "slider", 
                "name" => "exposure", 
                "value" => [ 
                  "binding" => "world:exposure:amount",   
                  "min" => 0,
                  "max" => 5,
                ]
              ],
            ]
          ],
        ],
      ],
    ],
  ];

  $detailType = $mappingPerType[$target_type];

  
  if (array_key_exists("minheight", $detailType)){
    if (!$detailType["minheight"] == false){
      echo("(test_panel:minheight:" . $detailType["minheight"] . "\n");
    }
  }else{
    echo("(test_panel:minheight:2\n");
  }

  if (array_key_exists("tint", $detailType)){
    if(!$detailType["tint"] == false){
      echo ("(test_panel:backpanel:true\n");
      echo ("(test_panel:tint:" . $detailType["tint"] . "\n");
    }
  }else{
    echo ("(test_panel:tint:" . "0.1 0.1 0.1 1" . "\n");
    echo ("(test_panel:backpanel:true\n");
  }

  if (array_key_exists("margin-top", $detailType)){
    if (!($detailType["margin-top"] == false)){
      echo ("(test_panel:margin-top:" . $detailType["margin-top"] . "\n");
    }
  }else{
    echo ("(test_panel:margin-top:0.1\n");
  }

  $multiplier = 1;
  $depth = [
    0 => $zpos,
    1 => ($zpos + $multiplier * 0.05),
    2 => ($zpos + $multiplier * 0.1),
    28 => ($zpos + $multiplier * 0.18),
    29 => ($zpos + $multiplier * 0.24),
    3 => ($zpos + $multiplier * 0.3),
  ];
  $default_rootLayout = [
    "layer" => "basicui",
    "type" => "horizontal",
    "backpanel" => "true",
    "tint" => "0.2 0.2 0.2 1",  
    "margin" => "0.02",
    "margin-left" => "0.01",
    "margin-right" => "0.01",
    "border-size" => "0.002",
    "border-color" => "0 0 0 1",
    "position" => "0 0 " . $depth[2],
  ];

  if (array_key_exists("spacing", $detailType)){
    if (!($detailType["spacing"] == false)){
      $default_rootLayout["spacing"] = $detailType["spacing"];
    }
  }else{
    $default_rootLayout["spacing"] = "0.02";
  }

  if (array_key_exists("minwidth", $detailType)){
    if (!$detailType["minwidth"] == false){
      echo("(test_panel:minwidth:" . $detailType["minwidth"] . "\n");
      $default_rootLayout["minwidth"] = $detailType["minwidth"];
    }
  }else{
    echo("(test_panel:minwidth:0.44\n");
    $default_rootLayout["minwidth"] = "0.44";
  }

  
  if (array_key_exists("horizontal", $detailType)){
    echo("(test_panel:align-items-horizontal:" . $detailType["horizontal"] . "\n");
  }else{
    echo("(test_panel:align-items-horizontal:left\n");
  }
  if (array_key_exists("vertical", $detailType)){
    echo("(test_panel:align-items-vertical:" . $detailType["vertical"] . "\n");
  }else{
    echo("(test_panel:align-items-vertical:up\n");
  }

  $window_elements = [
    ")window_x:layer:basicui",
    ")window_x:scale:0.008 0.016 0.008",
    ")window_x:tint:0.8 0.8 0.8 1",
    ")window_x:value:x",
    ")window_x:position:0.2 0.95 0",
    "(test_panel:child:)window_x",
  ];
  
  if (!(array_key_exists("hidex", $detailType) && $detailType["hidex"])){
    echo (implode("\n", $window_elements) . "\n");
  }

  $test_panel_elements = [];
  if (array_key_exists("title", $detailType)){
    echo(")title:value:" . $detailType["title"] . "\n");
    echo(")title:layer:basicui\n");
    echo(")title:scale:0.008 0.02 0.008\n");
    echo(")title:position:0 0 -1\n");
    $test_panel_elements = [")title"];
  }

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
    $sqlBinding = [];

    if (array_key_exists("sql", $keyvaluePairs[$i])){
      $sqlBinding = $keyvaluePairs[$i]["sql"];
    }

    includeTemplate($templateFile, $rootElementName, $i, $data, $unique_control_id, $zpos, $sqlBinding, $depth, $default_rootLayout);
    array_push($test_panel_elements, $rootElementName);

    echo ("\n");
  }

  $test_panel_elements =  array_reverse($test_panel_elements);
  echo ("(test_panel:elements:" . implode(",", $test_panel_elements) . "\n");
?>

