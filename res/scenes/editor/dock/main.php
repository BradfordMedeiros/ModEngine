<?php

  if (count ($argv) < 2){
    print("Must define which detail to build\n");
    exit(1);
  }

  $configFile = $argv[1];
  if ($configFile == "main"){
    include 'config.php';
  }else if ($configFile == "gameplay"){
    include 'config_gameplay.php';
  }else{
    echo ("invalid config: " . $configFile . "\n");
    exit(1);
  }
  #
  
  $target_type = $argv[2];
  $detailType = $mappingPerType[$target_type];
	ob_start();
	include 'details.php';
	$xhtml = ob_get_clean();
	echo ($xhtml);
?>

