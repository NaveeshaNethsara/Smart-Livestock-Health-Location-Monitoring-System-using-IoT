<?php
include('conf.php');

if (isset($_POST['submit'])) {
    $email = $_POST['email'];
    $password = $_POST['password'];

    $sql = "SELECT * FROM users WHERE email = '$email' AND password = '$password'";
    $result = mysqli_query($conn, $sql);
    $count = mysqli_num_rows($result);

    if ($count === 1) {
        header("Location: dashboard.html");
        exit();
    } else {
        echo '<script>
            alert("Login failed. Invalid email or password.");
            window.location.href = "form.html";
        </script>';
    }
}
?>