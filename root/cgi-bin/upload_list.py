#!/usr/bin/env python3

import html
import os
from urllib.parse import quote


def main():
	print("Content-Type: text/html\r\n\r\n", end="")

	script_dir = os.path.dirname(os.path.abspath(__file__))
	upload_dir = os.path.abspath(os.path.join(script_dir, "..", "uploads"))

	print("""<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="utf-8">
	<title>Upload List</title>
	<link rel="stylesheet" href="/css/styles.global.css">
	<style>
		.file-list {
			max-width: 520px;
			margin: 40px auto;
			text-align: left;
			background: white;
			color: #333;
			border: 1px solid #333;
			border-radius: 8px;
			padding: 24px;
		}
		.file-list a {
			color: #333;
			font-weight: bold;
		}
		.empty {
			text-align: center;
		}
	</style>
</head>
<body>
	<header>
		<h1>Webserver 42</h1>
	</header>
	<nav>
		<a href="/index.html">Home</a>
		<a href="/videos.html">Videos</a>
		<a href="/diagram.html">Diagram</a>
		<a href="/upload.html">Upload</a>
		<a href="/cgi-bin/upload_list.py">Upload list</a>
		<a href="/wiki">Wikipedia</a>
		<a href="/login.html">Login</a>
		<a href="/register.html">Register</a>
	</nav>
	<main>
		<h2>Uploaded Files</h2>""")

	if not os.path.isdir(upload_dir):
		print("<p class=\"empty\">No upload directory found.</p>")
	else:
		files = []
		for name in os.listdir(upload_dir):
			file_path = os.path.join(upload_dir, name)
			if os.path.isfile(file_path):
				files.append(name)

		files.sort()

		if not files:
			print("<p class=\"empty\">No files uploaded yet.</p>")
		else:
			print("<div class=\"file-list\">")
			for filename in files:
				safe_name = html.escape(filename)
				safe_url = quote(filename)
				print(f"<p><a href=\"/uploads/{safe_url}\">{safe_name}</a></p>")
			print("</div>")

	print("""    </main>
	<footer>
		<p>&copy; 2026 Webserver 42</p>
	</footer>
</body>
</html>""")


if __name__ == "__main__":
	main()
