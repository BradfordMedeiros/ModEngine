<?php
  $mappingPerType = [
    "movement" => [
      "items" => [
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
                "name" => "Movement Speed Air", 
                "sql" => [
                  "binding" => "player-speed-air",
                  "query" => "select traits.speed-air from traits where profile = default",
                  "update" => 'update traits set traits.speed-air = $player-speed-air where traits.profile = default',
                  "cast" => "number",
                ], 
                "value" => [ 
                  "binding" => "player-speed-air", 
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "Movement Speed Water", 
                "sql" => [
                  "binding" => "player-speed-water",
                  "query" => "select traits.speed-water from traits where profile = default",
                  "update" => 'update traits set traits.speed-water = $player-speed-water where traits.profile = default',
                  "cast" => "number",
                ], 
                "value" => [ 
                  "binding" => "player-speed-water", 
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
              [ 
                "type" => "float", 
                "name" => "Restitution", 
                "sql" => [
                  "binding" => "player-restitution",
                  "query" => "select traits.restitution from traits where profile = default",
                  "update" => 'update traits set traits.restitution = $player-restitution where traits.profile = default',
                  "cast" => "number",
                ], 
                "value" => [ 
                  "binding" => "player-restitution", 
                  "type" => "number",
                ]
              ],
            ]
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "crouch",
            "sql" => [
              "binding" => "player-crouch",
              "query" => "select traits.crouch from traits where profile = default",
              "update" => 'update traits set traits.crouch = $player-crouch where traits.profile = default',
              "cast" => "string",
            ], 
            "value" => [
              "binding" => "player-crouch",  
              "binding-on" => "TRUE",
              "binding-off" => "FALSE",
            ],
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
            "action" => "create-volume", 
            "tint" => "1 1 0 1",
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Trigger Name",
            "value" => [
              "binding" => "gameobj:trigger-switch",  
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
        [
          "type" => "label",
          "data" => [
            "key" => "Reload Hud",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ],
            "action" => "reload-hud", 
            "tint" => "0 1 1 1",
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Hud Name",
            "sql" => [
              "binding" => "player-hud",
              "query" => "select traits.profile from traits where profile = default",
              "update" => 'update traits set traits.speed-water = $profile where traits.profile = default',
              "cast" => "string",
            ], 
            "value" => [
              "binding" => "player-hud",
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
            "key" => "Water",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ]
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Create Water",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ],
            "action" => "create-water", 
            "tint" => "0 0 1 1",
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "Water Properties", 
            "value" => [
              [ 
                "type" => "float", 
                "name" => "Density", 
                "value" => [ 
                  "binding" => "gameobj:water-density", 
                  "type" => "positive-number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "Viscosity", 
                "value" => [ 
                  "binding" => "gameobj:water-gravity", 
                  "type" => "positive-number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "Gravity", 
                "value" => [ 
                  "binding" => "gameobj:water-viscosity", 
                  "type" => "positive-number",
                ]
              ],
            ]
          ],
        ],

      ],
    ],
  ];
?>