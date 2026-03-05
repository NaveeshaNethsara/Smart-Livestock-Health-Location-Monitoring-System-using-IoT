<?php
include 'conf.php';

echo "<pre>";
print_r($_POST);
echo "</pre>";

if (isset($_POST['email']) && isset($_POST['name']) && isset($_POST['password'])) {
    $email = mysqli_real_escape_string($conn, $_POST['email']);
    $name = mysqli_real_escape_string($conn, $_POST['name']);
    $password = mysqli_real_escape_string($conn, $_POST['password']); 

    $sql = "INSERT INTO users (email, name, password) VALUES ('$email', '$name', '$password')";

    if (mysqli_query($conn, $sql)) {
        echo "New user registered successfully.";
    } else {
        echo "Error: " . $sql . "<br>" . mysqli_error($conn);
    }

} else {
    echo "Required fields are missing.";
}

mysqli_close($conn);
?>