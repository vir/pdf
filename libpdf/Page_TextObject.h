#ifndef PAGE_TEXTOBJECT_HPP_INCLUDED
#define PAGE_TEXTOBJECT_HPP_INCLUDED

#include "Page.hpp"

namespace PDF {

/** Work around mutiple text-showing operators in same text object.
 *  if so, concatenate them and output them all together.
 */
class Page::TextObject
{
	private:
		enum TextDir { LTR, RTL, TTB, BTT };
		const Page::GraphicsState * gs;
		Media * media;
		double kerning_too_big;
		unsigned int spaces;
		double space_width; // width of last space
	public:
		CTM tm;
		CTM lm;
		std::wstring accumulated_text; // work around multiple Tj in one text object
		double total_width;
		bool update_font;

		TextObject(const Page::GraphicsState * g, Media * m):gs(g),media(m),total_width(0),update_font(false)
		{
			kerning_too_big = 0.3; //0.4;
			spaces = 0;
		}
		void SetMatrix(const CTM & m)
		{
			tm = lm = m;
			update_font = true;
		}
		void Append(const String * str);
		void Kerning(double k);
		void NewLine() {
			Flush();
			spaces = 0;
			lm.offset_unscaled(0, -gs->text_state.Tl); // XXX +Tl in spec!!! BUG?
			tm = lm;
		}
		void Offset(const Point & p) {
			Flush();
			spaces = 0;
			lm.offset_unscaled(p);
			tm = lm;
		}
		void Flush();
		void FontChanged()
		{
			update_font = true;
		}
};

}; // namespace PDF

#endif /* PAGE_TEXTOBJECT_HPP_INCLUDED */

