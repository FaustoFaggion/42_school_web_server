<?php

echo "HELLO WORD\n";

var_dump($_ENV);
echo "<br>";
echo "<hr>";
parse_str($_SERVER['QUERY_STRING'], $queryParams);
// Accessing individual parameters
$username = $queryParams['username'];
$password = $queryParams['password'];

echo "Bem vindo " . $username . "!!!" . "<br>"; 
echo "Seu password é: " . $password . "." . "<br>";

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
?>