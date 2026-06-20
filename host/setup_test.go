package main

import (
	"encoding/json"
	"strings"
	"testing"
)

func TestBuildStatusLineCommand_Full(t *testing.T) {
	got := buildStatusLineCommand("/Users/x/go/bin/tokenpulse", "/dev/cu.usbmodemTP_00011", "bash /Users/x/.claude/statusline-command.sh")
	want := "TOKENPULSE_SERIAL=/dev/cu.usbmodemTP_00011 TOKENPULSE_INNER='bash /Users/x/.claude/statusline-command.sh' /Users/x/go/bin/tokenpulse statusline"
	if got != want {
		t.Fatalf("got %q want %q", got, want)
	}
}

func TestBuildStatusLineCommand_NoSerialNoInner(t *testing.T) {
	got := buildStatusLineCommand("/usr/local/bin/tokenpulse", "", "")
	want := "/usr/local/bin/tokenpulse statusline"
	if got != want {
		t.Fatalf("got %q want %q", got, want)
	}
}

func TestShellSingleQuote_EscapesQuote(t *testing.T) {
	got := buildStatusLineCommand("/bin/tp", "", `echo 'hi'`)
	// inner with single quotes must be safely escaped
	if !strings.Contains(got, `TOKENPULSE_INNER='echo '\''hi'\'''`) {
		t.Fatalf("quote not escaped: %q", got)
	}
}

func TestBuildStatusLineCommand_QuotesSelfPathWithSpaces(t *testing.T) {
	got := buildStatusLineCommand("/Users/My Apps/tokenpulse", "", "")
	want := "'/Users/My Apps/tokenpulse' statusline"
	if got != want {
		t.Fatalf("got %q want %q", got, want)
	}
}

func TestExtractStatusLineCommand(t *testing.T) {
	s := []byte(`{"foo":1,"statusLine":{"type":"command","command":"bash x.sh"}}`)
	got, err := extractStatusLineCommand(s)
	if err != nil {
		t.Fatal(err)
	}
	if got != "bash x.sh" {
		t.Fatalf("got %q", got)
	}
}

func TestExtractStatusLineCommand_None(t *testing.T) {
	got, err := extractStatusLineCommand([]byte(`{"foo":1}`))
	if err != nil || got != "" {
		t.Fatalf("got %q err %v", got, err)
	}
}

func TestSetStatusLineCommand_PreservesOtherKeys(t *testing.T) {
	in := []byte(`{"permissions":{"allow":["Read"]},"statusLine":{"type":"command","command":"old"}}`)
	out, err := setStatusLineCommand(in, "NEW statusline")
	if err != nil {
		t.Fatal(err)
	}
	// statusLine updated
	cmd, _ := extractStatusLineCommand(out)
	if cmd != "NEW statusline" {
		t.Fatalf("command not updated: %q", cmd)
	}
	// other keys preserved (semantically; re-indentation is expected)
	var top map[string]json.RawMessage
	if err := json.Unmarshal(out, &top); err != nil {
		t.Fatal(err)
	}
	if _, ok := top["permissions"]; !ok {
		t.Fatal("permissions key lost")
	}
	var perms struct {
		Allow []string `json:"allow"`
	}
	if err := json.Unmarshal(top["permissions"], &perms); err != nil {
		t.Fatal(err)
	}
	if len(perms.Allow) != 1 || perms.Allow[0] != "Read" {
		t.Fatalf("permissions value changed: %v", perms.Allow)
	}
}

func TestSetStatusLineCommand_AddsWhenMissing(t *testing.T) {
	out, err := setStatusLineCommand([]byte(`{"foo":1}`), "C statusline")
	if err != nil {
		t.Fatal(err)
	}
	cmd, _ := extractStatusLineCommand(out)
	if cmd != "C statusline" {
		t.Fatalf("got %q", cmd)
	}
}

func TestIsTokenPulseCommand(t *testing.T) {
	if !isTokenPulseCommand("TOKENPULSE_SERIAL=x /go/bin/tokenpulse statusline") {
		t.Fatal("should detect existing tokenpulse command")
	}
	if isTokenPulseCommand("bash statusline-command.sh") {
		t.Fatal("false positive")
	}
}

func TestAutodetectSerial_PrefersTokenPulse(t *testing.T) {
	glob := func(string) ([]string, error) {
		return []string{"/dev/cu.usbmodem1101", "/dev/cu.usbmodemTP_00011"}, nil
	}
	if got := autodetectSerial(glob); got != "/dev/cu.usbmodemTP_00011" {
		t.Fatalf("got %q, want TokenPulse device", got)
	}
}

func TestAutodetectSerial_FallbackFirst(t *testing.T) {
	glob := func(string) ([]string, error) {
		return []string{"/dev/cu.usbmodemABC", "/dev/cu.usbmodemXYZ"}, nil
	}
	if got := autodetectSerial(glob); got != "/dev/cu.usbmodemABC" {
		t.Fatalf("got %q, want first", got)
	}
}

func TestAutodetectSerial_Empty(t *testing.T) {
	glob := func(string) ([]string, error) { return nil, nil }
	if got := autodetectSerial(glob); got != "" {
		t.Fatalf("got %q, want empty", got)
	}
}
