SOURCES=private.bin
HEADER=private_bin.h
XXD = xxd
DIR_BIN = ../../framework/dist/private

all: $(SOURCES) $(HEADER)

clean:
	rm $(SOURCES) $(HEADER)

$(SOURCES):
	./dir2nvfs $(DIR_BIN) > $@

$(HEADER): $(SOURCES)
	@date '+/* Generated at %a %b %e %Y %H:%M:%S %Z */' > $@
	$(XXD) -i $< >> $@
