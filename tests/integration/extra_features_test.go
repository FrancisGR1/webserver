package main

import (
	"bytes"
	//"fmt"
	"io"
	"net/http"
	"strings"
	"sync"
	"testing"
)

// ---------------------------------------------------------------------------
// Extra feature tests — uses ports 8091-8102
// ---------------------------------------------------------------------------

var (
	baseURL1 = "http://127.0.0.1:8091" // service 1
	baseURL2 = "http://127.0.0.1:8097" // service 2
	baseURL3 = "http://127.0.0.1:8100" // service 3
)

// ---------------------------------------------------------------------------
// Multiple services — all clients try to GET the same file
// ---------------------------------------------------------------------------

func TestMultipleServices(t *testing.T) {
	// first POST a file to service 1
	content := []byte("shared file content\n")
	req, err := http.NewRequest(http.MethodPost, baseURL1+"/post", bytes.NewReader(content))
	if err != nil {
		t.Fatalf("POST build failed: %v", err)
	}
	req.Header.Set("Content-Type", "application/octet-stream")
	resp, err := httpClient.Do(req)
	if err != nil {
		t.Fatalf("POST failed: %v", err)
	}
	defer resp.Body.Close()
	loc := resp.Header.Get("Location")
	if loc == "" {
		t.Fatalf("POST: no Location header")
	}

	// all services try to GET the same resource path
	services := []string{
		"http://127.0.0.1:8091",
		"http://127.0.0.1:8092",
		"http://127.0.0.1:8093",
		"http://127.0.0.1:8094",
		"http://127.0.0.1:8095",
		"http://127.0.0.1:8096",
		"http://127.0.0.1:8097",
		"http://127.0.0.1:8098",
		"http://127.0.0.1:8099",
		"http://127.0.0.1:8100",
		"http://127.0.0.1:8101",
		"http://127.0.0.1:8102",
	}

	var wg sync.WaitGroup
	wg.Add(len(services))

	for _, base := range services {
		base := base
		go func() {
			defer wg.Done()
			t.Run("service_"+base, func(t *testing.T) {
				req, err := http.NewRequest(http.MethodGet, base+loc, nil)
				if err != nil {
					t.Fatalf("GET build failed: %v", err)
				}
				resp, err := httpClient.Do(req)
				if err != nil {
					t.Fatalf("GET %s%s failed: %v", base, loc, err)
				}
				defer resp.Body.Close()
				if resp.StatusCode != http.StatusOK {
					t.Errorf("GET %s%s: expected 200, got %d", base, loc, resp.StatusCode)
					return
				}
				body, err := io.ReadAll(resp.Body)
				if err != nil {
					t.Fatalf("GET %s%s: read body failed: %v", base, loc, err)
				}
				if !bytes.Equal(body, content) {
					t.Errorf("GET %s%s: body mismatch", base, loc)
				} else {
					t.Logf("OK - %s returned correct content", base)
				}
			})
		}()
	}

	wg.Wait()
}

// ---------------------------------------------------------------------------
// Redirection — expect 301
// ---------------------------------------------------------------------------

func TestRedirection(t *testing.T) {
	// don't follow redirects
	client := &http.Client{
		CheckRedirect: func(req *http.Request, via []*http.Request) error {
			return http.ErrUseLastResponse
		},
		Transport: &http.Transport{DisableKeepAlives: true},
	}

	req, err := http.NewRequest(http.MethodGet, baseURL1+"/redir", nil)
	if err != nil {
		t.Fatalf("GET build failed: %v", err)
	}

	resp, err := client.Do(req)
	if err != nil {
		t.Fatalf("GET /redir failed: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusMovedPermanently {
		t.Errorf("expected 301, got %d", resp.StatusCode)
	} else {
		t.Logf("OK - got 301, Location: %s", resp.Header.Get("Location"))
	}
}

// ---------------------------------------------------------------------------
// Custom error page — GET nonexistent file, expect custom 404 body
// ---------------------------------------------------------------------------

func TestCustomErrorPage(t *testing.T) {
	req, err := http.NewRequest(http.MethodGet, baseURL1+"/errors/doesnotexist.html", nil)
	if err != nil {
		t.Fatalf("GET build failed: %v", err)
	}

	resp, err := httpClient.Do(req)
	if err != nil {
		t.Fatalf("GET failed: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNotFound {
		t.Errorf("expected 404, got %d", resp.StatusCode)
	}

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		t.Fatalf("read body failed: %v", err)
	}

	if !strings.Contains(string(body), "custom 404 error") {
		t.Errorf("expected 'custom 404 error' in body, got: %q", string(body)[:min(len(body), 200)])
	} else {
		t.Logf("OK - got custom 404 page")
	}
}

// ---------------------------------------------------------------------------
// Upload off — expect 403 Forbidden
// ---------------------------------------------------------------------------

func TestUploadOff(t *testing.T) {
	req, err := http.NewRequest(http.MethodPost, baseURL1+"/no_upload", bytes.NewReader([]byte("some data")))
	if err != nil {
		t.Fatalf("POST build failed: %v", err)
	}
	req.Header.Set("Content-Type", "application/octet-stream")

	resp, err := httpClient.Do(req)
	if err != nil {
		t.Fatalf("POST /no_upload failed: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusForbidden {
		t.Errorf("expected 403, got %d", resp.StatusCode)
	} else {
		t.Logf("OK - got 403 for upload off location")
	}
}

// ---------------------------------------------------------------------------
// Default file — expect body containing "default"
// ---------------------------------------------------------------------------

func TestDefaultFile(t *testing.T) {
	req, err := http.NewRequest(http.MethodGet, baseURL1+"/default", nil)
	if err != nil {
		t.Fatalf("GET build failed: %v", err)
	}

	resp, err := httpClient.Do(req)
	if err != nil {
		t.Fatalf("GET /default failed: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		t.Errorf("expected 200, got %d", resp.StatusCode)
	}

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		t.Fatalf("read body failed: %v", err)
	}

	if !strings.Contains(string(body), "default") {
		t.Errorf("expected 'default' in body, got: %q", string(body)[:min(len(body), 200)])
	} else {
		t.Logf("OK - got default file: %q", string(body)[:min(len(body), 100)])
	}
}

// ---------------------------------------------------------------------------
// Max body size — POST >100 bytes to /custom_errors, expect 413
// ---------------------------------------------------------------------------

func TestMaxBodySize(t *testing.T) {
	body := bytes.Repeat([]byte("x"), 200)

	req, err := http.NewRequest(http.MethodPost, baseURL1+"/errors", bytes.NewReader(body))
	if err != nil {
		t.Fatalf("POST build failed: %v", err)
	}
	req.Header.Set("Content-Type", "application/octet-stream")

	resp, err := httpClient.Do(req)
	if err != nil {
		t.Fatalf("POST /custom_errors failed: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusRequestEntityTooLarge {
		t.Errorf("expected 413, got %d", resp.StatusCode)
	} else {
		t.Logf("OK - got 413 for oversized body")
	}
}

// ---------------------------------------------------------------------------
// Directory listing — expect dir1, dir2, dir3 in body
// ---------------------------------------------------------------------------

func TestDirectoryListing(t *testing.T) {
	req, err := http.NewRequest(http.MethodGet, baseURL1+"/listing", nil)
	if err != nil {
		t.Fatalf("GET build failed: %v", err)
	}

	resp, err := httpClient.Do(req)
	if err != nil {
		t.Fatalf("GET /listing failed: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		t.Errorf("expected 200, got %d", resp.StatusCode)
	}

	body, err := io.ReadAll(resp.Body)
	t.Logf("body: %q", body)
	if err != nil {
		t.Fatalf("read body failed: %v", err)
	}

	bodyStr := string(body)
	dirs := []string{"dir1", "dir2", "dir3"}
	for _, dir := range dirs {
		if !strings.Contains(bodyStr, dir) {
			t.Errorf("expected '%s' in listing body", dir)
		} else {
			t.Logf("OK - found '%s' in listing", dir)
		}
	}

	if t.Failed() {
		t.Logf("listing body preview: %q", bodyStr[:min(len(bodyStr), 500)])
	}
}

func min(a, b int) int {
	if a < b {
		return a
	}
	return b
}
