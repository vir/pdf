#include "Page_Path.hpp"
#include "Exceptions.hpp"

std::string PDF::Page::Path::dump() const
{
	std::stringstream ss;
	for(unsigned int i=0; i<size()-1; i++)
		ss << at(i).dump();
	return ss.str();
}

bool PDF::Page::Path::is_rectangular() const
{
	if(size() != 5)
		return false;
	if(at(0) != at(4))
		return false;
	if(at(0).x == at(1).x && at(2).x == at(3).x
		&& at(0).y == at(3).y && at(1).y == at(2).y)
		return true;
	if(at(0).y == at(1).y && at(2).y == at(3).y
		&& at(0).x == at(3).x && at(1).x == at(2).x)
		return true;
	if(at(0).x == at(1).x && at(2).x == at(3).x
		&& at(0).y == at(3).y && at(1).y == at(2).y)
		return true;
	return false;
}

bool PDF::Page::Path::clip(Rect& r) const
{
	if(empty())
		return true;
	if(! is_rectangular())
		throw UnimplementedException("Non-rectangular clipping paths");
	Rect clipper(at(0), at(2));
	return clipper.clip_him(r);
}
