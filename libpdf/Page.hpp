
#ifndef PDF_PAGE_HPP
#define PDF_PAGE_HPP

#ifdef _MSC_VER
# pragma warning(disable : 4786) // get rid of "identifier was truncated to '255' characters..."
# pragma warning(disable : 4284)
#endif

#include <fstream>
#include <string>
#include <map>
#include <vector>
//#include <sstream>
#include <iostream>
#include <iomanip>
#include <stack>
#include <cmath>

#include "Object.hpp"
#include "Point.hpp"
#include "Rect.hpp"
#include "Ctm.hpp"

namespace PDF {

class Font;
class OH;
class Media;
class GraphicsState;

/** \brief Represents page content
 */
class Page
{
	public:
		class Operator;
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
	const Rect& get_meadia_box() const { return media_box; }
	const Rect& get_crop_box() const { return crop_box; }
    void draw(Media * m);
		/** Set limit on number of operators executed by draw() function (for
		 * debugging purposes */
		void set_operators_number_limit(unsigned int n) { m_operators_number_limit = n; }
		unsigned int get_operators_number_limit() const { return m_operators_number_limit; }
		size_t get_operators_count() const { return operators.size(); }
		std::streamoff get_operator_offset(unsigned int n) const;
};

class Page::Operator
{
  private:
    std::streamoff m_offset;
    std::string m_name;
    std::vector<Object *> * m_args;
  public:
    Operator(std::streamoff offset, std::string op, std::vector<Object *>* a):m_offset(offset),m_name(op),m_args(a) {}
    ~Operator()
    {
      if(!m_args) return;
      for(std::vector<Object *>::iterator it=m_args->begin(); it!= m_args->end(); it++) delete *it;
      delete m_args;
    }
    std::string name() const { return m_name; }
    std::streamoff offset() const { return m_offset; }
		bool operator == (const char * cmp) { return m_name == cmp; }
    const Object * arg(unsigned int i) const { return i>=m_args->size()?NULL:m_args->at(i); }
		const Object * operator[](unsigned int i) const { return arg(i); }
    double number(unsigned int i) const;
    Point point(unsigned int i) const
    {
      return Point(number(i), number(i+1));
    }
		CTM matrix() const
		{
			return CTM(number(0), number(1), number(2), number(3), number(4), number(5));
		}
    std::string dump() const;
};


}; // namespace PDF



#endif /* PDF_PAGE_HPP */

