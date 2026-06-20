package main

import (
	"bufio"
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

// defaultSettingsPath returns ~/.claude/settings.json.
func defaultSettingsPath() string {
	home, err := os.UserHomeDir()
	if err != nil {
		return ".claude/settings.json"
	}
	return filepath.Join(home, ".claude", "settings.json")
}

// shellSingleQuote wraps s in single quotes, safely escaping embedded quotes.
func shellSingleQuote(s string) string {
	return "'" + strings.ReplaceAll(s, "'", `'\''`) + "'"
}

// buildStatusLineCommand assembles the statusLine command string that registers
// TokenPulse: it sets the serial port + wraps the user's existing status-line
// command as the passthrough inner command, then runs `tokenpulse statusline`.
// serialPath == "" omits the env var (host autodetects at runtime).
// innerCmd == "" omits TOKENPULSE_INNER (host uses its built-in default).
func buildStatusLineCommand(selfPath, serialPath, innerCmd string) string {
	var b strings.Builder
	if serialPath != "" {
		b.WriteString("TOKENPULSE_SERIAL=")
		b.WriteString(serialPath)
		b.WriteString(" ")
	}
	if innerCmd != "" {
		b.WriteString("TOKENPULSE_INNER=")
		b.WriteString(shellSingleQuote(innerCmd))
		b.WriteString(" ")
	}
	if strings.ContainsAny(selfPath, " \t'\"") {
		b.WriteString(shellSingleQuote(selfPath))
	} else {
		b.WriteString(selfPath)
	}
	b.WriteString(" statusline")
	return b.String()
}

// extractStatusLineCommand returns the current statusLine.command from a
// settings.json blob ("" if there is no statusLine).
func extractStatusLineCommand(settings []byte) (string, error) {
	var top map[string]json.RawMessage
	if err := json.Unmarshal(settings, &top); err != nil {
		return "", err
	}
	raw, ok := top["statusLine"]
	if !ok {
		return "", nil
	}
	var sl struct {
		Command string `json:"command"`
	}
	if err := json.Unmarshal(raw, &sl); err != nil {
		return "", err
	}
	return sl.Command, nil
}

// setStatusLineCommand returns settings.json with statusLine replaced by a
// command-type entry running newCmd. Other keys are preserved (semantically);
// the file is re-emitted pretty-printed with top-level keys sorted, so exact
// formatting/key-order changes but the configuration is identical.
func setStatusLineCommand(settings []byte, newCmd string) ([]byte, error) {
	var top map[string]json.RawMessage
	if err := json.Unmarshal(settings, &top); err != nil {
		return nil, err
	}
	sl, err := json.Marshal(map[string]string{"type": "command", "command": newCmd})
	if err != nil {
		return nil, err
	}
	top["statusLine"] = sl
	out, err := json.MarshalIndent(top, "", "  ")
	if err != nil {
		return nil, err
	}
	return append(out, '\n'), nil
}

// isTokenPulseCommand reports whether a statusLine command already runs us.
func isTokenPulseCommand(cmd string) bool {
	return strings.Contains(cmd, "tokenpulse statusline")
}

// runSetup implements `tokenpulse setup`: register into ~/.claude/settings.json.
func runSetup(args []string, stdin *os.File) int {
	fs := flag.NewFlagSet("setup", flag.ContinueOnError)
	yes := fs.Bool("yes", false, "apply without confirmation")
	serial := fs.String("serial", "", "serial device (default: autodetect /dev/cu.usbmodem*)")
	settingsPath := fs.String("settings", defaultSettingsPath(), "path to Claude Code settings.json")
	if err := fs.Parse(args); err != nil {
		return 2
	}

	self, err := os.Executable()
	if err != nil {
		fmt.Fprintln(os.Stderr, "setup: cannot resolve own path:", err)
		return 1
	}

	data, err := os.ReadFile(*settingsPath)
	if err != nil {
		fmt.Fprintf(os.Stderr, "setup: cannot read %s: %v\n", *settingsPath, err)
		fmt.Fprintln(os.Stderr, "Run Claude Code once so it creates the file, or pass --settings.")
		return 1
	}

	prev, err := extractStatusLineCommand(data)
	if err != nil {
		fmt.Fprintf(os.Stderr, "setup: %s is not valid JSON: %v\n", *settingsPath, err)
		return 1
	}
	if isTokenPulseCommand(prev) {
		fmt.Println("Already configured — statusLine already runs tokenpulse. Nothing to do.")
		return 0
	}

	port := *serial
	if port == "" {
		port = autodetectSerial(globFiles)
	}
	if port == "" {
		fmt.Fprintln(os.Stderr, "Note: no TokenPulse serial port found now; the tool will autodetect at runtime once the board is plugged in.")
	}

	newCmd := buildStatusLineCommand(self, port, prev)

	fmt.Println("Will update statusLine in", *settingsPath)
	if prev == "" {
		fmt.Println("  current: (none)")
	} else {
		fmt.Println("  current:", prev)
	}
	fmt.Println("  new:    ", newCmd)
	if prev != "" {
		fmt.Println("(your existing status line is preserved as the passthrough inner command)")
	}

	if !*yes {
		fmt.Print("Apply this change? [y/N] ")
		r := bufio.NewReader(stdin)
		line, _ := r.ReadString('\n')
		switch strings.ToLower(strings.TrimSpace(line)) {
		case "y", "yes":
		default:
			fmt.Println("Aborted.")
			return 1
		}
	}

	if err := os.WriteFile(*settingsPath+".bak", data, 0o644); err != nil {
		fmt.Fprintln(os.Stderr, "setup: failed to write backup:", err)
		return 1
	}
	out, err := setStatusLineCommand(data, newCmd)
	if err != nil {
		fmt.Fprintln(os.Stderr, "setup:", err)
		return 1
	}
	if err := os.WriteFile(*settingsPath, out, 0o644); err != nil {
		fmt.Fprintln(os.Stderr, "setup: failed to write settings:", err)
		return 1
	}

	fmt.Printf("Done. Backup saved to %s.bak (top-level keys were re-sorted).\n", *settingsPath)
	fmt.Println("Claude Code applies it on the next status-line refresh (or restart).")
	return 0
}
