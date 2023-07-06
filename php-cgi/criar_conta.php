<?php

// Determine the size of each chunk (e.g., 8192 bytes)
$chunkSize = $_SERVER['BUFFER_SIZE'];

// Create a unique file name
$filename = uniqid();

// Open a file for writing
$file = fopen("../locations/uploads/{$filename}", "wb");
if ($file) {
    // Read the uploaded file in chunks from the input stream
    $inputStream = fopen('php://input', 'rb');
    while (!feof($inputStream)) {
        $chunk = fread($inputStream, $chunkSize);
        fwrite($file, $chunk);
    }
    fclose($inputStream);
    fclose($file);

    // File saved successfully
    echo "File uploaded successfully.";
} else {
    // Failed to open the file
    echo "Error opening file for writing.";
}

echo "<br>";
echo "<hr>";

echo "<br>";
echo "_SERVER[]" . "<br>";
foreach ($_SERVER as $key => $value) {
    echo $key . " = " . $value . "<br>";

}

echo "<br>";
echo "_POST[]" . "<br>";
foreach ($_POST as $key => $value) {
    echo $key . " = " . $value . "<br>";
}

echo "<br>";
echo "_GET[]" . "<br>";
foreach ($_GET as $key => $value) {
    echo $key . " = " . $value . "<br>";
}
?>