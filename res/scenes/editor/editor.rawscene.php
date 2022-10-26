
<?php /*include "../example.p.rawscene"; */ ?>

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
    "scale" => "0.004 0.01 0.004",
  ];
  $menu_elements = [];
  $menu_bar= [
    ["value" => "file", "popoption" => "file"],
    ["value" => "misc", "popoption" => "misc"],
  ];
  for ($i = 0; $i < count($menu_bar); $i++){
    $textfield_name = ")menuitem_" . $i;
    createElement($textfield_name, $menu_text_attr, 
      [
        "value" => $menu_bar[$i]["value"], 
        "popoption" => $menu_bar[$i]["popoption"]
      ]
    );
    array_push($menu_elements, $textfield_name);
  }
  echo ("(menubar:elements:" . implode(",", $menu_elements) . "\n");

  ### buttons with text next to them
  $row2_elements = [];
  $text_buttons = [
    ["value" => "hide", "dialogoption" => "HIDE"],
    //["value" => "test", "dialogoption" => "./res/scenes/editor/dock/testpanel.rawscene"],
    ["value" => "OBJECT DETAILS", "dialogoption" => "./res/scenes/editor/dock/object_details.rawscene"],
    ["value" => "SCENE INFO", "dialogoption" => "./res/scenes/editor/dock/world_state.rawscene"],
    ["value" => "SCENEGRAPH", "dialogoption" => "./res/scenes/editor/dock/scenegraph.rawscene"],
    ["value" => "PERFORMANCE", "dialogoption" => "./res/scenes/editor/dock/performance.rawscene"],
    ["value" => "CAMERAS", "dialogoption" => "./res/scenes/editor/dock/cameras.rawscene"],
    ["value" => "LIGHTS", "dialogoption" => "./res/scenes/editor/dock/lights.rawscene"],
    ["value" => "SOUND", "dialogoption" => "./res/scenes/editor/dock/sound.rawscene"],
    ["value" => "TEXT", "dialogoption" => "./res/scenes/editor/dock/text.rawscene"],
  ];

  $text_attr = [
    "layer" => "basicui",
    "scale" => "0.004 0.01 0.004",
  ];

  for ($i = 0; $i < count($text_buttons); $i++){
    $textfield_name = ")text_" . $i;
    createElement($textfield_name, $text_attr, $text_buttons[$i]);
    array_push($row2_elements, $textfield_name);
  }
  echo("\n");
  echo ("(row2:elements:" . implode(",", $row2_elements) . "\n");

?>

(menubar:layer:basicui
(menubar:position:-1 1 0
(menubar:type:horizontal
(menubar:backpanel:true
(menubar:tint:0.2 0.2 0.2 1
(menubar:margin:0.02
(menubar:spacing:0.02
(menubar:align-vertical:down
(menubar:align-horizontal:right
(menubar:margin-bottom:0.02
(menubar:minwidth:2
(menubar:border-color:1 0.5 0.5
(menubar:script:./res/scenes/editor/editor.scm


(row2:layer:basicui
(row2:position:0 0 0
(row2:type:horizontal
(row2:backpanel:true
(row2:tint:0.2 0.2 0.2 1
(row2:margin:0.02
(row2:spacing:0.05
(row2:align-vertical:down
(row2:align-horizontal:right
(row2:anchor:(menubar
(row2:anchor-offset:0 -0.04 0
(row2:anchor-direction:down
(row2:minwidth:2

(row3:layer:basicui
(row3:position:0 0 0
(row3:type:horizontal
(row3:backpanel:true
(row3:tint:1 1 1 0.8
(row3:theme-play-tint:0 0 1 0.8
(row3:theme-restore-tint:1 1 1 0.8
(row3:margin:0.002
(row3:anchor:(menubar
(row3:anchor-offset:1 -0.099 0
(row3:anchor-direction:down
(row3:minwidth:0.5
