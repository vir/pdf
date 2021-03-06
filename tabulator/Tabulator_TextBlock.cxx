#ifdef _MSC_VER
# pragma warning(disable : 4786) // get rid of "identifier was truncated to '255' characters..."
# ifdef _DEBUG
#  ifdef _CRTDBG_MAP_ALLOC
#   include <stdlib.h>  
#   include <crtdbg.h>  
#   ifndef DBG_NEW
#    define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#    define new DBG_NEW
#   endif
#  endif
# endif // _DEBUG
#endif

#include "Tabulator.hpp"
#include <assert.h>

bool Tabulator::TextBlock::debug = false;

Tabulator::TextBlock::TextBlock()
	: width(0)
	, height(0)
	, angle(0)
{
}

Tabulator::TextBlock::TextBlock(const PDF::Point & p, std::wstring s)
	: pos(p)
	, text(s)
	, width(0)
	, height(0)
	, angle(0)
{
}

Tabulator::TextBlock::TextBlock(const PDF::Point & p, double a, std::wstring s, double w, double h)
	: pos(p)
	, text(s)
	, width(w)
	, height(h)
	, angle(a)
{
	assert(width >= 0);
}

std::string Tabulator::TextBlock::dump() const
{
	std::ostringstream ss;
	ss << pos.dump();
	ss << '"';
	for(unsigned int i = 0; i < text.length(); ++i) {
		if(text[i] < 128 && text[i] != '\"') {
			ss << (char)text[i];
		} else {
			ss << "\\x{" << std::hex << (unsigned long)text[i] << "}";
		}
	}
	ss << '"';
	return ss.str();
}

PDF::Rect Tabulator::TextBlock::bounds() const
{
	double w = width; // XXX
	double h = height; // XXX \todo write it!
	return PDF::Rect(pos.x, pos.y, w, h);
}

bool Tabulator::TextBlock::merge_ok(const TextBlock & oth) const
{
	if(debug)
		std::clog << "merge_ok" << dump() << oth.dump() << ": ";
	if(Tabulator::Coord::compare(angle, oth.angle)) { // different angles
		if(debug)
			std::clog << "Different angles" << std::endl;
		return false;
	}
	if(Tabulator::Coord::compare(angle, 0)) {
		if(debug)
			std::cerr << "Only horizontal text joins currently" << std::endl;
		return false;
	}
	if(Tabulator::Coord::compare(pos.y, oth.pos.y)) { // Not on one line
		if(debug)
			std::clog << "Not on one line" << std::endl;
		return false;
	}
	double avgcw = width / double(text.length());
	double distance = oth.pos.x - (pos.x + width);
	bool result = distance < 0.3 * avgcw;
	if(debug)
		std::clog << "Average charwidth: " << avgcw << ", distance: " << distance << ", result: " << result << std::endl;
//	return (abs(distance) < 2*avgcw);
	return result;
}

Tabulator::TextBlock Tabulator::TextBlock::operator +(const Tabulator::TextBlock & oth) const
{
	std::wstring newtext = text;
	double distance = oth.pos.x - (pos.x + width);
#if 0
	double font_size = height;
	if(distance > 0.15*font_size) {
		std::clog << "Distance large enough, adding space" << std::endl;
		newtext += L' ';
	}
#endif
	newtext += oth.text;
	double newwidth = width + oth.width + distance;
	double newheight = height;
	if(oth.height > height)
		newheight = oth.height;
	return Tabulator::TextBlock(pos, angle, newtext, newwidth, newheight);
}



