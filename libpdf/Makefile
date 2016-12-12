.SUFFIXES: .a .so .cxx
.PHONY : clean docs
#---------------------------------------------------------------------------
SOURCES = \
	File.cxx Object.cxx Document.cxx ObjectsCache.cxx \
	Filter.cxx Filter_Flate.cxx Filter_Base85.cxx \
	Page.cxx Path.cxx Content.cxx \
	GraphicsState.cxx Font_Metrics.cxx \
	Content_Render.cxx Content_TextObject.cxx \
	Font.cxx Font_Encoding.cxx Font_CMap.cxx \
	Font_StdFonts.cxx SecHandler.cxx Crypto.cxx \
	ObjStrm.cxx AES.c Filter_AESV2.cxx
OBJS := $(filter %.o,$(SOURCES:.cxx=.o))
OBJS += $(filter %.o,$(SOURCES:.c=.o))
DEPS := $(filter %.d,$(SOURCES:.cxx=.d))
DEPS += $(filter %.d,$(SOURCES:.c=.d))

#CC = /usr/local/gcc-3.4.3/bin/gcc
#CXX = /usr/local/gcc-3.4.3/bin/g++
CXXFLAGS = -Wall -pipe -g

all: $(DEPS) $(OBJS) libpdf.a

clean:
	rm -f $(OBJS) $(DEPS) libpdf.a .depend core

libpdf.a: $(OBJS)
	ar r $@ $?

%.d: %.cxx
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.o: %.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

%.o: %.cxx
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

glyphlist.i: mk_encoding_table.pl glyphlist.txt pdf_encodings.txt
	./mk_encoding_table.pl

enc_win.i: mk_encoding_table.pl glyphlist.txt pdf_encodings.txt
	./mk_encoding_table.pl

enc_mac.i: mk_encoding_table.pl glyphlist.txt pdf_encodings.txt
	./mk_encoding_table.pl

enc_std.i: mk_encoding_table.pl glyphlist.txt pdf_encodings.txt
	./mk_encoding_table.pl

deffont_widths.i: Core14_AFMs.tar parse_font_metrics.pl
	./parse_font_metrics.pl > deffont_widths.i

glyphlist.txt:
	wget http://www.adobe.com/devnet/opentype/archives/glyphlist.txt

Core14_AFMs.tar:
	wget http://www.adobe.com/devnet/font/pdfs/Core14_AFMs.tar

stdfont_metrics.i: glyphlist.txt FontMetrics/Courier.afm
	FontMetrics/convert.pl > stdfont_metrics.i

FontMetrics/Courier.afm: Core14_AFMs.tar
	tar xf Core14_AFMs.tar -C FontMetrics

ifneq "$(wildcard *.d)" ""
include $(wildcard $(DEPS))
endif

