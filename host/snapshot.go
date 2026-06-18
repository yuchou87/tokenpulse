package main

import "encoding/json"

type StatusLineInput struct {
	RateLimits struct {
		FiveHour struct {
			UsedPercentage int   `json:"used_percentage"`
			ResetsAt       int64 `json:"resets_at"`
		} `json:"five_hour"`
		SevenDay struct {
			UsedPercentage int   `json:"used_percentage"`
			ResetsAt       int64 `json:"resets_at"`
		} `json:"seven_day"`
	} `json:"rate_limits"`
	Model struct {
		DisplayName string `json:"display_name"`
	} `json:"model"`
	Cost struct {
		TotalCostUSD float64 `json:"total_cost_usd"`
	} `json:"cost"`
}

type Provider struct {
	ID         string  `json:"id"`
	FivePct    int     `json:"five_pct"`
	FiveResetS int64   `json:"five_reset_s"`
	WeekPct    int     `json:"week_pct"`
	SessionUSD float64 `json:"session_usd"`
}

type Snapshot struct {
	Ts        int64      `json:"ts"`
	State     string     `json:"state"`
	Providers []Provider `json:"providers"`
}

func ParseInput(b []byte) (StatusLineInput, error) {
	var in StatusLineInput
	err := json.Unmarshal(b, &in)
	return in, err
}

func BuildSnapshot(in StatusLineInput, nowUnix int64) Snapshot {
	resetS := in.RateLimits.FiveHour.ResetsAt - nowUnix
	if resetS < 0 {
		resetS = 0
	}
	return Snapshot{
		Ts:    nowUnix,
		State: "running",
		Providers: []Provider{{
			ID:         "claude",
			FivePct:    in.RateLimits.FiveHour.UsedPercentage,
			FiveResetS: resetS,
			WeekPct:    in.RateLimits.SevenDay.UsedPercentage,
			SessionUSD: in.Cost.TotalCostUSD,
		}},
	}
}

func (s Snapshot) Line() ([]byte, error) {
	b, err := json.Marshal(s)
	if err != nil {
		return nil, err
	}
	return append(b, '\n'), nil
}
