package main

import (
	"path/filepath"
	"strings"

	"go.bug.st/serial"
)

// tokenPulseSerialHint matches the macOS device node of our board, which is
// named cu.usbmodem<serial> where the firmware's USB serial string is "TP_…".
const tokenPulseSerialHint = "TP_"

func autodetectSerial(glob func(string) ([]string, error)) string {
	matches, err := glob("/dev/cu.usbmodem*")
	if err != nil || len(matches) == 0 {
		return ""
	}
	// Prefer the TokenPulse board if present, so a second usbmodem device
	// (another board, or the ROM USB-JTAG port in download mode) isn't grabbed.
	for _, m := range matches {
		if strings.Contains(strings.ToUpper(m), tokenPulseSerialHint) {
			return m
		}
	}
	return matches[0]
}

type serialSink struct {
	port serial.Port
}

func newSerialSink(path string, baud int) (*serialSink, error) {
	p, err := serial.Open(path, &serial.Mode{BaudRate: baud})
	if err != nil {
		return nil, err
	}
	return &serialSink{port: p}, nil
}

func (s *serialSink) Send(line []byte) error {
	_, err := s.port.Write(line)
	return err
}

// 供 main 用 filepath.Glob 适配 autodetectSerial 的签名
func globFiles(pat string) ([]string, error) { return filepath.Glob(pat) }
