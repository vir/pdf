LIBPDFDIR = ../libpdf

all: pdfdig

.SUFFIXES: .so .cxx
.PHONY: clean

.cxx.o:
	$(CXX) -Wall -Wno-write-strings -pipe -g -c -I.. -I${LIBPDFDIR} `wx-config --cxxflags` -o $*.o $<

pdfdig: pdfdig.o MyFrame.o MyTree.o PdfDoc.o PdfExplorerView.o ${LIBPDFDIR}/libpdf.a
	$(LINK.cpp) `wx-config --libs` -lz -o $@ $^
#	strip $@

clean:
	rm -f ${OBJS_ALL} ${BINS} ${DLLS} .depend core *.o

.depend:
	gcc -MM -MG *.cxx >.depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif







