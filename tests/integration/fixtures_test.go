package main

import (
	"net/http"
)

const baseAddr = "127.0.0.1:8091"
const baseURL = "http://" + baseAddr

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

