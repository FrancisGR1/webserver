package main

import (
	//"bufio"
	"bytes"
	//"fmt"
	"io"
	//"time"
	//"net"
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

func fixtureFromFile(t *testing.T, path string) fixture {
    t.Helper()
    content, err := os.ReadFile(path)
    if err != nil {
        t.Fatalf("fixtureFromFile: failed to read %s: %v", path, err)
    }
    return fixture{name: filepath.Base(path), content: content}
}

//
//// ---------------------------------------------------------------------------
//// Integration test — adjust nConns to control parallelism
//// ---------------------------------------------------------------------------
//
//func TestPostGetDeleteGet(t *testing.T) {
//    baseFixtures := []fixture{
//        {name: "hello.txt", content: []byte("hello world\n")},
//        {name: "foo.html", content: []byte("<html><body>foo</body></html>\n")},
//        {name: "bar.json", content: []byte("{\"key\":\"value\"}\n")},
//        {name: "baz.md", content: []byte("# baz\n\nsome markdown\n")},
//        fixtureFromFile(t, "./data_to_send/pdf_file.pdf"),
//    }
//
//    const nConns = 4
//
//    var wg sync.WaitGroup
//    wg.Add(nConns)
//
//    for i := 0; i < nConns; i++ {
//        i := i
//        f := baseFixtures[i%len(baseFixtures)]
//
//        go func() {
//            defer wg.Done()
//            t.Run(fmt.Sprintf("conn_%d_%s", i, f.name), func(t *testing.T) {
//                runFlow(t, i, f)
//            })
//        }()
//    }
//
//    wg.Wait()
//}

//func buildMultipartBody(boundary string, parts []fixture) []byte {
//	var buf bytes.Buffer
//	for _, f := range parts {
//		buf.WriteString("--" + boundary + "\r\n")
//		buf.WriteString(fmt.Sprintf(
//			"Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n",
//			f.name, f.name,
//		))
//		buf.WriteString("Content-Type: application/octet-stream\r\n")
//		buf.WriteString("\r\n")
//		buf.Write(f.content)
//		buf.WriteString("\r\n")
//	}
//	buf.WriteString("--" + boundary + "--\r\n")
//	return buf.Bytes()
//}
//
//func TestMultipartPostGet(t *testing.T) {
//	fixtures := []fixture{
//		{name: "hello.txt", content: []byte("hello world\n")},
//		{name: "foo.html", content: []byte("<html><body>foo</body></html>\n")},
//		{name: "bar.json", content: []byte("{\"key\":\"value\"}\n")},
//		{name: "baz.md", content: []byte("# baz\n\nsome markdown\n")},
//		fixtureFromFile(t, "./data_to_send/pdf_file.pdf"),
//	}
//
//	boundary := "boundary42"
//	body := buildMultipartBody(boundary, fixtures)
//
//	req, err := http.NewRequest(http.MethodPost, baseURL+"/post", bytes.NewReader(body))
//	if err != nil {
//		t.Fatalf("multipart POST build failed: %v", err)
//	}
//	req.Header.Set("Content-Type", "multipart/form-data; boundary="+boundary)
//
//	resp, err := httpClient.Do(req)
//	if err != nil {
//		t.Fatalf("multipart POST failed: %v", err)
//	}
//	defer resp.Body.Close()
//
//	if resp.StatusCode != http.StatusCreated {
//		body, _ := io.ReadAll(resp.Body)
//		t.Fatalf("multipart POST: expected 201, got %d — body: %q", resp.StatusCode, body)
//	}
//
//	respBody, _ := io.ReadAll(resp.Body)
//	t.Logf("multipart POST response: %s", respBody)
//
//	// GET each uploaded file by filename and compare
//	for _, f := range fixtures {
//		f := f
//		t.Run("get_"+f.name, func(t *testing.T) {
//			got := doGet(t, "/post/"+f.name, http.StatusOK)
//			if !bytes.Equal(got, f.content) {
//				t.Errorf("%s: body mismatch — want %d bytes, got %d bytes",
//					f.name, len(f.content), len(got))
//				if len(got) > 0 {
//					preview := got
//					if len(preview) > 200 {
//						preview = preview[:200]
//					}
//					t.Logf("response preview: %q", preview)
//				}
//			} else {
//				t.Logf("OK - %s: %d bytes match", f.name, len(f.content))
//			}
//		})
//	}
//}
// ---------------------------------------------------------------------------
// CGI echo helper: POST body to CGI script, expect identical body back
// ---------------------------------------------------------------------------

//func doCGIEcho(t *testing.T, connID int, f fixture) {
//	t.Helper()
//
//	const cgiPath = "/scripts/good/echo_stdin.py"
//	t.Logf("[conn %d] POST %s (%d bytes)", connID, cgiPath, len(f.content))
//
//	req, err := http.NewRequest(http.MethodPost, baseURL+cgiPath, bytes.NewReader(f.content))
//	if err != nil {
//		t.Fatalf("[conn %d] CGI POST build failed: %v", connID, err)
//	}
//	req.Header.Set("Content-Type", "application/octet-stream")
//
//	resp, err := httpClient.Do(req)
//	if err != nil {
//		t.Fatalf("[conn %d] CGI POST failed: %v", connID, err)
//	}
//	defer resp.Body.Close()
//
//	if resp.StatusCode != http.StatusOK {
//		t.Fatalf("[conn %d] CGI POST: unexpected status %d", connID, resp.StatusCode)
//	}
//
//	got, err := io.ReadAll(resp.Body)
//	if err != nil {
//		t.Fatalf("[conn %d] CGI POST: failed to read body: %v", connID, err)
//	}
//
//	if !bytes.Equal(got, f.content) {
//		t.Errorf("[conn %d] CGI echo mismatch for %s\n\twant: %d bytes\n\tgot:  %d bytes",
//			connID, f.name, len(f.content), len(got))
//		if len(got) > 0 {
//			preview := got
//			if len(preview) > 200 {
//				preview = preview[:200]
//			}
//			t.Logf("[conn %d] response preview: %q", connID, preview)
//		}
//	} else {
//		t.Logf("[conn %d] CGI echoed %s correctly (%d bytes)", connID, f.name, len(f.content))
//	}
//}
//
//// ---------------------------------------------------------------------------
//// CGI test — one goroutine per fixture
//// ---------------------------------------------------------------------------
//
//func TestCGIEcho(t *testing.T) {
//	fixtures := []fixture{
//		{name: "hello.txt", content: []byte("hello world\n")},
//		{name: "foo.html", content: []byte("<html><body>foo</body></html>\n")},
//		{name: "bar.json", content: []byte("{\"key\":\"value\"}\n")},
//		{name: "baz.md", content: []byte("# baz\n\nsome markdown\n")},
//		fixtureFromFile(t, "./data_to_send/pdf_file.pdf"),
//	}
//
//	var wg sync.WaitGroup
//	wg.Add(len(fixtures))
//
//	for i, f := range fixtures {
//		i, f := i, f
//		go func() {
//			defer wg.Done()
//			t.Run(fmt.Sprintf("cgi_%d_%s", i, f.name), func(t *testing.T) {
//				doCGIEcho(t, i, f)
//			})
//		}()
//	}
//
//	wg.Wait()
//}

// ---------------------------------------------------------------------------
// CGI test - multiple cgis
// ---------------------------------------------------------------------------
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

// Error condition tests
// ---------------------------------------------------------------------------

//func TestMalformedRequest(t *testing.T) {
//	conn, err := net.Dial("tcp", baseAddr)
//	if err != nil {
//		t.Fatalf("dial failed: %v", err)
//	}
//	defer conn.Close()
//
//	// Completely malformed — not valid HTTP at all
//	fmt.Fprintf(conn, "THIS IS NOT HTTP\r\n\r\n")
//
//	resp, err := http.ReadResponse(bufio.NewReader(conn), nil)
//	if err != nil {
//		t.Fatalf("failed to read response: %v", err)
//	}
//	defer resp.Body.Close()
//
//	if resp.StatusCode != http.StatusBadRequest {
//		t.Errorf("malformed request: expected 400, got %d", resp.StatusCode)
//	} else {
//		t.Logf("OK - got 400 for malformed request")
//	}
//}
//
//func TestMethodNotAllowed(t *testing.T) {
//	req, err := http.NewRequest(http.MethodPut, baseURL+"/post", bytes.NewReader([]byte("data")))
//	if err != nil {
//		t.Fatalf("build failed: %v", err)
//	}
//	req.Header.Set("Content-Type", "application/octet-stream")
//
//	resp, err := httpClient.Do(req)
//	if err != nil {
//		t.Fatalf("PUT failed: %v", err)
//	}
//	defer resp.Body.Close()
//
//	if resp.StatusCode != http.StatusMethodNotAllowed {
//		t.Errorf("PUT /post: expected 405, got %d", resp.StatusCode)
//	} else {
//		t.Logf("OK - got 405 for unsupported method")
//	}
//}
//
//func TestNotFound(t *testing.T) {
//	got := doGet(t, "/this/path/does/not/exist", http.StatusNotFound)
//	t.Logf("OK - got 404, body: %q", got[:min(len(got), 100)])
//}
//
//func TestHTTPVersionNotSupported(t *testing.T) {
//	conn, err := net.Dial("tcp", baseAddr)
//	if err != nil {
//		t.Fatalf("dial failed: %v", err)
//	}
//	defer conn.Close()
//
//	fmt.Fprintf(conn, "GET / HTTP/2\r\nHost: 127.0.0.1:8084\r\n\r\n")
//
//	resp, err := http.ReadResponse(bufio.NewReader(conn), nil)
//	if err != nil {
//		t.Fatalf("failed to read response: %v", err)
//	}
//	defer resp.Body.Close()
//
//	if resp.StatusCode != http.StatusHTTPVersionNotSupported {
//		t.Errorf("HTTP/2 request: expected 505, got %d", resp.StatusCode)
//	} else {
//		t.Logf("OK - got 505 for HTTP/2")
//	}
//}
//
//func TestBodyTooLarge(t *testing.T) {
//	// max_body_size for /null_content_size is 100 bytes — send 200
//	body := bytes.Repeat([]byte("x"), 200)
//
//	req, err := http.NewRequest(http.MethodPost, baseURL+"/null_content_size", bytes.NewReader(body))
//	if err != nil {
//		t.Fatalf("build failed: %v", err)
//	}
//	req.Header.Set("Content-Type", "application/octet-stream")
//
//	resp, err := httpClient.Do(req)
//	if err != nil {
//		t.Fatalf("POST /null_content_size failed: %v", err)
//	}
//	defer resp.Body.Close()
//
//	if resp.StatusCode != http.StatusRequestEntityTooLarge {
//		t.Errorf("oversized body: expected 413, got %d", resp.StatusCode)
//	} else {
//		t.Logf("OK - got 413 for body exceeding max_body_size")
//	}
//}

//func TestCGIBadScript(t *testing.T) {
//	const cgiPath = "/scripts/bad/abort.py"
//
//	req, err := http.NewRequest(http.MethodGet, baseURL+cgiPath, nil)
//	if err != nil {
//		t.Fatalf("build failed: %v", err)
//	}
//
//	resp, err := httpClient.Do(req)
//	if err != nil {
//		t.Fatalf("GET %s failed: %v", cgiPath, err)
//	}
//	defer resp.Body.Close()
//
//	// CGI process crashes (SIGABRT) — server should return 5xx, not hang or crash
//	if resp.StatusCode < 500 || resp.StatusCode > 599 {
//		t.Errorf("abort.py: expected 5xx, got %d", resp.StatusCode)
//	} else {
//		t.Logf("OK - got %d for crashing CGI script", resp.StatusCode)
//	}
//}

//func TestPrematureClose(t *testing.T) {
//	conn, err := net.Dial("tcp", baseAddr)
//	if err != nil {
//		t.Fatalf("dial failed: %v", err)
//	}
//
//	header := "POST /post HTTP/1.1\r\n" +
//		"Host: 127.0.0.1:8084\r\n" +
//		"Content-Type: application/octet-stream\r\n" +
//		"Content-Length: 1048576\r\n" +
//		"\r\n"
//
//	_, err = fmt.Fprint(conn, header)
//	if err != nil {
//		t.Fatalf("failed to send headers: %v", err)
//	}
//
//	_, err = conn.Write(bytes.Repeat([]byte("x"), 128))
//	if err != nil {
//		t.Fatalf("failed to send partial body: %v", err)
//	}
//
//	conn.Close()
//	time.Sleep(100 * time.Millisecond)
//
//	// Health check: just verify server responds to anything at all
//	t.Log("checking server is still responsive after premature close")
//	req, err := http.NewRequest(http.MethodPost, baseURL+"/post", bytes.NewReader([]byte("alive")))
//	if err != nil {
//		t.Fatalf("health check build failed: %v", err)
//	}
//	req.Header.Set("Content-Type", "application/octet-stream")
//
//	resp, err := httpClient.Do(req)
//	if err != nil {
//		t.Fatalf("server did not respond after premature close: %v", err)
//	}
//	defer resp.Body.Close()
//	io.ReadAll(resp.Body)
//
//	if resp.StatusCode == 0 {
//		t.Error("got zero status — server may have crashed")
//	}
//	t.Logf("OK - server alive, responded with %d", resp.StatusCode)
//}
