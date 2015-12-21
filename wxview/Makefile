LIBPDFDIR = ../libpdf

all: wxview

.SUFFIXES: .so .cxx
.PHONY: clean

.cxx.o:
	$(CXX) -Wall -pipe -g -c -I${LIBPDFDIR} `wx-config --cxxflags` -o $*.o $<

wxview: wxview.o MyFrame.o MyCanvas.o MyDocument.o PagePaintHelper.o ${LIBPDFDIR}/libpdf.a
	$(LINK.cpp) `wx-config --libs` -lz -o $@ $^
#	strip $@

clean:
	rm -f ${OBJS_ALL} ${BINS} ${DLLS} .depend core *.o

.depend:
	gcc -MM -MG *.cxx >.depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif







