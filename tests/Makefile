.SUFFIXES: .a .so .cxx
.PHONY : clean docs
#---------------------------------------------------------------------------
SOURCES = t-Page.cxx
OBJS = $(SOURCES:.cxx=.o)
DEPS = $(SOURCES:.cxx=.d)
BINS = $(SOURCES:.cxx=)

#CC = /usr/local/gcc-3.4.3/bin/gcc
#CXX = /usr/local/gcc-3.4.3/bin/g++
CXXFLAGS = -Wall -pipe -g

all: $(DEPS) $(OBJS) ${BINS}

clean:
	rm -f ${OBJS} ${DEPS} ${BINS} .depend core

t-Page: t-Page.o ../libpdf/libpdf.a
	$(LINK.cpp) -lz -o $@ $^

%.d: %.cxx
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.o: %.cxx
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<


include $(SOURCES:.cxx=.d)


