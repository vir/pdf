#ifndef PDF_MEDIA_HPP
#define PDF_MEDIA_HPP

#include <string>
#include "Point.hpp"
#include "Path.hpp"
#include "Ctm.hpp"
#include "GraphicsState.hpp"

namespace PDF {

class Font;
class Stream;

/// Interface to draw on some media (i.e. paper).
class Media
{
	public:
		enum PathFillMode { PATH_NO_FILL = 0, PATH_FILL_EOR, PATH_FILL_NZWNR };
	public:
		virtual ~Media() {};
		virtual void Size(Point unity) { }
		virtual const CTM & Matrix() { static CTM m; return m; }
		virtual void Debug(unsigned int opnum, std::string s, const GraphicsState& gs) { }
		virtual void SetFont(const Font * font, double size) {}
		virtual void Text(Rect pos, double angle, std::wstring text, bool visible, const GraphicsState& gs)=0;
		virtual void DrawPath(const Path& path, PathFillMode fill, bool stroke, const GraphicsState& gs)
		{
			if(path.size() < 2)
				return;
			for(unsigned int i = 1; i < path.size(); i++)
				Line(gs.ctm.translate(path.at(i - 1)), gs.ctm.translate(path.at(i)), gs);
		};
		virtual void Line(const Point & p1, const Point & p2, const GraphicsState& gs) { };
		virtual void Image(const Rect & rect, const Stream & strm, const GraphicsState& gs) { }
};

};

#endif /* PDF_MEDIA_HPP */



