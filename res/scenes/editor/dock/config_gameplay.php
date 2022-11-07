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
        [
          "type" => "numeric",
          "data" => [
            "key" => "Core Movement", 
            "value" => [
              [ 
                "type" => "float", 
                "name" => "Movement Speed", 
                "value" => [ 
                  "binding" => "false-binding", 
                  "binding-index" => 0,
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "Jump Height", 
                "value" => [ 
                  "binding" => "false-binding", 
                  "binding-index" => 0,
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "Gravity", 
                "value" => [ 
                  "binding" => "false-binding", 
                  "binding-index" => 0,
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "Friction", 
                "value" => [ 
                  "binding" => "false-binding", 
                  "binding-index" => 0,
                  "type" => "number",
                ]
              ],
            ]
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "Animation", 
            "value" => [
              [ 
                "type" => "float", 
                "name" => "Vertical Bob", 
                "value" => [ 
                  "binding" => "false-binding", 
                  "binding-index" => 0,
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "Horizontal Lean", 
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
  ];
?>