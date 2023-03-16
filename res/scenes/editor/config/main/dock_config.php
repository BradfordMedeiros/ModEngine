<?php
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
    "portal" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Modify Portal Settings",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Create Portal",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ],
            "action" => "create-portal",
            "tint" => "1 1 0 1",
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "perspective", 
            "value" => [
              "binding" => "gameobj:perspective",  
              "binding-on" => "true",
              "binding-off" => "false",
            ],
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Portal Camera",
            "value" => [
              "valueFromSelection" => true,
              "binding" => "gameobj:camera",
            ]
          ],
        ],
      ],
    ],
    "heightmap" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Modify Heightmap Settings",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Create Heightmap",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ],
            "action" => "create-heightmap",
            "tint" => "1 1 0 1",
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Path to Heightmap",
            "value" => [
              "binding" => "gameobj:map",
              "valueFromDialog" => "load-heightmap",
            ]
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Save Heightmap",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ],
            "action" => "save-heightmap",
            "tint" => "1 1 0 1",
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Save Heightmap As",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ],
            "action" => "save-heightmap-as",
            "tint" => "1 1 0 1",
          ],
        ],
        [ "type" => "label", 
          "data" => [
            "key" => "Heightmap Name", 
            "value" => [
              "binding" => "heightmap:filename",
            ],
          ]
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "sizing", 
            "value" => [
              [ 
                "type" => "float", 
                "name" => "dim", 
                "value" => [ 
                  "binding" => "gameobj:dim",
                  "type" => "number",
                ]
              ],
            ]
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Heightmap Brush Tool",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
        //[
        //  "type" => "list",
        //  "data" => [
        //    "key" => "",
        //    "values" => [
        //      [ 
        //        "image" => "./res/scenes/editor/dock/images/play.png", 
        //        "action" => "toggle-play-mode",
        //        "binding" => "play-mode-on", 
        //        "binding-on" => "on",
        //        "binding-off" => "off",
        //      ],
        //    ],
        //  ],
        //],
        [
          "type" => "label",
          "data" => [
            "key" => "Type",
            "value" => [
              "binding" => "world:tools:terrainpaint-brush",
              "valueFromDialog" => "load-heightmap-brush",
            ]
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "Smoothing", 
            "value" => [
              "binding" => "world:tools:terrainpaint-smoothing",  
              "binding-on" => "true",
              "binding-off" => "false",
            ],
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "brush parameters", 
            "value" => [
              [ 
                "type" => "float", 
                "name" => "Radius", 
                "value" => [ 
                  "binding" => "world:tools:terrainpaint-radius",
                  "type" => "positive-number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "Amount", 
                "value" => [ 
                  "binding" => "world:tools:terrainpaint-amount",
                  "type" => "number",
                ]
              ],
            ]
          ],
        ],
      ],
    ],
    "voxel" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Modify Voxel Settings",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Create Voxel",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ],
            "action" => "create-voxels",
            "tint" => "1 1 0 1",
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "area", 
            "value" => [
              [ 
                "type" => "float", 
                "name" => "width", 
                "value" => [ 
                  "binding" => "gameobj:dim",
                  "binding-index" =>  0,
                  "type" => "positive-number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "height", 
                "value" => [ 
                  "binding" => "gameobj:dim",
                  "binding-index" =>  1,
                  "type" => "positive-number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "depth", 
                "value" => [ 
                  "binding" => "gameobj:dim",
                  "binding-index" =>  1,
                  "type" => "positive-number",
                ]
              ],
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
          "type" => "label",
          "data" => [
            "key" => "Create Sound",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ],
            "action" => "create-sound",
            "tint" => "1 1 0 1",
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
            "value" => [
              "binding" => "gameobj:clip",
              "valueFromDialog" => "load-sound",
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
    "geo" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Modify Geo Settings",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Create Geo",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ],
            "action" => "create-geo",
            "tint" => "1 1 0 1",
          ],
        ],
        [
          "type" => "options",
          "data" => [
            "key" => "shape", 
            "options" => [
                [ "label" => "default", "binding" => "gameobj:shape", "binding-on" => "default" ],
                [ "label" => "sphere", "binding" => "gameobj:shape", "binding-on" => "sphere" ],
            ],
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "points",
            "value" => [
              "binding" => "gameobj:points",
              "type" => "list",
            ]
          ],
        ],
      ],
    ],
    "object_details" => [
      "title" => "Object Details",
      "items" => [
        [ "type" => "label", 
          "data" => [
            "key" => "Current Object", 
            "value" => [
              "binding" => "object_name",
              "type" => "number",
            ],
          ]
        ],
        [ "type" => "label", 
          "data" => [
            "key" => "Look at Target", 
            "value" => [
              "valueFromSelection" => true,
              "binding" => "gameobj:lookat",
            ],
          ]
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "enable physics", 
            "value" => [
              "binding" => "gameobj:physics",
              "binding-on" => "enabled",
              "binding-off" => "disabled",
            ],
          ],
        ],
        [
          "type" => "options",
          "data" => [
            "key" => "physics shape", 
            "options" => [
                [ "label" => "auto", "binding" => "gameobj:physics_shape", "binding-on" => "shape_auto" ],
                [ "label" => "sphere", "binding" => "gameobj:physics_shape", "binding-on" => "shape_sphere" ],
                [ "label" => "capsule", "binding" => "gameobj:physics_shape", "binding-on" => "shape_capsule" ],
                [ "label" => "cylinder", "binding" => "gameobj:physics_shape", "binding-on" => "shape_cylinder" ],
                [ "label" => "hull", "binding" => "gameobj:physics_shape", "binding-on" => "shape_hull" ],
                [ "label" => "exact", "binding" => "gameobj:physics_shape", "binding-on" => "shape_exact" ],
            ],
          ],
        ],
        [
          "type" => "options",
          "data" => [
            "key" => "physics_type", 
            "options" => [
                [ "label" => "static", "binding" => "gameobj:physics_type", "binding-on" => "static" ],
                [ "label" => "dynamic", "binding" => "gameobj:physics_type", "binding-on" => "dynamic" ],
            ],
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "Physics Tuning", 
            "value" => [
              [ 
                "type" => "slider", 
                "name" => "gravity", 
                "value" => [ 
                  "binding" => "gameobj:physics_gravity", 
                  "binding-index" =>  1,
                  "min" => -10,
                  "max" => 10,
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "physics max speed", 
                "value" => [ 
                  "binding" => "gameobj:physics_maxspeed", 
                  "type" => "number",
                  "min" => 0,
                  "max" => 10,
                ],
              ],
            ]
          ],
        ],
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
        [ "type" => "label", 
          "data" => [
            "key" => "Render Layer", 
            "value" => [
              "binding" => "gameobj:layer",
            ],
          ]
        ],
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
            "values" => [
              ["image" => "gentexture-scenegraph", "size" => "0.42 1 0.16" ],
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
            "key" => "enable culling", 
            "value" => [
              "binding" => "world:rendering:cull",  
              "binding-on" => "enabled",
              "binding-off" => "disabled",
            ],
          ],
        ],
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
        [
          "type" => "checkbox",
          "data" => [
            "key" => "enable shadows", 
            "value" => [
              "binding" => "world:shadows:enabled",  
              "binding-on" => "true",
              "binding-off" => "false",
            ],
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "shadow amount", 
            "value" => [
              [ 
                "type" => "slider", 
                "name" => "shadow", 
                "value" => [ 
                  "binding" => "world:shadows:intensity", 
                  "min" => 0,  // what should min and max really be?d
                  "max" => 1,
                ]
              ],
            ]
          ],
        ],
      ],
    ],
    "models" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Models",
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
            "values" => [
              ["image" => "gentexture-models", "size" => "0.42 1 0.16" ],
            ],
            "type" => "vertical",
          ],
        ],
      ],
    ],
    "textures" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Textures",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "texture options", 
            "value" => [
              [ 
                "type" => "float", 
                "name" => "tiling x", 
                "value" => [ 
                  "binding" => "gameobj:texturetiling", 
                  "binding-index" => 0,
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "tiling y", 
                "value" => [ 
                  "binding" => "gameobj:texturetiling", 
                  "binding-index" => 1,
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "size x", 
                "value" => [ 
                  "binding" => "gameobj:texturesize", 
                  "binding-index" => 0,
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "size y", 
                "value" => [ 
                  "binding" => "gameobj:texturesize", 
                  "binding-index" => 1,
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "offset x", 
                "value" => [ 
                  "binding" => "gameobj:textureoffset", 
                  "binding-index" => 0,
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "offset y", 
                "value" => [ 
                  "binding" => "gameobj:textureoffset", 
                  "binding-index" => 1,
                  "type" => "number",
                ]
              ],
            ]
          ],
        ],
        [
          "type" => "list",
          "data" => [
            "key" => "",
            "values" => [
              ["image" => "gentexture-textures", "size" => "0.42 1 0.16" ],
            ],
            "type" => "vertical",
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
    "transform_types" => [
      "items" => [ 
        [
          "type" => "label",
          "data" => [
            "key" => "Editor Defaults",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "Group Selection", 
            "value" => [
              "binding" => "world:editor:groupselection",  
              "binding-on" => "true",
              "binding-off" => "false",
            ],
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "Symmetric Translate", 
            "value" => [
              "binding" => "world:tools:position-mirror",  
              "binding-on" => "true",
              "binding-off" => "false",
            ],
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "Absolute Translate", 
            "value" => [
              "binding" => "world:tools:snap-position",  
              "binding-on" => "absolute",
              "binding-off" => "relative",
            ],
          ],
        ],
        [
          "type" => "options",
          "data" => [
            "key" => "Snap Translate", 
            "options" => [
              [ "label" => "0.01", "binding" => "world:editor:snaptranslate-index", "binding-on" => "0" ],
              [ "label" => "0.1", "binding" => "world:editor:snaptranslate-index", "binding-on" => "1" ],
              [ "label" => "0.5", "binding" => "world:editor:snaptranslate-index", "binding-on" => "2" ],
              [ "label" => "1", "binding" => "world:editor:snaptranslate-index", "binding-on" => "3" ],
              [ "label" => "5", "binding" => "world:editor:snaptranslate-index", "binding-on" => "4" ],
            ],
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "position", 
            "value" => [
              [ 
                "type" => "float", 
                "name" => "x", 
                "value" => [ 
                  "binding" => "gameobj:position", 
                  "binding-index" =>  0,
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "y", 
                "value" => [ 
                  "binding" => "gameobj:position", 
                  "binding-index" =>  1,
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "z", 
                "value" => [ 
                  "binding" => "gameobj:position", 
                  "binding-index" =>  2,
                  "type" => "number",
                ]
              ]
            ]
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "Preserve Scale", 
            "value" => [
              "binding" => "world:tools:preserve-scale",  
              "binding-on" => "true",
              "binding-off" => "false",
            ],
          ],
        ],
        [
          "type" => "options",
          "data" => [
            "key" => "Snap Scales", 
            "options" => [
              [ "label" => "0.01", "binding" => "world:editor:snapscale-index", "binding-on" => "0" ],
              [ "label" => "0.1", "binding" => "world:editor:snapscale-index", "binding-on" => "1" ],
              [ "label" => "0.5", "binding" => "world:editor:snapscale-index", "binding-on" => "2" ],
              [ "label" => "1", "binding" => "world:editor:snapscale-index", "binding-on" => "3" ],
              [ "label" => "5", "binding" => "world:editor:snapscale-index", "binding-on" => "4" ],
            ],
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "scale", 
            "value" => [
              [ 
                "type" => "float", 
                "name" => "x", 
                "value" => [ 
                  "binding" => "gameobj:scale", 
                  "binding-index" =>  0,
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "y", 
                "value" => [ 
                  "binding" => "gameobj:scale", 
                  "binding-index" =>  1,
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "z", 
                "value" => [ 
                  "binding" => "gameobj:scale", 
                  "binding-index" =>  2,
                  "type" => "number",
                ]
              ]
            ]
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "Absolute Rotation", 
            "value" => [
              "binding" => "world:tools:snap-rotate",  
              "binding-on" => "absolute",
              "binding-off" => "relative",
            ],
          ],
        ],
        [
          "type" => "options",
          "data" => [
            "key" => "Snap Rotation", 
            "options" => [
              [ "label" => "1", "binding" => "world:editor:snapangle-index", "binding-on" => "0" ],
              [ "label" => "5", "binding" => "world:editor:snapangle-index", "binding-on" => "1" ],
              [ "label" => "15", "binding" => "world:editor:snapangle-index", "binding-on" => "2" ],
              [ "label" => "30", "binding" => "world:editor:snapangle-index", "binding-on" => "3" ],
              [ "label" => "45", "binding" => "world:editor:snapangle-index", "binding-on" => "4" ],
              [ "label" => "90", "binding" => "world:editor:snapangle-index", "binding-on" => "5" ],
              [ "label" => "180", "binding" => "world:editor:snapangle-index", "binding-on" => "6" ],
            ],
          ],
        ],
      ],
    ],
    "debug" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Modify Debug Masks",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "Show Debug", 
            "action" => "submit-debug-vis",
            "value" => [
              "binding" => "world:editor:debug",  
              "binding-on" => "true",
              "binding-off" => "false",
            ],
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "Cameras", 
            "action" => "submit-debug-vis",
            "value" => [
              "binding" => "debug-show-cameras",  
              "binding-on" => "enabled",
              "binding-off" => "disabled",
            ],
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "Lights", 
            "action" => "submit-debug-vis",
            "value" => [
              "binding" => "debug-show-lights",  
              "binding-on" => "enabled",
              "binding-off" => "disabled",
            ],
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "Sound", 
            "action" => "submit-debug-vis",
            "value" => [
              "binding" => "debug-show-sound",  
              "binding-on" => "enabled",
              "binding-off" => "disabled",
            ],
          ],
        ],
      ],
    ],
  ];
?>

