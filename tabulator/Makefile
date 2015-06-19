.SUFFIXES: .a .so .cxx .d
.PHONY : clean docs
#---------------------------------------------------------------------------
SOURCES_LIBTAB = \
	Tabulator.cxx Tabulator_Metafile.cxx Tabulator_Table.cxx Tabulator_Grid.cxx \
	Tabulator_Exporter.cxx Tabulator_TextBlock.cxx
OBJS_LIBTAB = $(SOURCES_LIBTAB:.cxx=.o)

SOURCES_TABULATOR = tabmain.cxx
OBJS_TABULATOR = $(SOURCES_TABULATOR:.cxx=.o)

SOURCES_WXTAB = \
	wxtab.cxx WxTabCanvas.cxx WxTabDocument.cxx WxTabFrame.cxx WxTabTabulator.cxx \
	WxTabBatchExportDlg.cxx PageNumCtrl.cxx
OBJS_WXTAB = $(SOURCES_WXTAB:.cxx=.o)

CXXFLAGS = -Wall -Wno-write-strings -pipe -g -I.. `wx-config --cxxflags`
#	$(CXX) -Wall -pipe -g -c -I${LIBPDFDIR}  -o $*.o $<

all: libtab.a tabulator wxtab

clean:
	rm -f ${OBJS_LIBTAB} ${OBJS_TABULATOR} ${OBJS_WXTAB} libtab.a .depend core

libtab.a: $(OBJS_LIBTAB)
	ar r $@ $?

tabulator: $(OBJS_TABULATOR) ../libpdf/libpdf.a libtab.a
	$(LINK.cpp) -lz  -o $@ $^

wxtab: $(OBJS_WXTAB) ../libpdf/libpdf.a libtab.a
	$(LINK.cpp) `wx-config --libs` -lz  -o $@ $^

%.o: %.cxx
	$(COMPILE.cpp) $<

.depend: Makefile
	gcc -MM -MG -I.. `wx-config --cxxflags` *.cxx > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
else
all: .depend
endif




