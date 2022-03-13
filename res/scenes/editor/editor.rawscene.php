
#$header:include:./res/scenes/editor/header.rawscene  # like copy and pasting the values of the header, basically a macro
#$header:import:./res/scenes/editor/header.rawscene   # imports elements as children under header?

<?php include "../example.p.rawscene"; ?>

<?php 
  /* include "./dock/testpanel.php"; */ 

?>



<?php 

  function createElement($name, $attr, $more_attr){
    foreach ($attr as $key => $value){
      echo ($name . ":" . $key . ":" . $value . "\n");
    }
    foreach ($more_attr as $key => $value){
      echo ($name . ":" . $key . ":" . $value . "\n");
    }
  }

  #### Navigation controls 

  $menu_text_attr = [
    "layer" => "basicui",
    "scale" => "0.01 0.01 0.01",
  ];
  $menu_elements = [];
  $menu_bar= [
    ["value" => "file", "items" => ["runoptions", "quit"]],
    ["value" => "misc", "items" => ["fullscreen (alt + enter)", "info"]],
  ];
  for ($i = 0; $i < count($menu_bar); $i++){
    $textfield_name = ")menuitem_" . $i;
    createElement($textfield_name, $menu_text_attr, ["value" => $menu_bar[$i]["value"]]);
    array_push($menu_elements, $textfield_name);
  }
  echo ("(menubar:elements:" . implode(",", $menu_elements) . "\n");

  ### buttons with text next to them
  $row2_elements = [];
  $text_buttons = [
    ["value" => "HIDE"],
    ["value" => "SCENE INFO"],
    ["value" => "SCENEGRAPH"],
    ["value" => "OBJECT DETAILS"],
    ["value" => "WORLD STATE"],
    ["value" => "MESHES"],
    ["value" => "LIGHTS"],
    ["value" => "CAMERAS"],
    ["value" => "HEIGHTMAPS"],
    ["value" => "VOXEL"],
  ];

  $text_attr = [
    "layer" => "basicui",
    "scale" => "0.01 0.01 0.01",
  ];
  $button_attr = [
    "layer" => "basicui",
    "scale" => "0.02 0.02 0.02",
  ];

  for ($i = 0; $i < count($text_buttons); $i++){
    $button_name = "*buttonname_" . $i;
    createElement($button_name, $button_attr, []);
    array_push($row2_elements, $button_name);

    $textfield_name = ")text_" . $i;
    createElement($textfield_name, $text_attr, $text_buttons[$i]);
    array_push($row2_elements, $textfield_name);
  }
  echo("\n");
  echo ("(row2:elements:" . implode(",", $row2_elements) . "\n");

?>

(menubar:layer:basicui
(menubar:position:-0.9 0.97 -1  # 0.42, 0.97 is kind of arbitary, needs to be aligned from the left/top.  The actual width is measured and a function of text size and button size, really want layout alignment stuff
(menubar:type:horizontal
(menubar:backpanel:true
(menubar:tint:1.0 0.1 0.1
(menubar:margin:0.02
(menubar:spacing:0.02


(row2:layer:basicui
(row2:position:0.42 0.91 -1 
(row2:type:horizontal
(row2:backpanel:true
(row2:tint:1.0 0.1 0.1
(row2:margin:0.02
(row2:spacing:0.05
