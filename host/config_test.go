package main

import "testing"

func TestLoadConfig_Defaults(t *testing.T) {
	c := LoadConfig(func(string) string { return "" })
	if c.InnerCmd != "bash ~/.claude/statusline-command.sh" {
		t.Errorf("inner default = %q", c.InnerCmd)
	}
	if c.Baud != 115200 {
		t.Errorf("baud default = %d", c.Baud)
	}
	if c.SerialPath != "" {
		t.Errorf("serial default empty (autodetect later), got %q", c.SerialPath)
	}
}

func TestLoadConfig_EnvOverride(t *testing.T) {
	env := map[string]string{
		"TOKENPULSE_SERIAL": "/dev/cu.usbmodem99",
		"TOKENPULSE_INNER":  "echo hi",
		"TOKENPULSE_BAUD":   "9600",
	}
	c := LoadConfig(func(k string) string { return env[k] })
	if c.SerialPath != "/dev/cu.usbmodem99" || c.InnerCmd != "echo hi" || c.Baud != 9600 {
		t.Errorf("override failed: %+v", c)
	}
}
