<?php
  $mappingPerType = [
    "movement" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Modify movement settings for main character controller",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
        //[
        //  "type" => "numeric",
        //  "sql" => [
        //    "binding" => "sql-trait-speed",
        //    "query" => "select people.topspeed from people where people.name = john",
        //    "update" => 'update people set people.topspeed = $sql-trait-speed where people.name = john',
        //    "cast" => "number",
        //  ], 
        //  "data" => [
        //    "key" => "Physics Tuning", 
        //    "value" => [
        //      [ 
        //        "type" => "float", 
        //        "name" => "physics max speed", 
        //        "value" => [ 
        //          "binding" => "sql-trait-speed", 
        //          "type" => "number",
        //          "min" => 0,
        //          "max" => 100,
        //        ],
        //      ],
        //    ]
        //  ],
        //],
        [
          "type" => "numeric",
          "data" => [
            "key" => "Core Movement", 
            "value" => [
              [ 
                "type" => "float", 
                "name" => "Movement Speed", 
                "sql" => [
                  "binding" => "player-speed",
                  "query" => "select traits.speed from traits where profile = default",
                  "update" => 'update traits set traits.speed = $player-speed where traits.profile = default',
                  "cast" => "number",
                ], 
                "value" => [ 
                  "binding" => "player-speed", 
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "Jump Height", 
                "sql" => [
                  "binding" => "player-jump-height",
                  "query" => "select traits.jump-height from traits where profile = default",
                  "update" => 'update traits set traits.jump-height = $player-jump-height where traits.profile = default',
                  "cast" => "number",
                ], 
                "value" => [ 
                  "binding" => "player-jump-height", 
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "Gravity", 
                "sql" => [
                  "binding" => "player-gravity",
                  "query" => "select traits.gravity from traits where profile = default",
                  "update" => 'update traits set traits.gravity = $player-gravity where traits.profile = default',
                  "cast" => "number",
                ], 
                "value" => [ 
                  "binding" => "player-gravity", 
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "Friction", 
                "sql" => [
                  "binding" => "player-friction",
                  "query" => "select traits.friction from traits where profile = default",
                  "update" => 'update traits set traits.friction = $player-friction where traits.profile = default',
                  "cast" => "number",
                ], 
                "value" => [ 
                  "binding" => "player-friction", 
                  "type" => "number",
                ]
              ],
            ]
          ],
        ],
      ],
    ],
    "weapons" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Global shooting settings",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "iron sights", 
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
            "key" => "sway", 
            "value" => [
              [ 
                "type" => "float", 
                "name" => "Vertical Sway", 
                "value" => [ 
                  "binding" => "false-binding", 
                  "binding-index" => 0,
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "Horizontal Sway", 
                "value" => [ 
                  "binding" => "false-binding", 
                  "binding-index" => 0,
                  "type" => "number",
                ]
              ],
            ]
          ],
        ],
      ],
    ],
    "volumes" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Create Collision Volumes",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Create Volume",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ],
            "action" => "create-volume", # not yet implemented
            "tint" => "1 1 0 1",
          ],
        ],
        [
          "type" => "options",
          "data" => [
            "key" => "type", 
            "options" => [
              # bindings need to be corrected
              [ "label" => "box", "binding" => "gameobj:type", "binding-on" => "point" ], #
              [ "label" => "sphere", "binding" => "gameobj:type", "binding-on" => "spotlight" ],
              [ "label" => "cylinder", "binding" => "gameobj:type", "binding-on" => "directional" ],
            ],
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "Has Trigger", 
            "value" => [
              "binding" => "gameobj:dof",  
              "binding-on" => "enabled",
              "binding-off" => "disabled",
            ],
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Trigger Name",
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
      ],
    ],
    "hud" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Select Hud",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
      ],
    ],
    "water" => [
      "items" => [
        [
          "type" => "label",
          "data" => [
            "key" => "Create Water",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
      ],
    ],
  ];
?>