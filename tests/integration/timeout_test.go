package main

import (
	"bytes"
	"fmt"
	"io"
	"time"
	"net"
	"net/http"
	"strings"
	"testing"
)

func TestPrematureClose(t *testing.T) {
	conn, err := net.Dial("tcp", baseAddr)
	if err != nil {
		t.Fatalf("dial failed: %v", err)
	}

	header := "POST /post HTTP/1.1\r\n" +
		"Host: 127.0.0.1:8084\r\n" +
		"Content-Type: application/octet-stream\r\n" +
		"Content-Length: 1048576\r\n" +
		"\r\n"

	_, err = fmt.Fprint(conn, header)
	if err != nil {
		t.Fatalf("failed to send headers: %v", err)
	}

	_, err = conn.Write(bytes.Repeat([]byte("x"), 128))
	if err != nil {
		t.Fatalf("failed to send partial body: %v", err)
	}

	conn.Close()
	time.Sleep(100 * time.Millisecond)

	// Health check: just verify server responds to anything at all
	t.Log("checking server is still responsive after premature close")
	req, err := http.NewRequest(http.MethodPost, baseURL+"/post", bytes.NewReader([]byte("alive")))
	if err != nil {
		t.Fatalf("health check build failed: %v", err)
	}
	req.Header.Set("Content-Type", "application/octet-stream")

	resp, err := httpClient.Do(req)
	if err != nil {
		t.Fatalf("server did not respond after premature close: %v", err)
	}
	defer resp.Body.Close()
	io.ReadAll(resp.Body)

	if resp.StatusCode == 0 {
		t.Error("got zero status — server may have crashed")
	}
	t.Logf("OK - server alive, responded with %d", resp.StatusCode)
}

func TestRequestTimeout(t *testing.T) {
	conn, err := net.Dial("tcp", baseAddr)
	if err != nil {
		t.Fatalf("dial failed: %v", err)
	}
	defer conn.Close()

	// send request byte by byte, slowly enough to exceed the 20s request timeout
	request := []byte("GET /post HTTP/1.1\r\nHost: " + baseAddr + "\r\n\r\n",)
	for i := 0; i < len(request); i++ {
		_, err := conn.Write(request[i : i+1])
		if err != nil {
			// server closed connection on us — that's expected after timeout
			t.Logf("server closed connection after %d bytes: %v", i, err)
			break
		}
		time.Sleep(1 * time.Second)
	}

	// read whatever the server sent before closing
	conn.SetReadDeadline(time.Now().Add(5 * time.Second))
	buf := make([]byte, 4096)
	n, _ := conn.Read(buf)
	response := string(buf[:n])

	if !strings.Contains(response, "408") {
		t.Errorf("expected 408 in response, got: %q", response)
	} else {
		t.Logf("OK - got 408: %q", response)
	}
}
