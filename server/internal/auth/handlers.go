package auth

import (
	"database/sql"
	"encoding/json"
	"fmt"
	"net/http"
	"volcano_server/internal/logger"
)

func LogoutHandler(w http.ResponseWriter, r *http.Request) {

	requestUser := User{
		Name: getUserNameFromRequest(r),
		IP:   getUserIpFromRequest(r),
	}

	logger.Logger().Info("User logout attempt", "user", requestUser.Name, "ip", requestUser.IP)
	err := requestUser.updateLastSeen()

	if err != nil {
		logger.Logger().Error("Failed to update last seen on logout", "user", requestUser.Name, "error", err)
	}

}

func LoginHandler(w http.ResponseWriter, r *http.Request) {

	requestUser := User{
		Name: getUserNameFromRequest(r),
		IP:   getUserIpFromRequest(r),
	}

	user, err := findUserByName(requestUser.Name)

	if user.IP != requestUser.IP {
		user.IP = requestUser.IP
		user.updateIP()
	}

	if err == sql.ErrNoRows {
		newUser := &User{
			Name:       requestUser.Name,
			ApiKey:     "",
			Authorized: false,
			Privileged: false,
			IP:         requestUser.IP,
		}

		loadErr := newUser.createUser()

		if loadErr != nil {
			logger.Logger().Error("Failed to create user", "user", newUser.Name, "error", loadErr)
			response := AuthResponse{Success: false, Status: "database_error"}
			sendJSONResponse(w, response)
			return
		}

		logger.Logger().Info("New user created", "user", newUser.Name, "ip", newUser.IP)
		response := AuthResponse{Success: false, Status: "pending_authorization"}
		sendJSONResponse(w, response)
		return
	}

	if err != nil {
		logger.Logger().Error("Failed to find user in database", "user", requestUser.Name, "error", err)
		response := AuthResponse{Success: false, Status: "database_error"}
		sendJSONResponse(w, response)
		return
	}

	if user.Privileged {

		if user.IP != requestUser.IP {
			user.IP = requestUser.IP
			err := user.updateIP()

			if err != nil {
				logger.Logger().Error("Failed to update IP", "user", user.Name, "error", err)
			}

		}

		logger.Logger().Info("Privileged user login", "user", user.Name, "ip", user.IP)
		response := AuthResponse{Success: true, Status: "privileged_user"}
		sendJSONResponse(w, response)
		return
	}

	if !user.Authorized {
		logger.Logger().Warn("Unauthorized user access attempt", "user", user.Name, "ip", requestUser.IP)
		response := AuthResponse{Success: false, Status: "unauthorized_user"}
		sendJSONResponse(w, response)
		return
	}

	if user.Authorized && user.ApiKey == "" {

		err := user.updateApiKey()

		if err != nil {
			logger.Logger().Error("Failed to generate API key", "user", user.Name, "error", err)
			response := AuthResponse{Success: false, Status: "error"}
			sendJSONResponse(w, response)
			return
		}

		logger.Logger().Info("API key generated", "user", user.Name)
		response := AuthResponse{Success: true, Status: "api_key_generated", ApiKey: user.ApiKey}
		sendJSONResponse(w, response)
		return
	}

	logger.Logger().Info("Login attempt", "user", user.Name, "ip", user.IP)
	response := AuthResponse{Success: true, Status: "authorized", ApiKey: user.ApiKey}
	sendJSONResponse(w, response)
}

func ApiKeyVerificationHandler(w http.ResponseWriter, r *http.Request) {

	requestUser := &User{
		Name:   getUserNameFromRequest(r),
		ApiKey: getApiKeyFromRequest(r),
	}

	user, err := findUserByName(requestUser.Name)

	if err != nil {
		logger.Logger().Error("Failed to find user during api key verification", "user", requestUser.Name, "error", err)
		response := AuthResponse{Success: false, Status: "invalid_user"}
		sendJSONResponse(w, response)
		return
	}

	if user.ApiKey != requestUser.ApiKey {
		response := AuthResponse{Success: false, Status: "invalid_api_key"}
		sendJSONResponse(w, response)
		return
	}

	logger.Logger().Info("Successful login", "user", user.Name)
	response := AuthResponse{Success: true, Status: "valid_api_key"}
	sendJSONResponse(w, response)
}

func HealthHandler(w http.ResponseWriter, r *http.Request) {
	w.WriteHeader(http.StatusOK)
	fmt.Fprint(w, "== Server ON ==\n")
}

func sendJSONResponse(w http.ResponseWriter, response AuthResponse) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(response)
}
