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
    

  ];
?>