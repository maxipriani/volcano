package auth

import (
	"log/slog"
	"time"
	"volcano_server/internal/database"
)

type User struct {
	IP         string
	Name       string
	ApiKey     string
	Authorized bool
	Privileged bool
}

type AuthResponse struct {
	Success bool   `json:"success"`
	Status  string `json:"status"`
	ApiKey  string `json:"api_key,omitempty"`
}

func findUserByName(userName string) (*User, error) {

	var user *User = &User{Name: userName}
	query := "SELECT ip, api_key, authorized, privileged FROM users WHERE user_name = ?"
	row := database.DB.QueryRow(query, userName)
	err := row.Scan(&user.IP, &user.ApiKey, &user.Authorized, &user.Privileged)

	return user, err
}

func (user *User) createUser() error {
	query := "INSERT INTO users (ip, user_name, api_key, authorized, privileged) VALUES (?, ?, ?, ?, ?)"
	_, err := database.DB.Exec(query, user.IP, user.Name, user.ApiKey, user.Authorized, user.Privileged)
	return err
}

func (user *User) updateIP() error {
	query := "UPDATE users SET ip = ? WHERE user_name = ?"
	_, err := database.DB.Exec(query, user.IP, user.Name)
	return err
}

func (user *User) updateApiKey() error {
	apiKey := generateApiKey()
	query := "UPDATE users SET api_key = ? WHERE user_name = ?"
	_, err := database.DB.Exec(query, apiKey, user.Name)

	if err != nil {
		slog.Error("Failed to update API key", "user", user.Name, "error", err)
		return err
	}

	user.ApiKey = apiKey
	return nil
}

func (user *User) updateLastSeen() error {

	loc, _ := time.LoadLocation("America/Argentina/Buenos_Aires")
	currentTime := time.Now().In(loc)
	query := `UPDATE users SET last_seen = ? WHERE user_name = ?`
	_, err := database.DB.Exec(query, currentTime, user.Name)
	return err
}
