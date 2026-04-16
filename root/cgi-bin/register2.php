<?php
echo "Content-Type: text/html\r\n\r\n";

function show_message($title, $message)
{
	echo "<!DOCTYPE html>";
	echo "<html lang=\"en\">";
	echo "<head>";
	echo "<meta charset=\"utf-8\">";
	echo "<title>" . $title . "</title>";
	echo "</head>";
	echo "<body style=\"font-family: Arial, sans-serif; text-align: center; padding: 40px; background-color: #111; color: white;\">";
	echo "<h1>" . $title . "</h1>";
	echo "<p>" . $message . "</p>";
	echo "<p><a href=\"/register.html\" style=\"color: #7ec8ff;\">Back to Register</a></p>";
	echo "<p><a href=\"/login.html\" style=\"color: #7ec8ff;\">Go to Login</a></p>";
	echo "</body>";
	echo "</html>";
	exit();
}

if ($_SERVER["REQUEST_METHOD"] !== "POST")
	show_message("Invalid Request", "This page only accepts POST requests.");

$username = trim($_POST["username"]);
$email = trim($_POST["email"]);
$password = $_POST["password"];
$confirm_password = $_POST["confirm_password"];

if ($username === "" || $email === "" || $password === "" || $confirm_password === "")
	show_message("Register Error", "All fields are required.");

if ($password !== $confirm_password)
	show_message("Register Error", "Passwords do not match.");

if (!filter_var($email, FILTER_VALIDATE_EMAIL))
	show_message("Register Error", "Invalid email format.");

$data_file = __DIR__ . "/../data/users.json";

if (!file_exists($data_file))
{
	$created = file_put_contents($data_file, "[]");
	if ($created === false)
		show_message("Server Error", "Could not create users file.");
}

$json_data = file_get_contents($data_file);
if ($json_data === false)
	show_message("Server Error", "Could not read users file.");

$users = json_decode($json_data, true);
if (!is_array($users))
	$users = [];

foreach ($users as $user)
{
	if ($user["username"] === $username)
		show_message("Register Error", "Username already exists.");
	if ($user["email"] === $email)
		show_message("Register Error", "Email already exists.");
}

$new_user = array(
	"username" => $username,
	"email" => $email,
	"password" => password_hash($password, PASSWORD_DEFAULT)
);

$users[] = $new_user;

$new_json = json_encode($users, JSON_PRETTY_PRINT);
if ($new_json === false)
	show_message("Server Error", "Could not encode users data.");

$saved = file_put_contents($data_file, $new_json);
if ($saved === false)
	show_message("Server Error", "Could not save new user.");

show_message("Register Success", "Account created successfully.");
?>
