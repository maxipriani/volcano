package auth

import (
	"log/slog"
	"net/http"
	"volcano_server/internal/config"
)

var globalConfig *config.Config

func SetConfig(cfg *config.Config) {
	globalConfig = cfg
}

func ValidateToken(next http.HandlerFunc) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		clientToken := r.FormValue("token")
		if clientToken != globalConfig.AuthToken {
			slog.Warn("Invalid token attempt",
				"ip", getUserIpFromRequest(r),
				"user_agent", r.UserAgent())
			http.NotFound(w, r)
			return
		}
		next.ServeHTTP(w, r)
	}
}

func RequirePOST(next http.HandlerFunc) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		if r.Method != http.MethodPost {
			slog.Warn("Invalid method attempt",
				"method", r.Method,
				"endpoint", r.URL.Path,
				"ip", getUserIpFromRequest(r),
			)
			http.NotFound(w, r)
			return
		}
		next.ServeHTTP(w, r)
	}
}
