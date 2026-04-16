<?php
	echo "Content-Type: text/html\r\n\r\n";

	echo "<h1>Register CGI working</h1>";

	if ($_SERVER["REQUEST_METHOD"] === "POST")
	{
		echo "<p>Username: " . $_POST["username"] . "</p>";
		echo "<p>Email: " . $_POST["email"] . "</p>";
		echo "<p>Register data received</p>";
	}
?>
