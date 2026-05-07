package main

import (
	"bufio"
	"bytes"
	"fmt"
	"net"
	"net/http"
	"testing"
)

func TestMalformedRequest(t *testing.T) {
	conn, err := net.Dial("tcp", baseAddr)
	if err != nil {
		t.Fatalf("dial failed: %v", err)
	}
	defer conn.Close()

	// Completely malformed — not valid HTTP at all
	fmt.Fprintf(conn, "THIS IS NOT HTTP\r\n\r\n")

	resp, err := http.ReadResponse(bufio.NewReader(conn), nil)
	if err != nil {
		t.Fatalf("failed to read response: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusBadRequest {
		t.Errorf("malformed request: expected 400, got %d", resp.StatusCode)
	} else {
		t.Logf("OK - got 400 for malformed request")
	}
}

func TestMethodNotAllowed(t *testing.T) {
	req, err := http.NewRequest(http.MethodPut, baseURL+"/post", bytes.NewReader([]byte("data")))
	if err != nil {
		t.Fatalf("build failed: %v", err)
	}
	req.Header.Set("Content-Type", "application/octet-stream")

	resp, err := httpClient.Do(req)
	if err != nil {
		t.Fatalf("PUT failed: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusMethodNotAllowed {
		t.Errorf("PUT /post: expected 405, got %d", resp.StatusCode)
	} else {
		t.Logf("OK - got 405 for unsupported method")
	}
}

func TestNotFound(t *testing.T) {
	got := doGet(t, "/this/path/does/not/exist", http.StatusNotFound)
	t.Logf("OK - got 404, body: %q", got[:min(len(got), 100)])
}

func TestHTTPVersionNotSupported(t *testing.T) {
	conn, err := net.Dial("tcp", baseAddr)
	if err != nil {
		t.Fatalf("dial failed: %v", err)
	}
	defer conn.Close()

	fmt.Fprintf(conn, "GET / HTTP/2\r\nHost: 127.0.0.1:8084\r\n\r\n")

	resp, err := http.ReadResponse(bufio.NewReader(conn), nil)
	if err != nil {
		t.Fatalf("failed to read response: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusHTTPVersionNotSupported {
		t.Errorf("HTTP/2 request: expected 505, got %d", resp.StatusCode)
	} else {
		t.Logf("OK - got 505 for HTTP/2")
	}
}

