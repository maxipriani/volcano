package config

import (
	"os"
)

type Config struct {
	ServerAddress string
	DBUser        string
	DBPassword    string
	DBName        string
	AuthToken     string
	VolcanoLogDir string
}

func Load() *Config {

	return &Config{
		ServerAddress: os.Getenv("SERVER_ADDRESS"),
		DBUser:        os.Getenv("DB_USER"),
		DBPassword:    os.Getenv("DB_PASSWORD"),
		DBName:        os.Getenv("DB_NAME"),
		AuthToken:     os.Getenv("AUTH_TOKEN"),
		VolcanoLogDir: os.Getenv("VOLCANO_LOG_DIR"),
	}
}
