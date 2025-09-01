package logger

import (
	"fmt"
	"log/slog"
	"os"
	"time"
)

var logger *slog.Logger

func Init() {

	logDir := os.Getenv("LOG_DIR")

	if logDir == "" {
		logDir, _ = os.UserHomeDir()
	}

	err := os.MkdirAll(logDir, 0755)
	if err != nil {
		fmt.Printf("Failed to create log directory: %v\n", err)
		os.Exit(1)
	}

	timestamp := time.Now().Format("2006-01-02")
	logFile, err := os.OpenFile(
		fmt.Sprintf(logDir+"/volcano_server-%s.log", timestamp), os.O_CREATE|os.O_WRONLY|os.O_TRUNC, 0644)

	if err != nil {
		fmt.Printf("Failed to open log file: %v\n", err)
		os.Exit(1)
	}

	logger = slog.New(slog.NewTextHandler(logFile, &slog.HandlerOptions{

		ReplaceAttr: func(groups []string, a slog.Attr) slog.Attr {
			if a.Key == slog.TimeKey {
				a.Value = slog.StringValue(a.Value.Time().Format("02/01/2006 15:04:05"))
			}
			return a
		},
	}))

	logger.Info("Logger initialized", "log_file", fmt.Sprintf(logDir+"/volcano_server-%s.log", timestamp))
}

func Logger() *slog.Logger {
	return logger
}
