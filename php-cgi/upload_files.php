<?php

// Determine the size of each chunk (e.g., 8192 bytes)
$chunkSize = $_SERVER['BUFFER_SIZE'];

// // Create a unique file name
// $filename = uniqid();
// $size = $_SERVER['CONTENT_LEGTH'];

// echo 'size: ' . $size;

// // Open a file for writing
// $file = fopen("../locations/uploads/{$filename}", "wb");
// if ($file) {
//     // Read the uploaded file in chunks from the input stream
//     $inputStream = fopen('php://input', 'rb');
//     while (!feof($inputStream)) {
//         $chunk = fread($inputStream, $chunkSize);
//         fwrite($file, $chunk);
//     }
//     fclose($inputStream);
//     fclose($file);

//     // File saved successfully
//     echo "File uploaded successfully.";
// } else {
//     // Failed to open the file
//     echo "Error opening file for writing.";
// }

echo file_get_contents('php://input');
echo "<br>";
echo "<hr>";

var_dump($_FILES);
echo "<br>";
echo "<hr>";

var_dump($_POST);
echo "<br>";
echo "<hr>";

// Check if a file was uploaded
if (isset($_FILES['userfile']))
{
    // Specify the target directory to save the file
    $targetDirectory = '../locations/uploads';

    // Get the file name and temporary file path
    $fileName = $_FILES['userfile']['name'];
	echo $fileName . "\n";
    // $tempFilePath = "./_TMP_FILE";
	$tempFilePath = $_FILES['userfile']['tmp_name'];
	echo $tempFilePath . "\n";
    // Generate the target file path
    $targetFilePath = $targetDirectory . "/" . $fileName;
	echo $targetFilePath . "\n";
    // Move the uploaded file to the target directory
    if (move_uploaded_file($tempFilePath, $targetFilePath))
    {
        echo "<br>File saved successfully.";
    }
    else
    {
        echo "\n\nError saving the file.";
    }
}
else
{
    echo '\n\nNo file uploaded.';
}
?>
