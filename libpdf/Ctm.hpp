#ifndef CTM_HPP_INCLUDED
#define CTM_HPP_INCLUDED

#include <string>
#include <iostream>
#include <iomanip>
#include <math.h>

#include "Point.hpp"
#include "Rect.hpp"

#ifndef M_PI
#define M_PI atan2(0, -1)
#endif

namespace PDF {

/** \brief Coordinate transormation matrix
 * 
 * Referenced on pages: 178 (transormation matrix general), 372 (text matrix)
 *
 * Coordinate transformation: [x', y', 1] = [x, y, 1] x M
 * 
 */
class CTM
{
  private:
    double a, b, c, d, e, f;
  public:
    CTM() { set_unity(); }
    CTM(double ma, double mb, double mc, double md, double me, double mf):a(ma),b(mb),c(mc),d(md),e(me),f(mf)
		{
			if(a == 0 && b == 0) { // workaround a BUG in dium at least (how other viewers shows it???) // support 0,0,0,1,e,f and 0,0,-1,0,e,f
				a = d;
				b = -c;
			}
		}
    Point translate(const Point & p) const
    {
      Point t(a*p.x + c*p.y + e, b*p.x + d*p.y + f);
//      std::clog << "Translate " << p.dump() << " to " << t.dump() << std::endl;
      return t;
    }
    void set_unity() { a=d=1.0; b=c=e=f=0.0; }
    void offset(Point p) { e+=p.x; f+=p.y; }
    void offset(double ox, double oy) { e+=ox; f+=oy; }
    void offset_unscaled(Point p) { e+=a*p.x + c*p.y; f+=b*p.x + d*p.y; }
    void offset_unscaled(double ox, double oy) { e+=a*ox + c*oy; f+=b*ox + d*oy; }
		void scale(double sx, double sy=0)
		{
			if(sy == 0) sy = sx;
			a*=sx; c*=sx; e*=sx;
			b*=sy; d*=sy; f*=sy;
		}
		void rotate(double angle)
		{
			angle = angle*M_PI/180.0;
			double co = cos(angle);
			double si = sin(angle);
			CTM m(co, si, -si, co, 0, 0);
			operator *=(m);
		}
		double get_rotation_angle() const
		{
			return atan2(b, a)*180/M_PI;
		}
		double get_scale_h() const
		{
			return sqrt(a*a + b*b);
		}
		double get_scale_v() const
		{
			return sqrt(c*c + d*d);
		}
		void skew(double alpha, double beta)
		{
			alpha = alpha*M_PI/180.0;
			beta  =  beta*M_PI/180.0;
			CTM m(1, tan(alpha), tan(beta), 1, 0, 0);
			operator *=(m);
		}
		CTM operator * (const CTM & m2) const {
			CTM r;
			r.a = a*m2.a + b*m2.c;
			r.b = a*m2.b + b*m2.d;
			r.c = c*m2.a + d*m2.c;
			r.d = c*m2.b + d*m2.d;
			r.e = e*m2.a + f*m2.c + m2.e;
			r.f = e*m2.b + f*m2.d + m2.f;
			return r;
		}
		CTM & operator *= (const CTM & m2) {
			CTM n = m2 * *this;
			*this = n;
			return *this;
		}
		std::string dump()
		{
      std::stringstream ss;
			ss << std::setiosflags(std::ios::fixed) << std::setprecision(1) << std::setiosflags(std::ios::right);
			ss <<  " / " << std::setw(6) << a << " | " << std::setw(6) << b << " | 0 \\" << std::endl;
			ss <<  "|  " << std::setw(6) << c << " | " << std::setw(6) << d << " | 0  |" << std::endl;
			ss << " \\ " << std::setw(6) << e << " | " << std::setw(6) << f << " | 1 /" << std::endl;
			return ss.str();
		}
		Point ef() const
		{
			return Point(e, f);
		}
};

} /* namespace PDF */

#endif /* CTM_HPP_INCLUDED */

