
#ifndef PDF_RECT_HPP
#define PDF_RECT_HPP

#include "Point.hpp"
#include <string>
#include <sstream>
#include <algorithm> /* for swap */

namespace PDF {

class Rect
{
	private:
//		void swap(double d1, double d2) { double }
		void normalize() { if(x2<x1) std::swap(x1,x2); if(y2<y1) std::swap(y1,y2); }
  public:
    double x1,y1,x2,y2;
    Rect():x1(0),y1(0),x2(0),y2(0) {}
    Rect(double a[4]):x1(a[0]),y1(a[1]),x2(a[2]),y2(a[3]) { normalize(); }
    Rect(double nx1, double ny1, double nx2, double ny2):x1(nx1),y1(ny1),x2(nx2),y2(ny2) { normalize(); }
    Rect(const Point & p1, const Point & p2):x1(p1.x),y1(p1.y),x2(p2.x),y2(p2.y) { normalize(); }
    Rect(const Point & p1, double w, double h):x1(p1.x),y1(p1.y),x2(p1.x + w),y2(p1.y + h) { normalize(); }
    Rect & operator =(const Rect & o) { x1=o.x1; y1=o.y1; x2=o.x2; y2=o.y2; return *this; }
    std::string dump() const { std::stringstream ss; ss << "(" << x1 << "," << y1 << "," << x2 << "," << y2 << ")"; return ss.str(); }
		Point offset() const { return Point(x1,y1); }
		Point size() const { return Point(x2-x1, y2-y1); }
		bool in(Point p) const { return p.x>=x1 && p.x<=x2 && p.y>=y1 && p.y<=y2; }
		bool normalized() const { return x2 >= x1 && y2 >= y1; }
		double width() const { return size().x; }
		double height() const { return size().y; }

//    bool operator < (const Point & p) const { return (y == p.y)?x<p.x:y<p.y; }
};
  
};

#endif /* PDF_RECT_HPP */



