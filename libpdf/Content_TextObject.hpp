#ifndef CONTENT_TEXTOBJECT_HPP_INCLUDED
#define CONTENT_TEXTOBJECT_HPP_INCLUDED

#include "Page.hpp"
#include "GraphicsState.hpp"

namespace PDF {

class String;

/** Work around mutiple text-showing operators in same text object.
 *  if so, concatenate them and output them all together.
 */
class Content::TextObject
{
	private:
		enum TextDir { LTR, RTL, TTB, BTT };
		const GraphicsState * gs;
		Media * media;
		double kerning_too_big;
	public:
		CTM tm;
		CTM lm;
		std::wstring accumulated_text; // work around multiple Tj in one text object
		double total_width;
		bool update_font;

		TextObject(const GraphicsState * g, Media * m):gs(g),media(m),total_width(0),update_font(false)
		{
			kerning_too_big = 0.3; //0.4;
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
			lm.offset_unscaled(0, -gs->text_state.Tl); // XXX +Tl in spec!!! BUG?
			tm = lm;
		}
		void Offset(const Point & p) {
			Flush();
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

#endif /* CONTENT_TEXTOBJECT_HPP_INCLUDED */

