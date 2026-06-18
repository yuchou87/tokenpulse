SHELL := /bin/bash

# ESP-IDF activation script — override if yours lives elsewhere:
#   make fw-43c IDF_ACTIVATE=/path/to/activate_idf_vX.Y.Z.sh
IDF_ACTIVATE ?= $(HOME)/.espressif/tools/activate_idf_v6.0.1.sh

# 4.3C build needs a separate build dir + sdkconfig fragment (selects the board)
FW_43C_FLAGS := -B build_43c -DSDKCONFIG=sdkconfig.43c \
                -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.43c.defaults"

# Optional serial port for flash targets: make fw-flash-147b PORT=/dev/cu.usbmodemXXX
PORT_ARG := $(if $(PORT),-p $(PORT),)

.PHONY: help \
        host host-test host-install \
        fw-147b fw-43c fw-flash-147b fw-flash-43c fw-clean \
        clean release release-snapshot

help: ## Show this help
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) \
		| awk 'BEGIN{FS=":.*?## "}{printf "  \033[36m%-16s\033[0m %s\n", $$1, $$2}'

## ---------------- host (Go) ----------------

host: ## Build host binary -> host/tokenpulse
	cd host && go build -o tokenpulse .

host-test: ## Run host tests
	cd host && go test ./...

host-install: ## go install the host tool onto $GOBIN/$GOPATH/bin
	cd host && go install .

## ---------------- firmware (ESP-IDF) ----------------

fw-147b: ## Build firmware for ESP32-S3-LCD-1.47B (default board)
	source $(IDF_ACTIVATE) && cd firmware && idf.py build

fw-43c: ## Build firmware for ESP32-S3-Touch-LCD-4.3C
	source $(IDF_ACTIVATE) && cd firmware && idf.py $(FW_43C_FLAGS) build

fw-flash-147b: ## Flash + monitor 1.47B (PORT=... optional)
	source $(IDF_ACTIVATE) && cd firmware && idf.py $(PORT_ARG) flash monitor

fw-flash-43c: ## Flash + monitor 4.3C (PORT=... optional)
	source $(IDF_ACTIVATE) && cd firmware && idf.py $(FW_43C_FLAGS) $(PORT_ARG) flash monitor

fw-clean: ## Remove firmware build dirs
	rm -rf firmware/build firmware/build_43c

## ---------------- release (host, goreleaser) ----------------

release-snapshot: ## Local goreleaser dry-run (no publish, artifacts in dist/)
	goreleaser release --snapshot --clean

release: ## goreleaser release — needs a pushed git tag + GITHUB_TOKEN
	goreleaser release --clean

## ---------------- misc ----------------

clean: fw-clean ## Clean firmware builds + host binary + dist/
	rm -f host/tokenpulse
	rm -rf dist
