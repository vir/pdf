#ifndef PATH_HPP_INCLUDED
#define PATH_HPP_INCLUDED

#include <vector>
#include "Point.hpp"
#include "Rect.hpp"

namespace PDF {

class Path: public std::vector<Point>
{
public:
	std::string dump() const;
	bool is_rectangular() const;
	bool is_inside(const Point& pt, bool even_odd_rule) const { return even_odd_rule ? is_inside_eor(pt) : is_inside_n0wnr(pt); }
	bool clip_line(Point& pt1, Point& pt2, bool even_odd_rule) const;
	bool clip(Rect& r) const;
	static bool lines_intersects(const Point& A, const Point& B, const Point C, const Point D, Point* X = NULL);
	static inline double det2(double x1, double x2, double y1, double y2)
	{
		return (x1 * y2 - y1 * x2);
	}
	Rect bbox() const;
protected:
	bool is_inside_n0wnr(const Point& pt) const;
	bool is_inside_eor(const Point& pt) const;
};

} // namespace PDF

#endif /* PATH_HPP_INCLUDED */
