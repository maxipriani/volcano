package auth

import (
	"net/http"
	"volcano_server/internal/config"
	"volcano_server/internal/logger"
)

var globalConfig *config.Config

func SetConfig(cfg *config.Config) {
	globalConfig = cfg
}

func ValidateToken(next http.HandlerFunc) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		clientToken := r.FormValue("token")
		if clientToken != globalConfig.AuthToken {
			logger.Logger().Warn("Invalid token attempt",
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
			logger.Logger().Warn("Invalid method attempt",
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
