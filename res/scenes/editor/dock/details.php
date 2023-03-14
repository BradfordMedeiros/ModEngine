
(test_panel:layer:basicui
(test_panel:type:vertical
#(test_panel:margin:0.02
#(test_panel:spacing:0.02
(test_panel:align-content:neg


<?php 
  function createElement($name, $attr, $more_attr){
    $combined = array_merge($attr, $more_attr);
    foreach ($combined as $key => $value){
      echo ($name . ":" . $key . ":" . $value . "\n");
    }
  }

  function includeTemplate($file, $rootElementName, $i, $data,  $unique_control_id, $zpos, $depth, $default_rootLayout ){
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

    include $file;
  }


  function execute($detailType){
    $zpos = -1;  # + or +- 0.2 for certain order of elements within
    echo ("(test_panel:position:-0.78 -0.097 " . $zpos . "\n");

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
        includeTemplate($templateFile, $rootElementName, $i, $data, $unique_control_id, $zpos, $depth, $default_rootLayout);
        array_push($test_panel_elements, $rootElementName);
  
        echo ("\n");
      }
  
    $test_panel_elements =  array_reverse($test_panel_elements);
    echo ("(test_panel:elements:" . implode(",", $test_panel_elements) . "\n");
  }


  execute($detailType);


?>

