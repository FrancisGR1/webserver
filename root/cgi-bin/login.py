#!/usr/bin/env python3

import html
import json
import os
import sys
import warnings
from urllib.parse import parse_qs

warnings.filterwarnings("ignore", category=DeprecationWarning)
import crypt


def show_message(title, message):
	safe_title = html.escape(title)
	safe_message = html.escape(message)

	print("Content-Type: text/html\r\n\r\n", end="")
	print(f"""<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="utf-8">
	<title>{safe_title}</title>
</head>
<body style="font-family: Arial, sans-serif; text-align: center; padding: 40px; background-color: #111; color: white;">
	<h1>{safe_title}</h1>
	<p>{safe_message}</p>
	<p><a href="/login.html" style="color: #7ec8ff;">Back to Login</a></p>
	<p><a href="/register.html" style="color: #7ec8ff;">Go to Register</a></p>
</body>
</html>""")


def password_verify(password, stored_hash):
	if not stored_hash:
		return False
	return crypt.crypt(password, stored_hash) == stored_hash


def read_post_form():
	try:
		content_length = int(os.environ.get("CONTENT_LENGTH", "0"))
	except ValueError:
		content_length = 0

	body = sys.stdin.read(content_length)
	parsed = parse_qs(body, keep_blank_values=True)

	form = {}
	for key, values in parsed.items():
		if values:
			form[key] = values[0]
		else:
			form[key] = ""

	return form


def main():
	if os.environ.get("REQUEST_METHOD") != "POST":
		show_message("Invalid Request", "This page only accepts POST requests.")
		return

	form = read_post_form()
	username = form.get("username", "").strip()
	password = form.get("password", "")

	if not username or not password:
		show_message("Login Error", "Username and password are required.")
		return

	data_file = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data", "users.json")

	if not os.path.exists(data_file):
		show_message("Login Error", "No users registered yet.")
		return

	try:
		with open(data_file, "r") as file:
			users = json.load(file)
	except (OSError, json.JSONDecodeError):
		show_message("Server Error", "Users file is invalid.")
		return

	if not isinstance(users, list):
		show_message("Server Error", "Users file is invalid.")
		return

	for user in users:
		if user.get("username") == username:
			if password_verify(password, user.get("password", "")):
				show_message("Login Success", "Welcome, " + username + "!")
			else:
				show_message("Login Error", "Invalid password.")
			return

	show_message("Login Error", "User not found.")


if __name__ == "__main__":
	main()
