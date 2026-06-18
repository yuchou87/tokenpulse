package main

import "io"

type SnapshotSink interface {
	Send(line []byte) error
}

type InnerCmd func(stdin []byte) ([]byte, error)

func Run(stdin io.Reader, stdout io.Writer, sink SnapshotSink, inner InnerCmd, nowUnix int64) error {
	raw, err := io.ReadAll(stdin)
	if err != nil {
		raw = nil
	}
	// side effect:推板子(失败忽略,绝不影响 stdout)
	if in, perr := ParseInput(raw); perr == nil {
		if line, lerr := BuildSnapshot(in, nowUnix).Line(); lerr == nil {
			_ = sink.Send(line)
		}
	}
	// 透传内层 statusline 到 stdout
	if innerOut, ierr := inner(raw); ierr == nil {
		if _, werr := stdout.Write(innerOut); werr != nil {
			return werr
		}
	}
	return nil
}
