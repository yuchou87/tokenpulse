package main

import "strconv"

type Config struct {
	SerialPath string
	InnerCmd   string
	Baud       int
}

func LoadConfig(getenv func(string) string) Config {
	c := Config{
		SerialPath: getenv("TOKENPULSE_SERIAL"),
		InnerCmd:   getenv("TOKENPULSE_INNER"),
		Baud:       115200,
	}
	if c.InnerCmd == "" {
		c.InnerCmd = "bash ~/.claude/statusline-command.sh"
	}
	if v := getenv("TOKENPULSE_BAUD"); v != "" {
		if n, err := strconv.Atoi(v); err == nil {
			c.Baud = n
		}
	}
	return c
}
