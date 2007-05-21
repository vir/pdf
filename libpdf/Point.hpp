
#ifndef PDF_POINT_HPP
#define PDF_POINT_HPP

#include <string>
#include <sstream>

namespace PDF {

class Point
{
  public:
    double x,y;
    Point(double nx, double ny):x(nx),y(ny) {}
    Point(const Point & p):x(p.x),y(p.y) {}
    Point & operator =(const Point & p) { x=p.x; y=p.y; return *this; }
    std::string dump() const { std::stringstream ss; ss << "(" << x << "," << y << ")"; return ss.str(); }
    Point & operator+=(const Point & p) { x+=p.x; y+=p.y; return *this; }
    bool operator < (const Point & p) const { return (y == p.y)?x<p.x:y<p.y; }
};
  
};

#endif /* PDF_POINT_HPP */



