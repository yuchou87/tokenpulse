package main

import "testing"

func TestRunInner_PipesStdinToCommand(t *testing.T) {
	out, err := runInner("cat", []byte("HELLO"))
	if err != nil {
		t.Fatalf("runInner: %v", err)
	}
	if string(out) != "HELLO" {
		t.Errorf("got %q", out)
	}
}
