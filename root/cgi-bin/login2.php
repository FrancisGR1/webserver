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
	echo "<p><a href=\"/login.html\" style=\"color: #7ec8ff;\">Back to Login</a></p>";
	echo "<p><a href=\"/register.html\" style=\"color: #7ec8ff;\">Go to Register</a></p>";
	echo "</body>";
	echo "</html>";
	exit();
}

if ($_SERVER["REQUEST_METHOD"] !== "POST")
	show_message("Invalid Request", "This page only accepts POST requests.");

$username = trim($_POST["username"]);
$password = $_POST["password"];

if ($username === "" || $password === "")
	show_message("Login Error", "Username and password are required.");

$data_file = __DIR__ . "/../data/users.json";

if (!file_exists($data_file))
	show_message("Login Error", "No users registered yet.");

$json_data = file_get_contents($data_file);
if ($json_data === false)
	show_message("Server Error", "Could not read users file.");

$users = json_decode($json_data, true);
if (!is_array($users))
	show_message("Server Error", "Users file is invalid.");

foreach ($users as $user)
{
	if ($user["username"] === $username)
	{
		if (password_verify($password, $user["password"]))
			show_message("Login Success", "Welcome, " . htmlspecialchars($username) . "!");
		else
			show_message("Login Error", "Invalid password.");
	}
}

show_message("Login Error", "User not found.");
?>
