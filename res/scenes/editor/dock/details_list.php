<?php
  /*
  eg
    )key_2_checkbox_label:layer:basicui
    )key_2_checkbox_label:scale:0.004 0.01 0.004
    )key_2_checkbox_label:value:checkbox name
    *key_2_checkbox_check:layer:basicui
    *key_2_checkbox_check:scale:0.02 -0.05 0.02
    *key_2_checkbox_check:cantoggle:true
    *key_2_checkbox_check:ontexture:./res/scenes/editor/dock/checked.png
    *key_2_checkbox_check:offtexture:./res/scenes/editor/dock/unchecked.png
    (key_2:layer:basicui
    (key_2:type:horizontalfas
    (key_2:backpanel:true
    (key_2:tint:0.05 0.05 0.05 1
    (key_2:margin:0.02
    (key_2:spacing:0.02
    (key_2:minwidth:0.36
    (key_2:elements:)key_2_checkbox_label,*key_2_checkbox_check
  */

  $labelName = ")" . $unique_control_id . "_" . "list_label";
  createElement($labelName, $default_key, [ 
    "value" => $data["key"],
    "tint" => "5 5 5 1",
  ]);

  $values = $data["values"];
  $listElementNames = [];

  $listHolder = "(" . $unique_control_id . "_" . "list_holder";
  for ($i = 0; $i < count($values); $i++){
    $value = $values[$i];
    $listItemName = "*" . $unique_control_id . "_" . "list_item_" . $i;

    $scale = "0.04 0.1 0.04";
    if (array_key_exists("size", $value)){
      $scale = $value["size"];
    }

    $extraVals = [
      "scale" => $scale,  # negative since some bug with button textures, should fix
      "ontexture" => $value["image"],
      "offtexture" => $value["image"],
      "tint" => "1 1 1 1",
      "details-action" => $value["action"],
      "position" => "0 0 " . $depth[3],
      "layer" => "basicui",
    ];
    if (array_key_exists("binding", $value)){
      $extraVals["details-binding-toggle"] = $value["binding"];
    }
    if (array_key_exists("binding-on", $value)){
      $extraVals["details-binding-on"] = $value["binding-on"];
    }
    if (array_key_exists("binding-off", $value)){
      $extraVals["details-binding-off"] = $value["binding-off"];
    }

    createElement($listItemName, $default_text_style, $extraVals);

    array_push($listElementNames, $listItemName);
  }

  $listHolderVals = [
    "tint" => "0.5 0.5 0.5 1",
    "elements" => implode(",", $listElementNames),
    #"backpanel" => "true",
    "position" => "0 0 " . $depth[2],
    "align-content" => "center",
    "align-items-horizontal" => "left",
    "minwidth" => "0.42",
  ];
  if (array_key_exists("type", $data) && $data["type"] == "vertical"){
    $listHolderVals["type"] = "vertical";
  }

  createElement($listHolder, $default_style, $listHolderVals);


  $topLevelElements = [$listHolder, $labelName];

  $topLevelAttr = [ 
    "tint" => "0.1 0.1 0.1 1", 
    "elements" => implode(",", $topLevelElements),
    "type" => "vertical",
    "position" => "0 0 " . $depth[1],
    #"spacing" => "0.04",
    "align-content" => "neg"   
  ];

  if (array_key_exists("mode", $data)){
    $topLevelAttr["details-list-mode"] = $data["mode"];
  }
  createElement($rootElementName, $default_rootLayout, $topLevelAttr);
?>