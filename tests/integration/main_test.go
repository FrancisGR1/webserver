package main

import (
	"bytes"
	"fmt"
	"io"
	"net/http"
	"sync"
	"testing"
)

const baseURL = "http://127.0.0.1:8090"

// one connection per request — matches server's Connection: close behavior
var httpClient = &http.Client{
	Transport: &http.Transport{
		DisableKeepAlives: true,
	},
}

// ---------------------------------------------------------------------------
// File fixtures — each entry is a filename + content pair.
// Add as many as you want; connections round-robin across them.
// ---------------------------------------------------------------------------

type fixture struct {
	name    string
	content []byte
}

var fixtures = []fixture{
	{name: "hello.txt", content: []byte("hello world\n")},
	{name: "foo.html", content: []byte("<html><body>foo</body></html>\n")},
	{name: "bar.json", content: []byte("{\"key\":\"value\"}\n")},
	{name: "baz.md", content: []byte("# baz\n\nsome markdown\n")},
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

func doPost(t *testing.T, location string, f fixture) string {
	t.Helper()
	req, err := http.NewRequest(http.MethodPost, baseURL+location, bytes.NewReader(f.content))
	if err != nil {
		t.Fatalf("POST build failed: %v", err)
	}
	req.Header.Set("Content-Type", "application/octet-stream")

	resp, err := httpClient.Do(req)
	if err != nil {
		t.Fatalf("POST %s failed: %v", location, err)
	}
	defer resp.Body.Close()
	if resp.StatusCode != http.StatusCreated &&
		resp.StatusCode != http.StatusNoContent &&
		resp.StatusCode != http.StatusOK {
		t.Fatalf("POST %s: unexpected status %d", location, resp.StatusCode)
	}

	loc := resp.Header.Get("Location")
	if loc == "" {
		t.Fatalf("POST %s: no Location header in response", location)
	}
	return loc
}

func doGet(t *testing.T, path string, expectedStatus int) []byte {
	t.Helper()
	req, err := http.NewRequest(http.MethodGet, baseURL+path, nil)
	if err != nil {
		t.Fatalf("GET build failed: %v", err)
	}
	resp, err := httpClient.Do(req)
	if err != nil {
		t.Fatalf("GET %s failed: %v", path, err)
	}
	defer resp.Body.Close()
	if resp.StatusCode != expectedStatus {
		t.Errorf("GET %s: expected status %d, got %d", path, expectedStatus, resp.StatusCode)
	}
	body, err := io.ReadAll(resp.Body)
	if err != nil {
		t.Fatalf("GET %s: failed to read body: %v", path, err)
	}
	return body
}

func doDelete(t *testing.T, path string) {
	t.Helper()
	req, err := http.NewRequest(http.MethodDelete, baseURL+path, nil)
	if err != nil {
		t.Fatalf("DELETE %s: failed to build request: %v", path, err)
	}
	resp, err := httpClient.Do(req)
	if err != nil {
		t.Fatalf("DELETE %s failed: %v", path, err)
	}
	defer resp.Body.Close()
	if resp.StatusCode != http.StatusNoContent && resp.StatusCode != http.StatusOK {
		t.Fatalf("DELETE %s: unexpected status %d", path, resp.StatusCode)
	}
}

// ---------------------------------------------------------------------------
// Core flow: POST -> GET (compare body) -> DELETE -> GET (expect 404)
// ---------------------------------------------------------------------------

func runFlow(t *testing.T, connID int, f fixture) {
	t.Helper()

	t.Logf("[conn %d] POST /post (%d bytes)", connID, len(f.content))
	resourcePath := doPost(t, "/post", f)
	t.Logf("[conn %d] server created resource at: %s", connID, resourcePath)

	t.Logf("[conn %d] GET %s — expect 200 + matching body", connID, resourcePath)
	got := doGet(t, resourcePath, http.StatusOK)
	if !bytes.Equal(got, f.content) {
		t.Errorf("[conn %d] body mismatch\n\twant: %q\n\tgot:  %q", connID, f.content, got)
	}

	t.Logf("[conn %d] DELETE %s", connID, resourcePath)
	doDelete(t, resourcePath)

	t.Logf("[conn %d] GET %s — expect 404 after delete", connID, resourcePath)
	doGet(t, resourcePath, http.StatusNotFound)
}

// ---------------------------------------------------------------------------
// Integration test — adjust nConns to control parallelism
// ---------------------------------------------------------------------------

func TestPostGetDeleteGet(t *testing.T) {
	const nConns = 4

	var wg sync.WaitGroup
	wg.Add(nConns)

	for i := 0; i < nConns; i++ {
		i := i
		f := fixtures[i%len(fixtures)]

		go func() {
			defer wg.Done()
			t.Run(fmt.Sprintf("conn_%d_%s", i, f.name), func(t *testing.T) {
				runFlow(t, i, f)
			})
		}()
	}

	wg.Wait()
}
