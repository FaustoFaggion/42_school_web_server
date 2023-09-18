
<?php

$fileToDelete = "../uploads/files/" . $_POST['file_path'];


if (is_file($fileToDelete) && unlink($fileToDelete)) {
    echo "File" . $_POST['file_path'] . "deleted successfully.";
} else {
    echo "File fail to delete.";
}

?>