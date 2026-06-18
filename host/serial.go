package main

import (
	"path/filepath"

	"go.bug.st/serial"
)

func autodetectSerial(glob func(string) ([]string, error)) string {
	matches, err := glob("/dev/cu.usbmodem*")
	if err != nil || len(matches) == 0 {
		return ""
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
