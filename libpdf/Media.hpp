#ifndef PDF_MEDIA_HPP
#define PDF_MEDIA_HPP

#include <string>
#include "Point.hpp"
#include "Ctm.hpp"

namespace PDF {

class Font;
class GraphicsState;

/// Interface to draw on some media (i.e. paper).
class Media
{
	public:
		virtual ~Media() {};
		virtual void Size(Point unity) { }
		virtual const CTM & Matrix() { static CTM m; return m; }
		virtual void Debug(unsigned int opnum, std::string s, const GraphicsState& gs) { }
		virtual void SetFont(const Font * font, double size) {}
		virtual void Text(Rect pos, double angle, std::wstring text, bool visible, const GraphicsState& gs)=0;
		virtual void Line(const Point & p1, const Point & p2, const GraphicsState& gs)=0;
};

};

#endif /* PDF_MEDIA_HPP */



