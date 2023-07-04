
<?php

$directory = "../locations/uploads"; // Replace with the actual path to your directory

if (is_dir($directory)) {
    if ($handle = opendir($directory)) {
        echo "<ul>";
        while (($file = readdir($handle)) !== false) {
            if ($file != "." && $file != "..") {
                echo "<li><a href='$directory/$file'>$file</a></li>";
            }
        }
        echo "</ul>";
        closedir($handle);
    }
}
?>