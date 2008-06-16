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

glyphlist.i: mk_encoding_table.pl glyphlist.txt pdf_encodings.txt
	./mk_encoding_table.pl

enc_win.i: mk_encoding_table.pl glyphlist.txt pdf_encodings.txt
	./mk_encoding_table.pl

enc_mac.i: mk_encoding_table.pl glyphlist.txt pdf_encodings.txt
	./mk_encoding_table.pl

glyphlist.txt:
	wget http://www.adobe.com/devnet/opentype/archives/glyphlist.txt

Font_Encoding.cxx: glyphlist.i enc_win.i enc_mac.i

include $(SOURCES:.cxx=.d)


