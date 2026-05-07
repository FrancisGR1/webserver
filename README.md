*This project has been created as part of the 42 curriculum by frmiguel, luiz-dos.*

# webserv

## Description

**webserv** is a fully functional HTTP/1.1 web server written in C++98, built from scratch as part of the 42 school curriculum. The goal of the project is to deeply understand how web servers work by reimplementing core features of HTTP from the ground up — without relying on any existing server framework.

The server is configured via a configuration file inspired by NGINX syntax, and supports:

- **HTTP/1.1** methods: `GET`, `POST`, and `DELETE`
- **CGI execution** (e.g. Python, PHP, Bash scripts) for dynamic content
- **Multiple virtual hosts** and server blocks
- **Custom error pages** and configurable error handling
- **File uploads** via POST requests
- **Directory listing** (autoindex)
- **Non-blocking I/O** using `epoll()` 
- **Static file serving** with configurable root directories and index files
- **Chunked transfer encoding**

## Instructions

### Requirements

- A Unix-based system (Linux or macOS)
- C++98 support
- `make`

### Compilation

```bash
make
```

This will produce the `webserv` executable at the project root.

### Running

```bash
./webserv [configuration_file]
```

If no configuration file is provided, the server falls back to a default configuration.

**Example:**
```bash
./webserv config/default.conf
```

### Configuration file

The configuration file allows you to define one or more server blocks. A minimal example:

```nginx
server {
    listen       127.0.0.1:8080;
    server_name  localhost;

    location /cgi-bin {
        cgi_extension .py;
        cgi_path      /usr/bin/python3;
        root         ./www;
    }
}
```

### Cleanup

```bash
make fclean
```

## Resources

### HTTP & Web Servers

- [RFC 7230 – HTTP/1.1 Message Syntax and Routing](https://datatracker.ietf.org/doc/html/rfc7230)
- [RFC 7231 – HTTP/1.1 Semantics and Content](https://datatracker.ietf.org/doc/html/rfc7231)
- [MDN Web Docs – HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP)
- [NGINX Documentation](https://nginx.org/en/docs/) — used as a reference for configuration syntax
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) — essential reference for socket programming in C/C++
- [The C10K Problem](http://www.kegel.com/c10k.html) — classic article on handling many concurrent connections

### CGI

- [RFC 3875 – The Common Gateway Interface (CGI)](https://datatracker.ietf.org/doc/html/rfc3875)
- [CGI Tutorial – W3Schools](https://www.w3schools.com/python/python_cgi.asp)

### C++ & Systems Programming

- [cppreference.com](https://en.cppreference.com/) — C++98 standard library reference
- [Linux man pages](https://man7.org/linux/man-pages/) — for `socket`, `select`, `poll`, `epoll`, `fork`, `execve`, etc.

### AI Usage
- Used AI to write this README
