.SUFFIXES: .a .so .cxx .d
.PHONY : clean docs
#---------------------------------------------------------------------------
SOURCES = tabmain.cxx \
	Tabulator.cxx Tabulator_Metafile.cxx Tabulator_Table.cxx Tabulator_Grid.cxx \
	Tabulator_Exporter.cxx Tabulator_TextBlock.cxx
OBJS = $(SOURCES:.cxx=.o)

CXXFLAGS = -Wall -pipe -g -I..

all: $(OBJS) tabulator

clean:
	rm -f ${OBJS} .depend core

tabulator: $(OBJS) ../libpdf/libpdf.a
	$(LINK.cpp) -lz  -o $@ $^

%.o: %.cxx
	$(COMPILE.cpp) $<

.depend:
	gcc -MM -MG *.cxx >.depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif



