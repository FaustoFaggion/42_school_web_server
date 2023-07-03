<?php

var_dump($_FILES);

echo " run test4.php\n";

// Read input from STDIN
$input = file_get_contents('php://stdin');

// Process the input
// ...

// Output the response
echo "Processed input: " . $input;

?>