<?php 
    header('Content-type: Application/JSON');

    echo file_get_contents("./config.json");
?>
