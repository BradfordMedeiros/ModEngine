<?php
  $titleName = ")" . $unique_control_id . "_label";
  createElement($titleName, $default_key, [ "value" => $data["key"] ]);

  $options = $data["options"];
  $optionElements = [];
  for ($optionIndex = 0; $optionIndex < count($options); $optionIndex++){
    $optionName = $options[$optionIndex]["label"];
    $binding = $options[$optionIndex]["binding"];
    $hasBindingIndex = false;
    $hasBindingOn = false;
    if (array_key_exists("binding-index", $options[$optionIndex])){
      $hasBindingIndex = true;
    }
    if (array_key_exists("binding-on", $options[$optionIndex])){
      $hasBindingOn = true;
    }

    $optionElementName = ")" . $unique_control_id . "_" . "option_" . $optionIndex;
    $attrs = [ 
      "value" => $optionName,  
      "details-binding-toggle" => $binding,
      "details-binding-set" => "true",
    ];
    if ($hasBindingIndex){
      $attrs["details-binding-index"] = $options[$optionIndex]["binding-index"];
    }
    if ($hasBindingOn){
      $attrs["details-binding-on"] = $options[$optionIndex]["binding-on"];
    }

    createElement($optionElementName, $default_key, $attrs);
    array_push($optionElements, $optionElementName);
  }

  $optionsLayout = "(" . $unique_control_id . "_options";

  $optionsHolder = $default_keyvalueLayout;
  $optionsHolder["tint"] = "0.1 0.1 0.1 1";
  $optionsHolder["position"] = "0 0 " . $depth[28];
  $optionsHolder["align-items-horizontal"] = "center";

  $attrValues = [ "elements" => implode(",", $optionElements) ];
  $sql = NULL;
  if (array_key_exists("sql", $data)){
    $sql = $data["sql"];
  }
  includeSql($sql, $attrValues);
  createElement($optionsLayout, $optionsHolder, $attrValues);
  createElement($rootElementName, $default_rootLayout, [ 
    #"tint" => "0 0 1 1", 
    "type" => "vertical", 
    "elements" => $optionsLayout . "," . $titleName  
  ]);
?>

