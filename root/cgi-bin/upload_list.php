<?php
echo "Content-Type: text/html\r\n\r\n";

$upload_dir = __DIR__ . "/../uploads";

echo "<!DOCTYPE html>";
echo "<html lang=\"en\">";
echo "<head>";
echo "<meta charset=\"utf-8\">";
echo "<title>Upload List</title>";
echo "<style>
	body {
		font-family: Arial;
		background-color: #111;
		color: white;
		text-align: center;
		padding: 40px;
	}
	a {
		color: #7ec8ff;
		text-decoration: none;
	}
	a:hover {
		text-decoration: underline;
	}
	.file-list {
		max-width: 400px;
		margin: 0 auto;
		text-align: left;
	}
</style>";
echo "</head>";
echo "<body>";

echo "<h1>Uploaded Files</h1>";

if (!is_dir($upload_dir))
{
	echo "<p>No upload directory found.</p>";
	exit();
}

$files = scandir($upload_dir);

echo "<div class='file-list'>";

foreach ($files as $file)
{
	if ($file === "." || $file === "..")
		continue;

	$safe_file = htmlspecialchars($file);

	echo "<p><a href=\"/uploads/" . $safe_file . "\">" . $safe_file . "</a></p>";
}

echo "</div>";

echo "<p><a href=\"/index.html\">Back to Home</a></p>";

echo "</body>";
echo "</html>";
?>
