#include "Font_Metrics.hpp"
#include "Font_StdFonts.hpp"

PDF::Font::Metrics::Metrics(Font::Encoding* enc)
	: defcharwidth(1000)
{

}
void PDF::Font::Metrics::load_widths_array(OH widths, OH firstchar)
{
	int first = 0;
	if(firstchar) {
		Integer * i;
		firstchar.put(i, "FirstChar is not an integer?!?");
		first = i->value();
	}

	for(unsigned int i = 0; i < widths.size(); i++) {
		OH aeh = widths[i];
		Integer * ip;
		aeh.put(ip, "Non-integer char width not supported");
		if (ip && ip->value())
			charwidths[i + first] = ip->value();
	}
}

void PDF::Font::Metrics::load_base_font(std::string font)
{
	load_stdfont_widths_table(charwidths, unicodewidths, font);
}

void PDF::Font::Metrics::set_char_widths(int first, int last, unsigned long val)
{
	for (int i = first; i <= last; i++) {
		charwidths[i] = val;
	}
}

double PDF::Font::Metrics::get_glyph_width(int glyph_index, wchar_t char_code)
{
	if (char_code && !unicodewidths.empty()) // first, try unicode map, as it takes into account Encoding's Differences arrays
	{
		std::map<wchar_t, unsigned long>::const_iterator it = unicodewidths.find(char_code);
		if (it != unicodewidths.end())
			return it->second / 1000.0;
	}
	std::map<int, unsigned long>::const_iterator it = charwidths.find(glyph_index);
	return ((it != charwidths.end()) ? it->second : defcharwidth) / 1000.0;
}

