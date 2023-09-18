<?php

// Get the POST data from the request
$postData = file_get_contents("php://input");

// Print the received POST data
echo $postData;

?>