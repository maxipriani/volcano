package main

import (
	"fmt"
	"net/http"
	"volcano_server/internal/auth"
	"volcano_server/internal/config"
	"volcano_server/internal/database"
	"volcano_server/internal/logger"

	"github.com/joho/godotenv"
)

func main() {

	err := godotenv.Load()

	if err != nil {
		fmt.Printf("Error loading .env file: %v\n", err)
		return
	}

	logger.Init()

	cfg := config.Load()
	auth.SetConfig(cfg)

	db, err := database.Init(cfg)

	if err != nil {
		logger.Logger().Error("Failed to initialize the database", "error", err)
		return
	}

	defer db.Close()

	setUpRoutes()
	startServer(cfg.ServerAddress)
}

func setUpRoutes() {
	http.HandleFunc("/health", auth.HealthHandler)
	http.HandleFunc("/auth/login", auth.RequirePOST(auth.ValidateToken(auth.LoginHandler)))
	http.HandleFunc("/auth/verify", auth.RequirePOST(auth.ValidateToken(auth.ApiKeyVerificationHandler)))
	http.HandleFunc("/auth/logout", auth.RequirePOST(auth.ValidateToken(auth.LogoutHandler)))

	logger.Logger().Info("Routes configured")
}

func startServer(address string) {
	logger.Logger().Info("Starting HTTP server", "address", address)
	fmt.Println("=== Server ON ===")

	err := http.ListenAndServe(address, nil)
	if err != nil {
		logger.Logger().Error("Failed to start server", "error", err)
		return
	}

}
