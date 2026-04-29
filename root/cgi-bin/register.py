#!/usr/bin/env python3

import html
import json
import os
import re
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
	<p><a href="/register.html" style="color: #7ec8ff;">Back to Register</a></p>
	<p><a href="/login.html" style="color: #7ec8ff;">Go to Login</a></p>
</body>
</html>""")


def is_valid_email(email):
	return re.match(r"^[^@\s]+@[^@\s]+\.[^@\s]+$", email) is not None


def password_hash(password):
	return crypt.crypt(password, crypt.mksalt(crypt.METHOD_BLOWFISH))


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
	email = form.get("email", "").strip()
	password = form.get("password", "")
	confirm_password = form.get("confirm_password", "")

	if not username or not email or not password or not confirm_password:
		show_message("Register Error", "All fields are required.")
		return

	if password != confirm_password:
		show_message("Register Error", "Passwords do not match.")
		return

	if not is_valid_email(email):
		show_message("Register Error", "Invalid email format.")
		return

	data_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")
	data_file = os.path.join(data_dir, "users.json")

	try:
		if not os.path.isdir(data_dir):
			os.makedirs(data_dir)
		if not os.path.exists(data_file):
			with open(data_file, "w") as file:
				file.write("[]")

		with open(data_file, "r") as file:
			users = json.load(file)
	except OSError:
		show_message("Server Error", "Could not read users file.")
		return
	except json.JSONDecodeError:
		users = []

	if not isinstance(users, list):
		users = []

	for user in users:
		if user.get("username") == username:
			show_message("Register Error", "Username already exists.")
			return
		if user.get("email") == email:
			show_message("Register Error", "Email already exists.")
			return

	users.append({
		"username": username,
		"email": email,
		"password": password_hash(password),
	})

	try:
		with open(data_file, "w") as file:
			json.dump(users, file, indent=4)
	except OSError:
		show_message("Server Error", "Could not save new user.")
		return

	show_message("Register Success", "Account created successfully.")


if __name__ == "__main__":
	main()
