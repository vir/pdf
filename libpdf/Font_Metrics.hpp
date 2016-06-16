#ifndef PDF_FONT_METRICS_HPP_INCLUDED
#define PDF_FONT_METRICS_HPP_INCLUDED

#include "Font.hpp"
#include "Font_Encoding.hpp"

namespace PDF {

/** Font metrics */
class Font::Metrics
{
	public:
		Metrics(Font::Encoding* enc);
		void load_widths_array(OH widths, OH firstchar);
		void load_base_font(std::string font);
		void set_char_widths(int first, int last, unsigned long val);
		double get_glyph_width(int glyph_index, wchar_t char_code);
		void set_defcharwidth(unsigned long cw)
		{
			defcharwidth = cw;
		}
	private:
		std::map<int, unsigned long> charwidths;
		std::map<wchar_t, unsigned long> unicodewidths;
		unsigned long defcharwidth;
		Font::Encoding* encoding;
};

} /* namespace PDF */

#endif /* PDF_FONT_METRICS_HPP_INCLUDED */
