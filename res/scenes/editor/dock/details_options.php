<?php
  $titleName = ")" . $unique_control_id . "_label";
  createElement($titleName, $default_key, [ "value" => "light-type" ]);

  $options = $data["options"];
  $optionElements = [];
  for ($optionIndex = 0; $optionIndex < count($options); $optionIndex++){
    $optionName = $options[$optionIndex];
    $isSelected = $data["selected"] == $optionIndex;
    $optionElementName = ")" . $unique_control_id . "_" . "option_" . $optionIndex;
    $attrs = [ "value" => $optionName,  "details-group" => $rootElementName, "details-group-index" => $optionIndex  ];
    if ($isSelected){
      $attrs["tint"] = "0 0 4 1";
    }
    createElement($optionElementName, $default_key, $attrs);
    array_push($optionElements, $optionElementName);
  }

  $optionsLayout = "(" . $unique_control_id . "_options";
  createElement($optionsLayout, $default_keyvalueLayout, [ "elements" => implode(",", $optionElements) ]);
  createElement($rootElementName, $default_keyvalueLayout, [ 
    "tint" => "0 0 1 0.8", 
    "type" => "vertical", 
    "elements" => $optionsLayout . "," . $titleName  
  ]);
?>