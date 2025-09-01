package auth

import (
	"crypto/rand"
	"encoding/hex"
	"log/slog"
	"net/http"
)

func generateApiKey() string {
	bytes := make([]byte, 32)
	_, err := rand.Read(bytes)

	if err != nil {
		slog.Error("Failed to generate API key", "error", err)
		return ""
	}

	return hex.EncodeToString(bytes)
}

func getUserIpFromRequest(r *http.Request) string {

	ip := r.Header.Get("CF-Connecting-IP")
	if ip != "" {
		return ip
	}

	return ""
}

func getUserNameFromRequest(r *http.Request) string {
	return r.FormValue("user_name")
}

func getApiKeyFromRequest(r *http.Request) string {
	return r.FormValue("api_key")
}
