SOURCES=preload.js
HEADER=preload.h
SED=sed

all: $(SOURCES) $(HEADER)

$(HEADER): $(SOURCES)
	@date '+/* Generated at %a %b %e %Y %H:%M:%S %Z */' > $@
	@/bin/echo -n "const char *preload_js = " >> $@
	$(SED) -e 's/\"/\\"/g' -e 's/^/\"/g' -e 's/$$/\\n\"/g' -e '/^$$/d'  $< >> $@
	@echo ";" >> $@
	cp $@ ../src/NativeJS_$@
