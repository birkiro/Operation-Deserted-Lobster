<?php
if (isset($_REQUEST["action"])) {
  switch ($_REQUEST["action"]) {
    case "runApp":
      exec("./../../home/root/controller_arm"); //linux command line..
      break;

    case "getData":
      require_once 'class.file.php';
      $file = "../../var/www/logfile";
      $fileMPP = "../../var/www/MPPfile";
      if (File::exsist($file) && File::exsist($fileMPP)) {
        $result = array();
	$resultMPP = array();
        $array = explode("\n", File::read($file));
        foreach ($array as $a) {
          $tmp = explode(" - ", $a);
          foreach ($tmp as $b) {
            $tmp2 = explode(":", $b);
            $r[$tmp2[0]] = $tmp2[1];
          }
          $result[] = $r;
        }
	$arrayMPP = explode("\n", File::read($fileMPP));
        foreach ($arrayMPP as $a) {
          $tmp = explode(" - ", $a);
          foreach ($tmp as $b) {
            $tmp2 = explode(":", $b);
            $r[$tmp2[0]] = $tmp2[1];
          }
          $resultMPP[] = $r;
        }
        echo json_encode(array("success" => true, "data" => $result, "dataMPP" => $resultMPP));
      } else {
        echo json_encode(array("success" => false));
      }
      break;

  }
}
?>
