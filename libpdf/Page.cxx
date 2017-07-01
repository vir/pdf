#ifdef _MSC_VER
# pragma warning(disable : 4786) // get rid of "identifier was truncated to '255' characters..."
# ifdef _DEBUG
#  ifdef _CRTDBG_MAP_ALLOC
#   include <stdlib.h>  
#   include <crtdbg.h>  
#   ifndef DBG_NEW
#    define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#    define new DBG_NEW
#   endif
#  endif
# endif // _DEBUG
#endif

#include <iostream>
#include <string>
#include "Page.hpp"
#include "OH.hpp"
#include "Font.hpp"
#include "Object.hpp"
#include "Exceptions.hpp"
#include "Media.hpp"

namespace PDF {
PageResourceProvider::~PageResourceProvider()
{
	// delete all fonts
	for(std::map<std::string, Font *>::iterator it = fonts.begin(); it != fonts.end(); it++) delete it->second;
	// xobjects will destroy itself
}

inline void PageResourceProvider::load(OH resources_h)
{
	// load fonts
	OH fonts_h = resources_h.find("Font");
	if(fonts_h)
	{
		Dictionary * fonts_d;
		fonts_h.put(fonts_d, "Font's node is not a Dictionary but is a ");

		// load all fonts' objects
		for(Dictionary::Iterator it = fonts_d->get_iterator(); fonts_d->check_iterator(it); it++)
		{
			OH font_h = fonts_h.dig(it->second);
			Font * f = new Font(it->first);
			f->load(font_h);
			fonts[it->first] = f;
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
			xobjects[it->first] = xobjs_h.find(it->first);
	}
}

Font * PageResourceProvider::get_font(std::string name)
{
	std::map<std::string, Font *>::const_iterator it = fonts.find(name);
	if(it == fonts.end())
		throw DocumentStructureException(std::string("Font not found: ") + name);
	return it->second;
}

XObject * PageResourceProvider::get_xobject(std::string name)
{
	std::map<std::string, OH>::const_iterator it = xobjects.find(name);
	if(it == xobjects.end())
		return NULL;
	return XObject::create(it->first, it->second);
}

void PageResourceProvider::dump(std::ostream & ss) const
{
	ss << "Resources: " << fonts.size() << " fonts" << std::endl;
	for(std::map<std::string, Font *>::const_iterator it = fonts.begin(); it != fonts.end(); it++)
		ss << "Font " << std::setw(5) << std::left << it->first << it->second->dump();
}



Page::Page():m_debug(0),m_operators_number_limit(0)
{
}

Page::~Page()
{
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
		resources_h.expand();
		resources.load(resources_h);
    }
  }

  return true;
}

std::string Page::dump() const
{
  std::stringstream ss;
  ss << "Page dump: " << std::endl;
  ss << "\t" << get_operators_count() << " operators" << std::endl;
  resources.dump(ss);
  ss << "Graphics state:" << std::endl;
  
  return ss.str();
}

void GraphicsState::TextState::dump(std::ostream & s) const {
	s << "Tc:" << Tc << " Tw:" << Tw << " Th:" << Th;
	s << " Tl:" << Tl << " Tfs:" << Tfs << " Trise:" << Trise;
	s << " Font:" << (Tf?Tf->name():"(null)");
}

void Page::draw(Media * m)
{
	assert(m);

	m->Size(media_box.size());
	gs->ctm = m->Matrix();

	unsigned int operators_num = get_operators_count();
	if(m_operators_number_limit && m_operators_number_limit < operators_num)
		operators_num = m_operators_number_limit;

	this->PDF::Content::draw(resources, *m, operators_num);
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
	if(subtype->value() == "Image")
	{
		ImageXObject* f = new ImageXObject(name);
		f->load(definition);
		return f;
	}
	return NULL;
}

PDF::FormXObject::FormXObject(std::string name)
	: XObject(name)
{
}

void PDF::FormXObject::load(OH strm)
{
	OH dic = strm.dig(strm.cast<Stream*>("Not a stream")->dict());
	// Matrix, BBox
	{
		OH bbox_h = dic.find("BBox");
		if(bbox_h)
			bbox = get_box(bbox_h);
		OH matrix_h = dic.find("Matrix");
		if(matrix_h && matrix_h.size() == 6)
			xobjctm = CTM(matrix_h[0].numvalue(), matrix_h[1].numvalue(),
				matrix_h[2].numvalue(), matrix_h[3].numvalue(),
				matrix_h[4].numvalue(), matrix_h[5].numvalue());
	}
	// load contents
	{
		std::vector<char> pagedata;
		Stream * stream;
		strm.put(stream, "Form XObject is not a stream");
		stream->get_data(pagedata);
		parse(pagedata);
	}

	// load resources
	{
		OH resources_h = dic.find("Resources");
		// XXX Resources - use page's unless defined
		if(!resources_h)
			throw UnimplementedException("Should use Page's resource");
		resources_h.expand();
		resources.load(resources_h);
	}
	
}

void PDF::FormXObject::draw(GraphicsStateStack& parentgs, Content::ResourceProvider& res, Media& m, Content::Render& r)
{
	gs.inherit(parentgs);
	this->Content::draw(resources, m, get_operators_count());
}

PDF::ImageXObject::ImageXObject(std::string name)
	: XObject(name)
{
}

void PDF::ImageXObject::load(OH dic)
{
	def = dic;
}

void PDF::ImageXObject::draw(PDF::GraphicsStateStack & gs, PDF::Content::ResourceProvider & res, PDF::Media & m, PDF::Content::Render & r)
{
	PDF::CTM ctm(gs->ctm);
	PDF::Rect rect(ctm.translate(PDF::Point(0.0, 0.0)), ctm.translate(PDF::Point(1.0, 1.0)));
	PDF::Stream* strm;
	def.put(strm);
	m.Image(rect, *strm, gs);
}

