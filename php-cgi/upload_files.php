<?php

var_dump($_FILES);

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
        echo '\n\nFile saved successfully.';
    }
    else
    {
        echo '\n\nError saving the file.';
    }
}
else
{
    echo '\n\nNo file uploaded.';
}

?>