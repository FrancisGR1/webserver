package main

import (
	"io"
	"net/http"
	"testing"
)

func TestGET(t *testing.T) {
	tests := []struct {
		name string
		path string
		expectedStatus int
		expectedBody string
	}{
		{
			name:           "existing html file",
			path:           "/get/hello_world.html",
			expectedStatus: 200,
			expectedBody:   "",
		},
		{
			name:           "missing file returns 404",
			path:           "/get/nonexistent.html",
			expectedStatus: 404,
			expectedBody:   "",
		},
		{
			name:           "root path",
			path:           "/",
			expectedStatus: 200,
			expectedBody:   "",
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			resp, err := http.Get("http://127.0.0.1:8080" + tt.path)
			if err != nil {
				t.Fatalf("Request failed: %v", err)
			}
			if resp.StatusCode != tt.expectedStatus {
				t.Errorf("expected status %d, got %d", tt.expectedStatus, resp.StatusCode)
			}
			if tt.expectedBody != "" {
				body, err := io.ReadAll(resp.Body)
				if err != nil {
					t.Fatalf("failed to read body: %v", err)
				}
				if string(body) != tt.expectedBody {
					t.Errorf("expected body %q, got %q", tt.expectedBody, string(body))
				}
			}
		})
	}
}
