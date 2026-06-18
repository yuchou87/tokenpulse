package main

import "testing"

func TestParseInput_RealSchema(t *testing.T) {
	raw := []byte(`{"rate_limits":{"five_hour":{"used_percentage":13,"resets_at":1781715000},"seven_day":{"used_percentage":26,"resets_at":1781888400}},"model":{"display_name":"Sonnet 4.6"},"cost":{"total_cost_usd":0.286}}`)
	in, err := ParseInput(raw)
	if err != nil {
		t.Fatalf("parse: %v", err)
	}
	if in.RateLimits.FiveHour.UsedPercentage != 13 {
		t.Errorf("five pct = %d, want 13", in.RateLimits.FiveHour.UsedPercentage)
	}
	if in.RateLimits.FiveHour.ResetsAt != 1781715000 {
		t.Errorf("five reset = %d", in.RateLimits.FiveHour.ResetsAt)
	}
	if in.Cost.TotalCostUSD != 0.286 {
		t.Errorf("cost = %v", in.Cost.TotalCostUSD)
	}
}

func TestBuildSnapshot_ResetClampAndFields(t *testing.T) {
	in, _ := ParseInput([]byte(`{"rate_limits":{"five_hour":{"used_percentage":13,"resets_at":1000},"seven_day":{"used_percentage":26,"resets_at":1000}},"cost":{"total_cost_usd":0.29}}`))
	// now 后于 five reset(1000) → clamp 0;7d reset(1000) 同样
	snap := BuildSnapshot(in, 2000)
	p := snap.Providers[0]
	if p.ID != "claude" || p.FivePct != 13 || p.WeekPct != 26 {
		t.Errorf("provider = %+v", p)
	}
	if p.FiveResetS != 0 {
		t.Errorf("reset_s = %d, want 0 (clamped)", p.FiveResetS)
	}
	if p.SessionUSD != 0.29 {
		t.Errorf("session_usd = %v", p.SessionUSD)
	}
	if snap.Ts != 2000 || snap.State != "running" {
		t.Errorf("ts/state = %d/%s", snap.Ts, snap.State)
	}
}

func TestBuildSnapshot_PositiveReset(t *testing.T) {
	in, _ := ParseInput([]byte(`{"rate_limits":{"five_hour":{"used_percentage":1,"resets_at":5000},"seven_day":{"used_percentage":2,"resets_at":9000}}}`))
	snap := BuildSnapshot(in, 1000)
	if snap.Providers[0].FiveResetS != 4000 {
		t.Errorf("reset_s = %d, want 4000", snap.Providers[0].FiveResetS)
	}
}

func TestSnapshotLine_EndsWithNewline(t *testing.T) {
	in, _ := ParseInput([]byte(`{"rate_limits":{"five_hour":{"used_percentage":1,"resets_at":5000}}}`))
	line, err := BuildSnapshot(in, 1000).Line()
	if err != nil {
		t.Fatal(err)
	}
	if len(line) == 0 || line[len(line)-1] != '\n' {
		t.Errorf("line must end with newline: %q", line)
	}
}
