// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include "Font_StdFonts.hpp"
#include <iostream>
/*
 * Font metrics available from http://www.adobe.com/devnet/font/
 * (link at page bottom).
 */

#if 0

struct width_table {
	const char * name;
	const short * w;
	unsigned int wn;
};

#include "deffont_widths.i"

void load_stdfont_widths_table(std::map<int, unsigned long> & charwidths, std::string basefont)
{
	unsigned int i, c;
	for(i = 0; i < sizeof(standard_font_widths_table)/sizeof(standard_font_widths_table[0]); i++) {
		if(basefont == standard_font_widths_table[i].name) {
			for(c = 0; c < standard_font_widths_table[i].wn; c++) {
				charwidths[c] = standard_font_widths_table[i].w[c];
			}
			return;
		}
	} // for all fonts
	std::cerr << "Can not find widths array for standard font " << basefont << std::endl;
}

#endif

// { "space", 0x0020, 32, 600 },
struct StdFontChar {
	const char* name;
	wchar_t unicode;
	int code;
	unsigned long width;
	const char* box;
};

enum FontEncoding {
	Unknown,
	FontSpecific,
	AdobeStandardEncoding,
};

// { "Courier", AdobeStandardEncoding, 315, &stdfont_Courier_chars },
struct StdFontMetric {
	const char* name;
	enum FontEncoding enc;
	size_t nchars;
	struct StdFontChar* chars;
};

#include "stdfont_metrics.i"

void load_stdfont_widths_table(std::map<int, unsigned long> & glyphwidths, std::map<wchar_t, unsigned long> & unicodewidths, std::string font)
{
	unsigned int i, c;
	for (i = 0; i < sizeof(fonts) / sizeof(fonts[0]); i++) {
		if (font != fonts[i].name)
			continue;
		for (c = 0; c < fonts[i].nchars; c++) {
			glyphwidths[fonts[i].chars[c].code] = fonts[i].chars[c].width;
			unicodewidths[fonts[i].chars[c].unicode] = fonts[i].chars[c].width;
		}
		return;
	} // for all fonts
	std::cerr << "Can not find widths array for standard font " << font << std::endl;
}
