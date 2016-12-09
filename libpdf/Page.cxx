// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include <iostream>
#include <string>
#include "Page.hpp"
#include "OH.hpp"
#include "Font.hpp"
#include "Object.hpp"
#include "Media.hpp"
#include "ObjStrm.hpp"
#include "Exceptions.hpp"

namespace PDF {

Page::Page():m_debug(0),m_operators_number_limit(0)
{
}

Page::~Page()
{
  /// \todo move to 'foreach'
  // delete all fonts
  for(std::map<std::string,Font *>::iterator it=fonts.begin(); it!=fonts.end(); it++) delete it->second;
  // delete all xobjects
  for(std::map<std::string, XObject *>::iterator it = xobjects.begin(); it != xobjects.end(); it++) delete it->second;
  // delete all operators
  for(unsigned int i=0;i<operators.size(); i++) { delete operators[i]; }
}

static Rect get_box(OH boxnode)
{
	Array * a;
	boxnode.put(a, "*Box is not an array ?!?");
	if(a->size()!=4)
		throw WrongPageException("Rect. array must contin 4 elements");
	double da[4];
	for(int i=0; i<4; i++) {
		Real * realn=dynamic_cast<Real *>(a->at(i));
		Integer * intgr=dynamic_cast<Integer *>(a->at(i));
		if(realn) {
			da[i]=realn->value();
		} else if(intgr) {
			da[i]=intgr->value();
		} else
			throw WrongPageException("Not a number in rectangle array?!?");

	}
	return Rect(da);
}

/** \brief Loads all page data from page node
 */
bool Page::load(OH pagenode)
{
  if(m_debug) std::clog << "Page node: " << pagenode.obj()->dump() << std::endl;
  // find page size
  {
    OH mediabox_h=pagenode.find("MediaBox");
		if(mediabox_h) {
			media_box = get_box(mediabox_h);
		}
    OH cropbox_h=pagenode.find("CropBox");
		if(cropbox_h) {
			crop_box = get_box(cropbox_h);
		} else {
			crop_box = media_box;
		}
  }
  // load contents
  {
    OH contents_h=pagenode.find("Contents");
    contents_h.expand();
    if(!contents_h)
    {
      std::cerr << "Page: no contents" << std::endl;
    }
    else
		{
			std::vector<char> pagedata;
			Stream * stream;
			if((stream=dynamic_cast<Stream *>(contents_h.obj()))) {
				stream->get_data(pagedata);
			} else {
				if(!dynamic_cast<Array *>(contents_h.obj()))
					throw WrongPageException("Page content is not a Stream and not an Array. That is wrong. I give up.");
				std::vector<char> data;
				for(unsigned int i=0; i<contents_h.size(); i++) {
					OH s = contents_h[i];
					s.expand();
					s.put(stream, "Page content is not a stream?!?!!?");
					stream->get_data(data);
					pagedata.insert(pagedata.end(), data.begin(), data.end());
				}
			}
			parse(pagedata);
		}
  }
  
  // load resources
  {
    OH resources_h=pagenode.find("Resources");
    if(!resources_h)
    {
      std::clog << "? Page: no resources" << std::endl;
    }
    else
    {
      // load fonts
      resources_h.expand();
      OH fonts_h=resources_h.find("Font");
      if(fonts_h)
      {
				Dictionary * fonts_d;
				fonts_h.put(fonts_d, "Font's node is not a Dictionary but is a ");

        // load all fonts' objects
        for( Dictionary::Iterator it=fonts_d->get_iterator(); fonts_d->check_iterator(it); it++)
        {
          OH font_h=fonts_h.dig(it->second);
          Font * f=new Font(it->first);
          f->load(font_h);
          fonts[it->first]=f;
        }
      }
      else
      {
        std::clog << "No fonts found" << std::endl;
      }

	  OH xobjs_h = resources_h.find("XObject");
	  if(xobjs_h)
	  {
		Dictionary * xobjs_d;
		xobjs_h.put(xobjs_d, "XObject node is not a Dictionary but is a ");
		for(Dictionary::Iterator it = xobjs_d->get_iterator(); xobjs_d->check_iterator(it); it++)
		{
			XObject * x = XObject::create(it->first, xobjs_h.dig(it->second));
			if(x)
				xobjects[it->first] = x;
		}

	  }
    }
  }

  return true;
}

/** \brief Parses page data to sequence of operators, stored in Page object
 */
bool Page::parse(const std::vector<char> & data)
{
  std::stringstream ss(std::string(data.begin(), data.end()));
	ObjIStream strm(ss);
	strm.throw_eof(false);
	strm.allow_keywords(true);

  std::vector<Object *> * args=NULL;
  Object * o=NULL;
  while((o = strm.read_direct_object()))
  {
    Keyword * kw=dynamic_cast<Keyword *>(o);
    if(kw)
    {
		std::streamoff offs = kw->m_offset;
      if(args)
          offs = (*args->begin())->m_offset;
      operators.push_back(new Operator(offs, kw->value(), args));
      args = NULL;
      delete kw; // name already copied
    }
    else
    {
      if(!args) args=new std::vector<Object *>;
      args->push_back(o);
    }
  }
  assert(!args);

  // delete tail of (broken?) content stream
  if(args)
  {
    for(std::vector<Object *>::iterator it=args->begin(); it!=args->end(); it++) delete *it;
    delete args;
  }
  
  return true;
}

std::string Page::dump() const
{
  std::stringstream ss;
  ss << "Page dump: " << std::endl;
  ss << "\t" << operators.size() << " operators" << std::endl;
  ss << "\t" << fonts.size() << " fonts" << std::endl;
  for(std::map<std::string,Font *>::const_iterator it=fonts.begin(); it!=fonts.end(); it++)
  {
    ss << "Font " << std::setw(5) << std::left << it->first << it->second->dump();
  }
  ss << "Graphics state:" << std::endl;
  
  return ss.str();
}

class SimplePageResourceProvider : public Page::ResourceProvider
{
public:
	SimplePageResourceProvider(std::map<std::string, Font *>& fonts, std::map<std::string, XObject *>& xobjects)
		: fonts(fonts), xobjects(xobjects)
	{
	}
	virtual Font* get_font(std::string name)
	{
		std::map<std::string, Font *>::const_iterator it = fonts.find(name);
		if(it == fonts.end())
			throw DocumentStructureException(std::string("Font not found: ") + name);
		return it->second;
	}
	virtual XObject* get_xobject(std::string name)
	{
		std::map<std::string, XObject*>::const_iterator it = xobjects.find(name);
		return it == xobjects.end() ? NULL : it->second;
	}
private:
	std::map<std::string, Font *>& fonts;
	std::map<std::string, XObject *>& xobjects;
};

void GraphicsState::TextState::dump(std::ostream & s) const {
	s << "Tc:" << Tc << " Tw:" << Tw << " Th:" << Th;
	s << " Tl:" << Tl << " Tfs:" << Tfs << " Trise:" << Trise;
	s << " Font:" << (Tf?Tf->name():"(null)");
}

void Page::draw(Media * m)
{

	m->Size(media_box.size());
	gs->ctm = m->Matrix();

	assert(m);
	SimplePageResourceProvider res(fonts, xobjects);
	Render r(gs, res, *m);

	unsigned int operators_num = operators.size();
	if(m_operators_number_limit && m_operators_number_limit < operators_num)
		operators_num = m_operators_number_limit;
	for(unsigned int operator_index = 0; operator_index < operators_num; operator_index++)
	{
		Operator * op = operators[operator_index];
		if(1) {
			std::stringstream ss;
			ss << operator_index << ": " << op->dump();
			m->Debug(operator_index, ss.str(), gs);
		}
		if(! r.draw(*op))
			m->Debug(operator_index, std::string("Ignoring operator ") + op->dump() + " in " + r.mode_string() + " mode", gs);
	}
	/* Output some debugging information */
	if(m_operators_number_limit) {
		std::ostringstream ss;
		ss << "=== Page drawing finished (" << operators_num << " operators executed) ===" << std::endl;
		gs.dump(ss);
		r.dump(ss);
		if(m_operators_number_limit < operators.size())
			ss << "Next operator: " << operators[m_operators_number_limit]->dump() << std::endl;
		m->Debug(operators_num, ss.str(), gs);
	}
}

std::streamoff Page::get_operator_offset(unsigned int n) const
{
	if(n < operators.size()) {
		Operator* o = operators.at(n);
		return o->offset();
	}
	return 0;
}

/*============== Page::Operator ========================*/

double Page::Operator::number( unsigned int i ) const
{
	const Object * o=arg(i);
	if(!o)
		throw WrongPageException("Can not find numeric argument number ") << i;
	const Real * real=dynamic_cast<const Real *>(o);
	const Integer * integer=dynamic_cast<const Integer *>(o);
	if(!real && !integer)
		throw WrongPageException("Argument ") << i << " (" << o->dump() << ") is not a number";
	return real?real->value():integer->value();
}

std::string Page::Operator::dump() const
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

}; // namespace PDF

#include "OH.hpp"

PDF::XObject* PDF::XObject::create(std::string name, OH definition)
{
	PDF::Stream* s = NULL;
	definition.expand();
	definition.put(s, "XObject description is not a stream");
	PDF::Name* subtype = dynamic_cast<PDF::Name*>(s->dict()->find("Subtype"));
	if(!subtype)
		return NULL;
	if(subtype->value() == "Form")
	{
		FormXObject* f = new FormXObject(name);
		f->load(definition);
		return f;
	}
	return NULL;
}

PDF::FormXObject::FormXObject(std::string name)
	: XObject(name)
{
}

void PDF::FormXObject::load(OH dic)
{
	// Matrix, BBox
	// Resources - use page's unless defined
}

void PDF::FormXObject::draw(Page::Render & r)
{
}

