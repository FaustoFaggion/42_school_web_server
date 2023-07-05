<?php
http_response_code(200);
header("HTTP/1.1 200 OK");
// header("Status: 200 OK");

        echo "<body>\n";

        echo "<br>";
        echo "<hr>";
        echo $_SERVER['SERVER_PROTOCOL'];

        echo "<br>";
        echo "<hr>";
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
        echo "</body>\n";
?>