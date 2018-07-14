.PHONY: nesasm ppmckc

BIN := nesasm pceas ppmckc ppmckc_e

all: nesasm ppmckc

nesasm:
	$(MAKE) -C src/nesasm

ppmckc:
	$(MAKE) -C src/ppmckc

clean:
	$(MAKE) -C src/nesasm clean
	$(MAKE) -C src/ppmckc clean
	rm -f $(addprefix bin/, $(BIN))
