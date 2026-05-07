package main

import (
	"bytes"
	"fmt"
	"io"
	"net/http"
	"sync"
	"testing"
)

func doCGIEcho(t *testing.T, connID int, f fixture) {
	t.Helper()

	const cgiPath = "/scripts/good/echo_stdin.py"
	t.Logf("[conn %d] POST %s (%d bytes)", connID, cgiPath, len(f.content))

	req, err := http.NewRequest(http.MethodPost, baseURL+cgiPath, bytes.NewReader(f.content))
	if err != nil {
		t.Fatalf("[conn %d] CGI POST build failed: %v", connID, err)
	}
	req.Header.Set("Content-Type", "application/octet-stream")

	resp, err := httpClient.Do(req)
	if err != nil {
		t.Fatalf("[conn %d] CGI POST failed: %v", connID, err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		t.Fatalf("[conn %d] CGI POST: unexpected status %d", connID, resp.StatusCode)
	}

	got, err := io.ReadAll(resp.Body)
	if err != nil {
		t.Fatalf("[conn %d] CGI POST: failed to read body: %v", connID, err)
	}

	if !bytes.Equal(got, f.content) {
		t.Errorf("[conn %d] CGI echo mismatch for %s\n\twant: %d bytes\n\tgot:  %d bytes",
			connID, f.name, len(f.content), len(got))
		if len(got) > 0 {
			preview := got
			if len(preview) > 200 {
				preview = preview[:200]
			}
			t.Logf("[conn %d] response preview: %q", connID, preview)
		}
	} else {
		t.Logf("[conn %d] CGI echoed %s correctly (%d bytes)", connID, f.name, len(f.content))
	}
}

// ---------------------------------------------------------------------------
// CGI test — one goroutine per fixture
// ---------------------------------------------------------------------------

func TestCGIEcho(t *testing.T) {
	fixtures := []fixture{
		{name: "hello.txt", content: []byte("hello world\n")},
		{name: "foo.html", content: []byte("<html><body>foo</body></html>\n")},
		{name: "bar.json", content: []byte("{\"key\":\"value\"}\n")},
		{name: "baz.md", content: []byte("# baz\n\nsome markdown\n")},
		fixtureFromFile(t, "./data_to_send/pdf_file.pdf"),
	}

	var wg sync.WaitGroup
	wg.Add(len(fixtures))

	for i, f := range fixtures {
		i, f := i, f
		go func() {
			defer wg.Done()
			t.Run(fmt.Sprintf("cgi_%d_%s", i, f.name), func(t *testing.T) {
				doCGIEcho(t, i, f)
			})
		}()
	}

	wg.Wait()
}

 //---------------------------------------------------------------------------
 //CGI test - multiple cgis
 //---------------------------------------------------------------------------
func TestCGIHelloWorld(t *testing.T) {
	scripts := []struct {
		path string
		ext  string
	}{
		{"/scripts/good/hello.py", ".py"},
		{"/scripts/good/hello.php", ".php"},
		{"/scripts/good/hello.lua", ".lua"},
	}

	for _, s := range scripts {
		s := s
		t.Run("cgi_hello"+s.ext, func(t *testing.T) {
			req, err := http.NewRequest(http.MethodGet, baseURL+s.path, nil)
			if err != nil {
				t.Fatalf("GET build failed: %v", err)
			}

			resp, err := httpClient.Do(req)
			if err != nil {
				t.Fatalf("GET %s failed: %v", s.path, err)
			}
			defer resp.Body.Close()

			if resp.StatusCode != http.StatusOK {
				t.Errorf("GET %s: expected 200, got %d", s.path, resp.StatusCode)
			}

			body, err := io.ReadAll(resp.Body)
			if err != nil {
				t.Fatalf("GET %s: failed to read body: %v", s.path, err)
			}

			if !bytes.Contains(body, []byte("Hello, World.")) {
				t.Errorf("GET %s: expected 'Hello, World.' in body, got: %q", s.path, body)
			} else {
				t.Logf("OK - %s returned: %q", s.path, body)
			}
		})
	}
}


func TestCGIBadScript(t *testing.T) {
	const cgiPath = "/scripts/bad/abort.py"

	req, err := http.NewRequest(http.MethodGet, baseURL+cgiPath, nil)
	if err != nil {
		t.Fatalf("build failed: %v", err)
	}

	resp, err := httpClient.Do(req)
	if err != nil {
		t.Fatalf("GET %s failed: %v", cgiPath, err)
	}
	defer resp.Body.Close()

	// CGI process crashes (SIGABRT) — server should return 5xx, not hang or crash
	if resp.StatusCode < 500 || resp.StatusCode > 599 {
		t.Errorf("abort.py: expected 5xx, got %d", resp.StatusCode)
	} else {
		t.Logf("OK - got %d for crashing CGI script", resp.StatusCode)
	}
}

func TestCGIBadBinaryPath(t *testing.T) {
	scripts := []struct {
		path string
		ext  string
	}{
		{"/scripts/bad/hello.py", ".py"},
		{"/scripts/bad/hello.php", ".php"},
		{"/scripts/bad/hello.lua", ".lua"},
	}

	for _, s := range scripts {
		s := s
		t.Run("cgi_bad_binary"+s.ext, func(t *testing.T) {
			req, err := http.NewRequest(http.MethodGet, baseURL+s.path, nil)
			if err != nil {
				t.Fatalf("GET build failed: %v", err)
			}

			resp, err := httpClient.Do(req)
			if err != nil {
				t.Fatalf("GET %s failed: %v", s.path, err)
			}
			defer resp.Body.Close()

			if resp.StatusCode < 500 || resp.StatusCode > 599 {
				t.Errorf("GET %s: expected 5xx (bad binary path), got %d", s.path, resp.StatusCode)
			} else {
				t.Logf("OK - %s correctly returned %d for bad binary path", s.path, resp.StatusCode)
			}
		})
	}
}

