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
#include "Page_TextObject.h"

namespace PDF {

Page::Page():m_debug(0),m_operators_number_limit(0)
{
  gs=new GraphicsState();
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
  // delete all forgotten graphics stack contents
  while(!gstack.empty()) { delete gstack.top(); gstack.pop(); }
  if(gs) delete gs;
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

void GraphicsState::TextState::dump(std::ostream & s) const {
	s << "Tc:" << Tc << " Tw:" << Tw << " Th:" << Th;
	s << " Tl:" << Tl << " Tfs:" << Tfs << " Trise:" << Trise;
	s << " Font:" << (Tf?Tf->name():"(null)");
}

void Page::draw(Media * m)
{
  Path * curpath=NULL;
  TextObject * tobj=NULL;

  enum { M_PAGE, M_PATH, M_TEXT, M_IMAGE } mode=M_PAGE;
  
	m->Size(media_box.size());
	gs->ctm = m->Matrix();

	unsigned int operators_num = operators.size();
	if(m_operators_number_limit && m_operators_number_limit < operators_num)
		operators_num = m_operators_number_limit;
  for(unsigned int operator_index = 0; operator_index < operators_num; operator_index++)
  {
    Operator * op = operators[operator_index];;
		if(1) {
			std::stringstream ss;
			ss << operator_index << ": " << op->dump();
			m->Debug(operator_index, ss.str(), *gs);
		}
//std::clog << "Operator " << operator_index << ": " << op->dump() << std::endl;

		/* Check for any-mode-operators */
		if(op->name() == "cm") {
			if(tobj)
				tobj->Flush();
			gs->ctm *= op->matrix();
			if(tobj)
				tobj->FontChanged();
			continue;
		}

    switch(mode)
    {
      case M_PAGE:
        if(op->name() == "m" || op->name() == "re") { mode=M_PATH; /* NO BREAK! */ }
        else if(op->name() == "BT") { mode=M_TEXT; break; }
        else if(op->name() == "BI") { mode=M_IMAGE; break; }
        else if(op->name() == "Do")
		{
			const Name * n = dynamic_cast<const Name *>(op->arg(0));
			if(!n)
				throw WrongPageException("XObject id is not a name");
			std::map<std::string, XObject*>::const_iterator it = xobjects.find(n->value());
			if(it == xobjects.end())
				throw UnimplementedException("XXX may be this xobject was not loaded");
			// 1. Saves the current graphics state, as if by invoking the q operator
			gstack.push(gs);
			gs = new GraphicsState(*gs);
			// 2. Concatenates the matrix from the form dictionary’s Matrix entry with the current transformation matrix (CTM)
			gs->ctm *= op->matrix();
			// 3. Clips according to the form dictionary’s BBox entry
			// XXX
			// 4. Paints the graphics objects specified in the form’s content stream
			// 5. Restores the saved graphics state, as if by invoking the Q operator
			if(gs) delete gs;
			gs = NULL;
			assert(!gstack.empty());
			if(!gstack.empty()) { gs = gstack.top(); gstack.pop(); }
			break;
		}
        else if(op->name() == "sh") { /* ignore it */ break; }
        else if(op->name() == "q")
        {
          gstack.push(gs);
          gs=new GraphicsState(*gs);
          break;
        }
        else if(op->name() == "Q")
        {
          if(gs) delete gs;
          if(!gstack.empty()) { gs=gstack.top(); gstack.pop(); }
          else {
						gs=new GraphicsState();
						std::cerr << "GraphicsState stack underrun" << std::endl;
						gs->ctm = m->Matrix();
					}
          break;
        } else if(op->name() == "Tf") // set current font (HACK!!! It should be done in text mode!!!)
		{
			std::clog << "Bad PDF: Tf outside text object!" << std::endl;
			const Name * n=dynamic_cast<const Name *>(op->arg(0));
			if(n) {
				std::map<std::string,Font *>::const_iterator it = fonts.find(n->value());
				if(it == fonts.end())
					throw DocumentStructureException(std::string("Font not found: ") + n->value());
				gs->text_state.Tf = it->second;
			}
			gs->text_state.Tfs=op->number(1);
			//gs->text_state.Tfs=abs(op->number(1)); // XXX WTF? Negative font sizes? Really?!?
			break;
		}
        else
				{
					m->Debug(operator_index, std::string("Ignoring operator ") + op->dump() + " in page mode", *gs);
					break;
				}
        /* NO BREAK!!! */
      case M_PATH: /*=============== path construction =====================*/
        if(op->name() == "m") // moveto
        {
          if(curpath)
          {
            /// \todo Implement full path with subpaths
            std::cerr << "Deleting live path!!!" << curpath->dump() << std::endl;
            delete curpath;
          }
          curpath=new Path;
          curpath->push_back(op->point(0));
        }
        else if(op->name() == "l") // lineto
        {
          if(curpath) curpath->push_back(op->point(0));
          else { std::cerr << "Line without path??" << std::endl; }
        }
				else if(op->name() == "re") // rectangle
				{
					if(!curpath) curpath=new Path;
					Point b = op->point(0);
					curpath->push_back(b);
					curpath->push_back(Point(b.x + op->number(2), b.y));
					curpath->push_back(Point(b.x + op->number(2), b.y + op->number(3)));
					curpath->push_back(Point(b.x, b.y + op->number(3)));
					curpath->push_back(b);
				}
				else if(op->name() == "W") // modify clipping path
				{
					gs->modify_clipping_path(curpath);
				}
        else if(op->name() == "h") // home
        {
          if(curpath && curpath->size()) curpath->push_back(curpath->at(0));
        }
        else if(op->name() == "c" || op->name() == "v" || op->name() == "y") // curves
        {
          std::cerr << "Beizer curves not implemented" << std::endl;
        }
        else if(op->name() == "n") // no-op
        {
          delete curpath; curpath=NULL;
          mode=M_PAGE;
        }
        else if(op->name() == "s" || op->name() == "S"
            || op->name() == "f" || op->name() == "F" || op->name() == "f*"
            || op->name() == "B" || op->name() == "B*"
            || op->name() == "b" || op->name() == "b*")
        {
					if(curpath) {
						/* paint path */
#if 0
						std::clog << "Drawing path: " << curpath->dump() << std::endl;
#endif
						for(unsigned int i=0; i<curpath->size()-1; i++)
						{
							m->Line(gs->ctm.translate(curpath->at(i)), gs->ctm.translate(curpath->at(i+1)), *gs);
						}
						if(op->name() == "s" || op->name() == "f")
							m->Line(gs->ctm.translate(curpath->at(curpath->size()-1)), gs->ctm.translate(curpath->at(0)), *gs);
						delete curpath; curpath=NULL;
					} else {
						std::cerr << "Attempted to draw (" << op->dump() << ") non-existent path" << std::endl;
					}
          mode=M_PAGE;
        }
        else
				{
					m->Debug(operator_index, std::string("Ignoring operator ") + op->dump() + " in path construction mode", *gs);
				}
        break;
      case M_TEXT:
        if(!tobj) tobj=new TextObject(gs, m);
        /* Check text showing operators first */
				if(op->name() == "Tj") // output text
				{
					if(!gs->text_state.Tf)
						throw WrongPageException("No font set");
					const String * str=dynamic_cast<const String *>(op->arg(0));
					if(str)
						tobj->Append(str);
					break;
				}
				else if(op->name() == "TJ")
				{
					const Array * a=dynamic_cast<const Array *>(op->arg(0));
					if(!a)
						throw WrongPageException("TJ with no array");
					for(Array::ConstIterator it=a->get_const_iterator(); a->check_iterator(it); it++) {
						const String * str=dynamic_cast<const String *>(*it);
						if(str) tobj->Append(str);
						else {
							const Integer * i = dynamic_cast<const Integer *>(*it);
							const Real * r = dynamic_cast<const Real *>(*it);
							if(i) tobj->Kerning(i->value()/1000.0);
							else if(r) tobj->Kerning(r->value()/1000.0);
							else
								std::cerr << "Unexpected object " << (*it)->type() << " in string (TJ)" << std::endl;
						}
					}
					break;
				} // TJ
				else if(op->name() == "\'" || op->name() == "\"")
				{
					const String * str=dynamic_cast<const String *>( op->arg((op->name()=="\"")?2:0) );
					if(op->name() == "\"") {
						gs->text_state.Tw = op->number(0);
						gs->text_state.Tc = op->number(1);
					}
					if(str)
					{
						tobj->NewLine();
						tobj->Append(str);
					}
					break;
				}
        else if(op->name() == "T*") // XXX it is not a text-showing operator!
        {
					tobj->NewLine();
					break;
        }
        else
        {
					tobj->Flush();
					// ... and executete other operators
        }
        
        /* check other text object operators */
				if(op->name() == "ET") { if(tobj) delete tobj; tobj=NULL; mode=M_PAGE; }
				else if(op->name() == "Tc") { gs->text_state.Tc    =  op->number(0);} // char spacing
				else if(op->name() == "Ts") { gs->text_state.Trise = op->number(0); } // rise
				else if(op->name() == "Tz") { gs->text_state.Th    = op->number(0); } // hor. scaling
				else if(op->name() == "Tw") { gs->text_state.Tw    = op->number(0); } // word spacing - useful if text begins with space(s)
				else if(op->name() == "Tf") // set current font
				{
					tobj->Flush();
					const Name * n=dynamic_cast<const Name *>(op->arg(0));
					if(n) {
						std::map<std::string,Font *>::const_iterator it = fonts.find(n->value());
						if(it == fonts.end())
							throw DocumentStructureException(std::string("Font not found: ") + n->value());
						gs->text_state.Tf = it->second;
					}
					gs->text_state.Tfs=op->number(1);
					tobj->FontChanged();
        }
        else if(op->name() == "Tm")
        {
					tobj->SetMatrix( op->matrix() );
        }
        else if(op->name() == "Td")
        {
					tobj->Offset(op->point(0));
        }
				else if(op->name() == "TL")
				{
					gs->text_state.Tl=op->number(0);
				}
				else if(op->name() == "TD") // tx ty TD === -ty TL; tx ty Td
				{
					tobj->Offset(op->point(0));
					gs->text_state.Tl=-op->number(1);
				}
        else
				{
					m->Debug(operator_index, std::string("Ignoring operator ") + op->dump() + " in text mode", *gs);
				}
        break;
      case M_IMAGE:
        if(op->name() == "EI") mode=M_PAGE;
        else /* skip everything about inline image */;
        break;
      default:
        throw WrongPageException("Default reached in switch(mode){...} while drawing page");
        break;
    } // switch(mode)
  }
	/* Output some debugging information */
	if(m_operators_number_limit) {
		std::ostringstream ss;
		ss << "=== Page drawing finished (" << operators_num << " operators executed) ===" << std::endl;
		if(gs) {
			ss << "Graphics state:";
			gs->dump(ss);
			ss << std::endl;
		}
		if(tobj) {
			ss << "Line matrix:" << std::endl << tobj->lm.dump();
			ss << "Text matrix:" << std::endl << tobj->tm.dump();
		}
		if(curpath)
			ss << "Current path: " << curpath->dump() << std::endl;
		if(m_operators_number_limit < operators.size())
			ss << "Next operator: " << operators[m_operators_number_limit]->dump() << std::endl;
		m->Debug(operators_num, ss.str(), *gs);
	}
	if(tobj) delete tobj;
	if(curpath) delete curpath;
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

PDF::XObject* PDF::XObject::create(std::string name, PDF::OH definition)
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

void PDF::FormXObject::load(PDF::OH dic)
{
	// Matrix, BBox
	// Resources - use page's unless defined
}

