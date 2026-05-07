package main

import (
	"os"
	"path/filepath"
	"bytes"
	"io"
	"net/http"
	"testing"
)

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

func fixtureFromFile(t *testing.T, path string) fixture {
    t.Helper()
    content, err := os.ReadFile(path)
    if err != nil {
        t.Fatalf("fixtureFromFile: failed to read %s: %v", path, err)
    }
    return fixture{name: filepath.Base(path), content: content}
}


