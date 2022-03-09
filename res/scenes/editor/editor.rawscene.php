
#$header:include:./res/scenes/editor/header.rawscene  # like copy and pasting the values of the header, basically a macro
#$header:import:./res/scenes/editor/header.rawscene   # imports elements as children under header?

<?php 

  $attributes = [
    "layer" => "basicui",
    "scale" => "0.02 0.02 0.02",
  ];

  $row1_elements = [];
  $number_of_buttons = 5;
  for ($i = 0; $i < $number_of_buttons; $i++){
    $button_name = "*basicbutton" . $i;
    array_push($row1_elements, $button_name);
    foreach ($attributes as $key => $value){
      echo ($button_name . ":" . $key . ":" . $value . "\n");
    }
    echo("\n");
  }

  echo ("(row1:elements:" . implode(",", $row1_elements));


?>

(row1:layer:basicui
(row1:position:0 0 -1
(row1:type:horizontal
(row1:backpanel:true
(row1:tint:1.0 0.1 0.1
(row1:margin:0.02
(row1:spacing:0.05
