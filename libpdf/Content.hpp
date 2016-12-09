#ifndef CONTENT_HPP_INCLUDED
#define CONTENT_HPP_INCLUDED

#include "Path.hpp"
#include "GraphicsState.hpp"

namespace PDF {

class XObject;
class Object;
class Media;

/** \brief Represents page content
 */
class Content
{
public:
	class Operator;
	class Render;
	class TextObject;
	struct ResourceProvider
	{
		virtual Font* get_font(std::string name) = 0;
		virtual XObject* get_xobject(std::string name) = 0;
	};
public:
	Content();
	~Content();
	bool parse(const std::vector<char> & data);
	size_t get_operators_count() const { return operators.size(); }
	std::streamoff get_operator_offset(unsigned int n) const;
protected:
	void draw(ResourceProvider& res, Media& m, unsigned int operators_num);
	GraphicsStateStack gs;
private:
	std::vector<Operator *> operators;
};

class Content::Operator
{
private:
	std::streamoff m_offset;
	std::string m_name;
	std::vector<Object *> * m_args;
public:
	Operator(std::streamoff offset, std::string op, std::vector<Object *>* a);
	~Operator();
	std::string name() const { return m_name; }
	std::streamoff offset() const { return m_offset; }
	bool operator == (const char * cmp) { return m_name == cmp; }
	const Object * arg(unsigned int i) const { return i >= m_args->size() ? NULL : m_args->at(i); }
	const Object * operator[](unsigned int i) const { return arg(i); }
	double number(unsigned int i) const;
	Point point(unsigned int i) const
	{
		return Point(number(i), number(i + 1));
	}
	CTM matrix() const
	{
		return CTM(number(0), number(1), number(2), number(3), number(4), number(5));
	}
	std::string dump() const;
};

class Content::Render
{
public:
	enum Mode { M_PAGE, M_PATH, M_TEXT, M_IMAGE };
public:
	Render(GraphicsStateStack& gs, ResourceProvider& res, Media& m);
	~Render();
	bool draw(const Operator& op);
	void dump(std::ostream& s);
	const char* mode_string() const;
protected:
	bool draw_page_mode(const Operator& op);
	bool draw_path_mode(const Operator& op);
	bool draw_text_mode(const Operator& op);
	bool draw_image_mode(const Operator& op);
private:
	GraphicsStateStack& gs;
	ResourceProvider& res;
	Media& m;
	Mode mode;
	Path* curpath;
	TextObject* tobj;
};

} /* namespace PDF */

#endif /* CONTENT_HPP_INCLUDED */

