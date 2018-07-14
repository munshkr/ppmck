.PHONY: nesasm ppmckc

ZIP_FILE := dist.zip
BIN_FILES := nesasm pceas ppmckc ppmckc_e
BIN := $(addprefix bin/, $(BIN_FILES))

all: nesasm ppmckc

nesasm:
	$(MAKE) -C src/nesasm

ppmckc:
	$(MAKE) -C src/ppmckc

clean:
	$(MAKE) -C src/nesasm clean
	$(MAKE) -C src/ppmckc clean
	@rm -f $(BIN)
	@rm -rf dist/ dist.zip

dist: all
	@rm -rf dist/
	@mkdir -p dist/bin/
	@cp $(BIN) dist/bin/
	@cp -a nes_include/ dist/
	@cp README.md dist/
	@cd dist/ && zip -r ../$(ZIP_FILE) *
