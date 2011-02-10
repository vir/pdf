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


