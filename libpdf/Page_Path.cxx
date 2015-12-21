#include "Page_Path.hpp"
#include "Exceptions.hpp"
#include <limits>

std::string PDF::Page::Path::dump() const
{
	std::stringstream ss;
	if(empty())
		return "Empty";
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
	bool even_odd_rule = true;
	if(empty())
		return true;
	Point p1, p2;
	Point p;
	// top left corner is inside clipping region
	p = Point(r.x1, r.y1);
	if(is_inside(p, even_odd_rule)) {
		Point oth1(r.x2, r.y1);
		Point oth2(r.x1, r.y2);
		clip_line(p, oth1, even_odd_rule);
		clip_line(p, oth2, even_odd_rule);
		r.x2 = oth1.x;
		r.y2 = oth2.y;
		return true;
	}
	// top right corner is inside clipping region
	p = Point(r.x2, r.y1);
	if(is_inside(p, even_odd_rule)) {
		Point oth1(r.x1, r.y1);
		Point oth2(r.x2, r.y2);
		clip_line(p, oth1, even_odd_rule);
		clip_line(p, oth2, even_odd_rule);
		r.x1 = oth1.x;
		r.y2 = oth2.y;
		return true;
	}
	// bottom left corner is inside clipping region
	p = Point(r.x1, r.y2);
	if(is_inside(p, even_odd_rule)) {
		Point oth1(r.x2, r.y2);
		Point oth2(r.x1, r.y1);
		clip_line(p, oth1, even_odd_rule);
		clip_line(p, oth2, even_odd_rule);
		r.x2 = oth1.x;
		r.y1 = oth2.y;
		return true;
	}
	// bottom right corner is inside clipping region
	p = Point(r.x2, r.y2);
	if(is_inside(p, even_odd_rule)) {
		Point oth1(r.x1, r.y2);
		Point oth2(r.x2, r.y1);
		clip_line(p, oth1, even_odd_rule);
		clip_line(p, oth2, even_odd_rule);
		r.x1 = oth1.x;
		r.y1 = oth2.y;
		return true;
	}
	return false;
}

/* Determines whether a given point is inside a path using
 * a "Nonzero Winding Number Rule"
 *
 * The nonzero winding number rule determines whether a given
 * point is inside a path by conceptually drawing a ray from
 * that point to infinity in any direction and then examining
 * the places where a segment of the path crosses the ray.
 * Starting with a count of 0, the rule adds 1 each time a path
 * segment crosses the ray from left to right and subtracts 1
 * each time a segment crosses from right to left.
 * After counting all the crossings, if the result is 0, the
 * point is outside the path; otherwise, it is inside.
 */
bool PDF::Page::Path::is_inside_n0wnr(const Point& pt) const
{
	NOT_IMPLEMENTED("PDF::Page::Path::is_inside_n0wnr");
}

/* Determines whether a given point is inside a path using
 * "Even-Odd Rule"
 *
 * This rule determines whether a point is inside a path by
 * drawing a ray from that point in any direction and simply
 * counting the number of path segments that cross the ray,
 * regardless of direction. If this number is odd, the point
 * is inside; if even, the point is outside.
 *
 * This yields the same results as the nonzero winding number
 * rule for paths with simple shapes, but produces different
 * results for more complex shapes.
 */
bool PDF::Page::Path::is_inside_eor(const Point& pt) const
{
	const static Point faraway(10000000.0, 10000000.0);
	unsigned int intersection_count = 0;
	const_iterator it = begin();
	const Point& A = *it;
	for(++it; it != end(); ++it) {
		const Point& B = *it;
		if(lines_intersects(A, B, pt, faraway))
			++intersection_count;
	}
	return intersection_count % 2 ? true : false;
}

bool PDF::Page::Path::clip_line(Point& pt1, Point& pt2, bool even_odd_rule) const
{
	assert(even_odd_rule);
	even_odd_rule = true;

	const_iterator it = begin();
	const Point& A = *it;
	for(++it; it != end(); ++it) {
		const Point& B = *it;
		if(is_inside(pt1, even_odd_rule)) {
			lines_intersects(A, B, pt1, pt2, &pt2);
			return true;
		} else if(is_inside(pt2, even_odd_rule)) {
			lines_intersects(A, B, pt1, pt2, &pt2);
			return true;
		} else {
			// here we ignoring case when both ends are outside of clipping region, but line intersects it.
		}
	}
	return false;
}

/* Checks that lines AB and CD intersects and returns intersection point if X is not NULL */
bool PDF::Page::Path::lines_intersects(const Point& A, const Point& B, const Point C, const Point D, Point* X /*= NULL*/)
{
	double tolerance = 0.00000001;
	double a = det2(A.x - B.x, A.y - B.y, C.x - D.x, C.y - D.y);
	if(std::abs(a) < std::numeric_limits<double>::epsilon())
		return false; // Lines are parallel

	double  d1 = det2(A.x, A.y, B.x, B.y);
	double  d2 = det2(C.x, C.y, D.x, D.y);
	double  x = det2(d1, A.x - B.x, d2, C.x - D.x) / a;
	double  y = det2(d1, A.y - B.y, d2, C.y - D.y) / a;

	if(x < std::min(A.x, B.x) - tolerance || x > std::max(A.x, B.x) + tolerance)
		return false;
	if(y < std::min(A.y, B.y) - tolerance || y > std::max(A.y, B.y) + tolerance)
		return false;
	if(x < std::min(C.x, D.x) - tolerance || x > std::max(C.x, D.x) + tolerance)
		return false;
	if(y < std::min(C.y, D.y) - tolerance || y > std::max(C.y, D.y) + tolerance)
		return false;
	if(X) {
		X->x = x;
		X->y = y;
	}
	return true;
}
