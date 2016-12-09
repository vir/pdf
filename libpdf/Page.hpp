
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

#include "Content.hpp"
#if 0
#include "Object.hpp"
#include "Point.hpp"
#include "Rect.hpp"
#include "Ctm.hpp"
#endif

namespace PDF {

class Font;
class OH;
class GraphicsState;

class Page: public Content
{
private:
		Rect media_box, crop_box;
		GraphicsStateStack gs;
    
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

