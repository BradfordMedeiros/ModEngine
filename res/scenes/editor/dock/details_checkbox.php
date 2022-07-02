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

  $labelName = ")" . $unique_control_id . "_" . "checkbox_label";
  createElement($labelName, $default_key, [ 
    "value" => $data["key"],
    #"tint" => "5 5 5 1",
  ]);

  $checkboxName = "*" . $unique_control_id . "_" . "checkbox_check";
  $buttonFields = [
    "cantoggle" => "true",
    "scale" => "0.02 -0.05 0.02",  # negative since some bug with button textures, should fix
    "ontexture" => "./res/scenes/editor/dock/images/checked.png",
    "offtexture" => "./res/scenes/editor/dock/images/unchecked.png",
    "ontint" => "5 5 5 1",
    "on" => "editor-button-on",
    "off" => "editor-button-off",
  ];
  if (is_bool($data["value"])){
    $buttonFields["state"] = $data["value"] ? "on" : "off";
  }else{
    $buttonFields["details-binding-toggle"] = $data["value"]["binding"];
    $buttonFields["details-binding-on"] = $data["value"]["binding-on"];
    $buttonFields["details-binding-off"] = $data["value"]["binding-off"];
  }
 
  createElement($checkboxName, $default_text_style, $buttonFields);
  createElement($rootElementName, $default_rootLayout, [ "spacing" => "0.25", /*"tint" => "0 0 1 1",*/ "elements" =>  $labelName . "," . $checkboxName ]);
?>