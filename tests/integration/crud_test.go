package main

import (
	"testing"
	"net/http"
	"bytes"
	"sync"
	"fmt"
)

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

func TestPostGetDeleteGet(t *testing.T) {
    baseFixtures := []fixture{
        {name: "hello.txt", content: []byte("hello world\n")},
        {name: "foo.html", content: []byte("<html><body>foo</body></html>\n")},
        {name: "bar.json", content: []byte("{\"key\":\"value\"}\n")},
        {name: "baz.md", content: []byte("# baz\n\nsome markdown\n")},
        fixtureFromFile(t, "./data_to_send/pdf_file.pdf"),
    }

    const nConns = 4

    var wg sync.WaitGroup
    wg.Add(nConns)

    for i := 0; i < nConns; i++ {
        i := i
        f := baseFixtures[i%len(baseFixtures)]

        go func() {
            defer wg.Done()
            t.Run(fmt.Sprintf("conn_%d_%s", i, f.name), func(t *testing.T) {
                runFlow(t, i, f)
            })
        }()
    }

    wg.Wait()
}
