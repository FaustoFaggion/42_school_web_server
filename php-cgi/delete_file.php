
<?php

$fileToDelete = "../locations/uploads/" . $_POST['file_path'];

echo $fileToDelete . "<hr>";

if (is_file($fileToDelete) && unlink($fileToDelete)) {
    echo "File" . $_Get['file_path'] . "deleted successfully.";
} else {
    echo "Failed to delete the file.";
}

?>