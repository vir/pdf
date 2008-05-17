
#ifndef PDF_MEDIA_HPP
#define PDF_MEDIA_HPP

#include <string>

namespace PDF {

/// Interface to draw on some media (i.e. paper).
class Media
{
	public:
		virtual ~Media() {};
		virtual void Size(Point unity) { }
		virtual const CTM & Matrix() { static CTM m; return m; }
		virtual void Debug(std::string s) { }
		virtual void SetFont(const Font * font, double size) {}
		virtual void Text(Point pos, std::wstring text) {};
		virtual void Text(Point pos, double angle, std::wstring text) { Text(pos, text); };
		virtual void Line(const Point & p1, const Point & p2)=0;
};

};

#endif /* PDF_MEDIA_HPP */



