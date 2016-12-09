
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
#include <cmath>
#include <stack>

#include "Object.hpp"
#include "Point.hpp"
#include "Rect.hpp"
#include "Ctm.hpp"
#include "Path.hpp"

namespace PDF {

class Font;
class OH;
class Media;
class GraphicsState;
class XObject;

/** \brief Represents page content
 */
class Page
{
	public:
		class Operator;
		class Render;
		class TextObject;
		class GraphicsStateStack
		{
		public:
			GraphicsStateStack();
			~GraphicsStateStack();
			GraphicsState* operator ->() { return gs; }
			operator GraphicsState& () { return *gs; }
			operator GraphicsState* () { return gs; }
			void push();
			void pop();
			void dump(std::ostream& ss);
		private:
			GraphicsState * gs;
			std::stack<GraphicsState *> gstack;
		};
		struct ResourceProvider
		{
			virtual Font* get_font(std::string name) = 0;
			virtual XObject* get_xobject(std::string name) = 0;
		};
private:
		Rect media_box, crop_box;
		GraphicsStateStack gs;
    
    std::vector<Operator *> operators;
    std::map<std::string,Font *> fonts;
    std::map<std::string, XObject *> xobjects;
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

class Page::Render
{
public:
	enum Mode { M_PAGE, M_PATH, M_TEXT, M_IMAGE };
public:
	Render(Page::GraphicsStateStack& gs, Page::ResourceProvider& res, Media& m);
	~Render();
	bool draw(const Page::Operator& op);
	void dump(std::ostream& s);
	const char* mode_string() const;
protected:
	bool draw_page_mode(const Page::Operator& op);
	bool draw_path_mode(const Page::Operator& op);
	bool draw_text_mode(const Page::Operator& op);
	bool draw_image_mode(const Page::Operator& op);
private:
	Page::GraphicsStateStack& gs;
	ResourceProvider& res;
	Media& m;
	Mode mode;
	Path* curpath;
	Page::TextObject* tobj;
};

class XObject
{
private:
	XObject();
	XObject(const XObject&);
protected:
	XObject(std::string name): m_name(name) { }
public:
	virtual ~XObject() { }
	static XObject* create(std::string name, OH definition);
	const std::string& name() const { return m_name; }
	virtual void update_ctm(CTM& ctm) { }
	virtual void draw(Page::Render& r) = 0;
private:
	std::string m_name;
};

class FormXObject : public XObject
{
private:
	CTM xobjctm;
public:
	FormXObject(std::string name);
	void load(OH dic);
	virtual void update_ctm(CTM& ctm) { ctm *= xobjctm; }
	virtual void draw(Page::Render& r);
};


}; // namespace PDF



#endif /* PDF_PAGE_HPP */

