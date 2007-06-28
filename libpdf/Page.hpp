
#ifndef PDF_PAGE_HPP
#define PDF_PAGE_HPP

#include <fstream>
#include <string>
#include <map>
#include <vector>
//#include <sstream>
#include <iostream>
#include <stack>

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
    void scale(double sx, double sy=0) { a*=sx; d*=sy?sy:sx; }
};
  
/** \brief Represents page content
 */
class Page
{
  private:
    class Operator;
    class GraphicsState;
		class TextObject;
    //class Path;
		Rect media_box, crop_box;
    typedef std::vector<Point> Path;
    
    // graphics state
    GraphicsState * gs;
    std::stack<GraphicsState *> gstack;
    
    std::vector<Operator *> operators;
    std::map<std::string,Font *> fonts;
    int m_debug;
  public:
    Page();
    ~Page();
    /// sets debug level, returns previous debug level
    int debug(int d) { int t=m_debug; m_debug=d; return t; }
    std::string dump() const;
    bool load(OH pagenode);
    bool parse(const std::vector<char> & data);
    void draw(Media * m);
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
    const Object * arg(unsigned int i) const { return i>=m_args->size()?NULL:m_args->at(i); }
    Point point(unsigned int i) const
    {
      const Object * o;
      const Real * real;
      const Integer * integer;

      o=arg(i);
      real=dynamic_cast<const Real *>(o);
      integer=dynamic_cast<const Integer *>(o);
      if(!real && !integer) throw std::string("Invalid x coordinate");
      double x=real?real->value():integer->value();

      o=arg(i+1);
      real=dynamic_cast<const Real *>(o);
      integer=dynamic_cast<const Integer *>(o);
      if(!real && !integer) throw std::string("Invalid y coordinate");
      double y=real?real->value():integer->value();
      return Point(x,y);
    }
    double number(unsigned int i) const
    {
      const Object * o=arg(i);
      if(!o) throw std::string("No such argument");
      const Real * real=dynamic_cast<const Real *>(o);
      const Integer * integer=dynamic_cast<const Integer *>(o);
      if(!real && !integer) throw std::string("Argument is not a number");
      return real?real->value():integer->value();
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
      // Tc // char spacing (Tc)=0
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

}; // namespace PDF



#endif /* PDF_PAGE_HPP */

