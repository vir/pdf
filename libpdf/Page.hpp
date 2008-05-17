
#ifndef PDF_PAGE_HPP
#define PDF_PAGE_HPP

#include <fstream>
#include <string>
#include <map>
#include <vector>
//#include <sstream>
#include <iostream>
#include <iomanip>
#include <stack>
#include <math.h>

#include "Object.hpp"
#include "Point.hpp"
#include "Rect.hpp"

namespace PDF {

class Font;
class OH;
class Media;
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
    CTM(double ma, double mb, double mc, double md, double me, double mf):a(ma),b(mb),c(mc),d(md),e(me),f(mf) { }
    Point translate(const Point & p) const
    {
      Point t(a*p.x + c*p.y + e, b*p.x + d*p.y + f);
//      std::clog << "Translate " << p.dump() << " to " << t.dump() << std::endl;
      return t;
    }
    void set_unity() { a=d=1.0; b=c=e=f=0.0; }
    void offset(Point p) { e+=p.x; f+=p.y; }
    void offset_unscaled(Point p) { e+=a*p.x + c*p.y; f+=b*p.x + d*p.y; }
    void offset(double ox, double oy) { e+=ox; f+=oy; }
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

/** \brief Represents page content
 */
class Page
{
  private:
    class Operator;
    class GraphicsState;
		class TextObject;
    class Path;
    //class Path;
		Rect media_box, crop_box;
    
    // graphics state
    GraphicsState * gs;
    std::stack<GraphicsState *> gstack;
    
    std::vector<Operator *> operators;
    std::map<std::string,Font *> fonts;
    int m_debug;
		unsigned int m_operators_number_limit;
  public:
    Page();
    ~Page();
    /// sets debug level, returns previous debug level
    int debug(int d) { int t=m_debug; m_debug=d; return t; }
    std::string dump() const;
    bool load(OH pagenode);
//    bool load(std::istream & pagestream);
    bool parse(const std::vector<char> & data);
    void draw(Media * m);
		/** Set limit on number of operators executed by draw() function (for
		 * debugging purposes */
		void set_operators_number_limit(unsigned int n) { m_operators_number_limit = n; }
};

class Page::Operator
{
  private:
    std::string m_name;
    std::vector<Object *> * m_args;
  public:
    Operator(std::string op, std::vector<Object *>* a):m_name(op),m_args(a) {}
    ~Operator()
    {
      if(!m_args) return;
      for(std::vector<Object *>::iterator it=m_args->begin(); it!= m_args->end(); it++) delete *it;
      delete m_args;
    }
    std::string name() const { return m_name; }
		bool operator == (const char * cmp) { return m_name == cmp; }
    const Object * arg(unsigned int i) const { return i>=m_args->size()?NULL:m_args->at(i); }
		const Object * operator[](unsigned int i) const { return arg(i); }
    double number(unsigned int i) const
    {
      const Object * o=arg(i);
      if(!o) throw std::string("No such argument");
      const Real * real=dynamic_cast<const Real *>(o);
      const Integer * integer=dynamic_cast<const Integer *>(o);
      if(!real && !integer) throw std::string("Argument is not a number");
      return real?real->value():integer->value();
    }
    Point point(unsigned int i) const
    {
      return Point(number(i), number(i+1));
    }
		CTM matrix() const
		{
			return CTM(number(0), number(1), number(2), number(3), number(4), number(5));
		}
    std::string dump() const
    {
      std::stringstream ss; bool flag=false;
      ss << name() << "(";
      if(m_args)
      {
        for(std::vector<Object *>::const_iterator it=m_args->begin(); it!=m_args->end(); it++)
        {
          if(flag) ss << ", ";
          ss << (*it)->dump();
          flag=true;
        }
      }
      ss << ")";
      return ss.str();
    }
};


/*class Page::Path:public std::vector<Point>
{
};*/

class Page::GraphicsState {
  public:
    GraphicsState() {}
    /*=== device-independent: ===*/
    CTM ctm;
    // clipping_path
    // color_space
    // color
    struct {
      double Tc; // char spacing (Tc)=0
      double Tw; // word spacing (Tw)=0
      // Th // horizontal scaling (Tz)=100
      double Tl; // leading (TL)=0
      Font * Tf; // text font (Tf)
      double Tfs; // font size (Tf)
      // Tmode // rendering mode (Tr)=0
      // Trise // text rise (Ts)=0
      // Tk // text knockout
    } text_state;
    // line_width // set with 'w'
    // line_cap // set with 'J'
    // line_join // set with 'j'
    // miter_limit // set with 'M'
    // dash_pattern set with 'd'
    // rendering_intent // set with 'ri'
    // stroke_adjustment
    // blend_mode
    // soft_mask
    // alpha_constant
    // alpha_source
    /*=== device-dependent: ===*/
    // overprint
    // overprint_mode
    // black_generation
    // undercolor_removal
    // transfer
    // halftone
    // flatness // set with 'i'
    // smothness
};

class Page::Path: public std::vector<Point>
{
	public:
		std::string dump()
		{
      std::stringstream ss;
			for(unsigned int i=0; i<size()-1; i++)
				ss << at(i).dump();
			return ss.str();
		}
};
  
}; // namespace PDF



#endif /* PDF_PAGE_HPP */

