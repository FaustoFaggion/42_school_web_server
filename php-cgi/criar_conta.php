<?php

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

// 	// Verifica se o formulário foi submetido
// if ($_SERVER['REQUEST_METHOD'] === 'POST') {
//     // Obtém os valores do formulário
//     $username = $_POST['username'];
//     $password = $_POST['password'];

//     // Aqui você pode adicionar código para verificar e validar os dados do formulário

//     // Exemplo simples: exibe os valores inseridos
//     echo  "<p>Nome de usuário:" . $username . "<br>";
//     echo "Senha: " . $password;
// }
?>