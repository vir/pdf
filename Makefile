# my makefile

#include MKCONF

VERSION = 0.0.1
TARNAME = proga-${VERSION}.tar.gz
OBJS_MAIN = main.o libpdf/libpdf.a utf8.o
OBJS_ALL = ${OBJS_MAIN}
SUBDIRS = libpdf tests #docs

INCLUDEDIR = -Ilibpdf

#CC = /usr/local/gcc-3.4.3/bin/gcc
#CXX = /usr/local/gcc-3.4.3/bin/g++
#CXXFLAGS += " -L/usr/local/gcc-3.4.3/lib -L/usr/local/gcc-3.4.3/lib/gcc/i686-pc-linux-gnu/3.4.3/"
#LDFLAGS = -L/usr/local/gcc-3.4.3/lib/gcc/i686-pc-linux-gnu/3.4.3/

all: subdirs main dump_object page_metafile tabulator2 page_to_html

.SUFFIXES: .so .cxx

.cxx.o:
	$(CXX) -Wall -pipe -g -c ${INCLUDEDIR} -o $*.o $<

ccmain: subdirs ${OBJS_MAIN}
	ccmalloc $(LINK.cpp) -g -lz -o $@ ${OBJS_MAIN}

main: ${OBJS_MAIN}
	$(LINK.cpp) -lz  -o $@ $^
#	gcc -rdynamic -ldl -lpthread -o $@ $^
#	strip $@

tabulator2: tabulator2.o libpdf/libpdf.a utf8.o
	$(LINK.cpp) -lz  -o $@ $^

dump_object: dump_object.o libpdf/libpdf.a # File.o Object.o Document.o Filter_Flate.o
	$(LINK.cpp) -lz -o $@ $^

page_metafile: page_metafile.o libpdf/libpdf.a utf8.o
	$(LINK.cpp) -lz -o $@ $^

page_to_html: page_to_html.o libpdf/libpdf.a utf8.o
	$(LINK.cpp) -lz -o $@ $^

#libpdf/libpdf.a:
#	make -C libpdf libpdf.a

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

#docs: pdflib # buld docs after library


#---------------------------------------------------------------------------
# PHony targets --- just commands, no depends on files created
.PHONY : clean docs

clean:
	for d in ${SUBDIRS}; do make -C $$d clean; done
	rm -f ${OBJS_ALL} ${BINS} ${DLLS} .depend core page_metafile.o dump_object.o
#	make -C libpdf clean

# generate documentation
docs:
	doxygen

tgz: clean
	rm -f ${TARNAME}
	tar czf ${TARNAME} *

#install:
#	install -d ${DIR_BINS}
#	install -d ${DIR_LIBS}
#	install -s -m 500 ${BINS} ${DIR_BINS}
#	install    -m 500 ${DLLS} ${DIR_LIBS}

.depend:
	gcc -MM -MG *.cxx >.depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif







