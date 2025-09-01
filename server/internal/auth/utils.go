package auth

import (
	"crypto/rand"
	"encoding/hex"
	"net/http"
	"volcano_server/internal/logger"
)

func generateApiKey() string {
	bytes := make([]byte, 32)
	_, err := rand.Read(bytes)

	if err != nil {
		logger.Logger().Error("Failed to generate API key", "error", err)
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
