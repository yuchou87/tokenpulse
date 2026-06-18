package main

import (
	"bytes"
	"errors"
	"testing"
)

type fakeSink struct {
	lines [][]byte
	err   error
}

func (f *fakeSink) Send(line []byte) error {
	f.lines = append(f.lines, line)
	return f.err
}

const sampleJSON = `{"rate_limits":{"five_hour":{"used_percentage":13,"resets_at":5000},"seven_day":{"used_percentage":26,"resets_at":9000}},"cost":{"total_cost_usd":0.29}}`

func TestRun_PushesSnapshotAndPassesThrough(t *testing.T) {
	sink := &fakeSink{}
	var out bytes.Buffer
	inner := func(stdin []byte) ([]byte, error) {
		return []byte("STATUSLINE-OUTPUT"), nil // 模拟内层 statusline
	}
	err := Run(bytes.NewReader([]byte(sampleJSON)), &out, sink, inner, 1000)
	if err != nil {
		t.Fatalf("run: %v", err)
	}
	if len(sink.lines) != 1 {
		t.Fatalf("sink got %d lines, want 1", len(sink.lines))
	}
	if !bytes.Contains(sink.lines[0], []byte(`"five_pct":13`)) {
		t.Errorf("snapshot line missing five_pct: %s", sink.lines[0])
	}
	if out.String() != "STATUSLINE-OUTPUT" {
		t.Errorf("stdout = %q, want inner output only", out.String())
	}
}

func TestRun_SinkErrorDoesNotAffectStdout(t *testing.T) {
	sink := &fakeSink{err: errors.New("serial down")}
	var out bytes.Buffer
	inner := func(stdin []byte) ([]byte, error) { return []byte("OK"), nil }
	if err := Run(bytes.NewReader([]byte(sampleJSON)), &out, sink, inner, 1000); err != nil {
		t.Fatalf("run should swallow sink error: %v", err)
	}
	if out.String() != "OK" {
		t.Errorf("stdout = %q", out.String())
	}
}

func TestRun_BadJSONStillPassesThrough(t *testing.T) {
	sink := &fakeSink{}
	var out bytes.Buffer
	inner := func(stdin []byte) ([]byte, error) { return []byte("OK"), nil }
	if err := Run(bytes.NewReader([]byte("not json")), &out, sink, inner, 1000); err != nil {
		t.Fatalf("run: %v", err)
	}
	if len(sink.lines) != 0 {
		t.Errorf("bad json should not push, got %d", len(sink.lines))
	}
	if out.String() != "OK" {
		t.Errorf("stdout = %q, passthrough must still happen", out.String())
	}
}
