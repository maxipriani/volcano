package database

import (
	"database/sql"
	"fmt"
	"volcano_server/internal/config"
	"volcano_server/internal/logger"

	_ "github.com/go-sql-driver/mysql"
)

var (
	DB *sql.DB
)

func Init(cfg *config.Config) (*sql.DB, error) {
	var err error

	connStr := fmt.Sprintf("%s:%s@tcp(localhost:3306)/%s", cfg.DBUser, cfg.DBPassword, cfg.DBName)

	DB, err = sql.Open("mysql", connStr)

	if err != nil {
		logger.Logger().Error("Failed to connect to database", "error", err)
		return nil, err
	}

	err = DB.Ping()
	if err != nil {
		logger.Logger().Error("Failed to ping database", "error", err)
		return nil, err
	}

	logger.Logger().Info("Database connection established", "database", cfg.DBName, "user", cfg.DBUser)
	return DB, nil
}
