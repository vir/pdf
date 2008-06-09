
#ifndef PDF_POINT_HPP
#define PDF_POINT_HPP

#include <string>
#include <sstream>
#include <iomanip>

namespace PDF {

class Point
{
  public:
    double x,y;
    Point():x(0),y(0) {}
    Point(double nx, double ny):x(nx),y(ny) {}
    Point(const Point & p):x(p.x),y(p.y) {}
    Point & operator =(const Point & p) { x=p.x; y=p.y; return *this; }
    std::string dump() const
		{
			std::stringstream ss;
			ss << std::setiosflags(std::ios::fixed) << std::setprecision(1) << std::setiosflags(std::ios::right);
			ss << "(" << x << "," << y << ")";
			return ss.str();
		}
    Point & operator+=(const Point & p) { x+=p.x; y+=p.y; return *this; }
		Point & operator-=(const Point & p) { x-=p.x; y+=p.y; return *this; }
		Point operator-(const Point & p) { return Point(x - p.x, y - p.y); }
    bool operator < (const Point & p) const { return (y == p.y)?x<p.x:y<p.y; }
};
  
};

#endif /* PDF_POINT_HPP */



