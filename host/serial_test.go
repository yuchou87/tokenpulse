package main

import "testing"

func TestAutodetectSerial_PicksFirstUsbmodem(t *testing.T) {
	glob := func(pat string) ([]string, error) {
		return []string{"/dev/cu.usbmodem1101", "/dev/cu.usbmodem2202"}, nil
	}
	if got := autodetectSerial(glob); got != "/dev/cu.usbmodem1101" {
		t.Errorf("got %q", got)
	}
}

func TestAutodetectSerial_NoneFound(t *testing.T) {
	glob := func(pat string) ([]string, error) { return nil, nil }
	if got := autodetectSerial(glob); got != "" {
		t.Errorf("got %q, want empty", got)
	}
}
