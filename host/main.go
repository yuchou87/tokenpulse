package main

import (
	"bytes"
	"os"
	"os/exec"
	"time"
)

func runInner(cmdline string, stdin []byte) ([]byte, error) {
	cmd := exec.Command("bash", "-c", cmdline)
	cmd.Stdin = bytes.NewReader(stdin)
	var out bytes.Buffer
	cmd.Stdout = &out
	err := cmd.Run()
	return out.Bytes(), err
}

func main() {
	if len(os.Args) < 2 || os.Args[1] != "statusline" {
		// 未知子命令:静默退出,绝不污染 stdout
		os.Exit(0)
	}
	cfg := LoadConfig(os.Getenv)
	path := cfg.SerialPath
	if path == "" {
		path = autodetectSerial(globFiles)
	}
	var sink SnapshotSink = noopSink{}
	if path != "" {
		if ss, err := newSerialSink(path, cfg.Baud); err == nil {
			defer ss.port.Close()
			sink = ss
		}
	}
	inner := func(stdin []byte) ([]byte, error) { return runInner(cfg.InnerCmd, stdin) }
	_ = Run(os.Stdin, os.Stdout, sink, inner, time.Now().Unix())
	os.Exit(0)
}

type noopSink struct{}

func (noopSink) Send([]byte) error { return nil }
