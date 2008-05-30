.SUFFIXES: .a .so .cxx
.PHONY : clean docs
#---------------------------------------------------------------------------
SOURCES = \
	File.cxx Object.cxx Document.cxx ObjectsCache.cxx \
	Filter.cxx Filter_Flate.cxx Filter_Base85.cxx \
	Page.cxx Font.cxx Font_Encoding.cxx Font_CMap.cxx
OBJS = $(SOURCES:.cxx=.o)
DEPS = $(SOURCES:.cxx=.d)

#CC = /usr/local/gcc-3.4.3/bin/gcc
#CXX = /usr/local/gcc-3.4.3/bin/g++
CXXFLAGS = -Wall -pipe -g

all: $(DEPS) $(OBJS) libpdf.a

clean:
	rm -f ${OBJS} $(SOURCES:.cxx=.d) libpdf.a .depend core

libpdf.a: $(OBJS)
	ar r $@ $?

%.d: %.cxx
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.o: %.cxx
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<


include $(SOURCES:.cxx=.d)


