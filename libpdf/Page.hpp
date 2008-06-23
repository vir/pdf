
#ifndef PDF_PAGE_HPP
#define PDF_PAGE_HPP

// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

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
#include "Ctm.hpp"

namespace PDF {

class Font;
class OH;
class Media;

/** \brief Represents page content
 */
class Page
{
	public:
		class Operator;
		class GraphicsState;
		class Path;
  private:
		class TextObject;
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
    struct TextState {
      double Tc; // char spacing (Tc)=0
      double Tw; // word spacing (Tw)=0
      double Th; // horizontal scaling (Tz)=100
      double Tl; // leading (TL)=0
      Font * Tf; // text font (Tf)
      double Tfs; // font size (Tf)
      // Tmode // rendering mode (Tr)=0
      double Trise; // text rise (Ts)=0
      // Tk // text knockout
			TextState():Tc(0),Tw(0),Th(100),Tl(0),Tf(NULL),Tfs(1),Trise(0) {}
			void dump(std::ostream & s) const;
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

