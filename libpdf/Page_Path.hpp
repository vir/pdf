#ifndef PAGE_PATH_HPP_INCLUDED
#define PAGE_PATH_HPP_INCLUDED

#include "Page.hpp"

namespace PDF {

class Page::Path: public std::vector<Point>
{
public:
	std::string dump() const;
	bool is_rectangular() const;
	bool clip(Rect& r) const;
};

}; // namespace PDF

#endif /* PAGE_PATH_HPP_INCLUDED */
