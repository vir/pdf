#include <iostream>
#include <string>

#include "Page.hpp"
#include "OH.hpp"
#include "Font.hpp"
#include "Object.hpp"
#include "Media.hpp"

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
		throw std::string("Rect. array must contin 4 elements");
	double da[4];
	for(int i=0; i<4; i++) {
		Real * rn=dynamic_cast<Real *>(a->at(i));
		if(rn) {
			da[i]=rn->value();
		} else {
			Integer * in=dynamic_cast<Integer *>(a->at(i));
			if(!in) throw std::string("Not a number in rectangle array?!?");
			da[i]=in->value();
		}
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
    OH mediabox_h=pagenode.find("CropBox");
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
					throw std::string("Page content is not a Stream and not an Array. That is wrong. I give up.");
				std::vector<char> data;
				for(unsigned int i=0; i<contents_h.size(); i++) {
					OH s = contents_h[i];
					s.expand();
					stream=s.cast<Stream *>("Page content is not a stream?!?!!?");
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
    }
  }

  return true;
}

/** \brief Parses page data to sequence of operators, stored in Page object
 */
bool Page::parse(const std::vector<char> & data)
{
  std::stringstream ss(std::string(data.begin(), data.end()));

  std::vector<Object *> * args=NULL;
  Object * o=NULL;
  while((o=Object::read(ss, true)))
  {
    Keyword * kw=dynamic_cast<Keyword *>(o);
    if(kw)
    {
      operators.push_back(new Operator(kw->value(), args));
      args=NULL;
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
    ss << "Font " << it->first << ": "  << it->second->dump();
  }
  return ss.str();
}

/** Work around mutiple text-showing operators in same text object.
 *  if so, concatenate them and output them all together.
 */
class Page::TextObject
{
	private:
		const static bool use_advanced_algorithm;
		const Page::GraphicsState * gs;
		Media * media;
	public:
		CTM tm;
		CTM lm;
		std::wstring accumulated_text; // work around multiple Tj in one text object
		double total_width;

		TextObject(const Page::GraphicsState * g, Media * m):gs(g),media(m),total_width(0) {}
		void Append(const String * str)
		{
			double w = 0;
			String tmp;
			if(use_advanced_algorithm) {
				if(/*!gs->text_state.Tf->is_multibyte() &&*/ gs->text_state.Tw != 0) { /* output words separately appending Tw space between */
					tmp = *str;
					String * sp;
					while((sp = tmp.cut_word())) {
						std::wstring s=gs->text_state.Tf->extract_text(sp, &w);
						w+=s.length() * gs->text_state.Tc; // XXX TODO Take Th into account!
						accumulated_text+=s;
						total_width+=w;
						Flush();
						tm.offset_unscaled(gs->text_state.Tw, 0);
						delete sp;
					}
					str = &tmp; /* tmp shuld not go out of scope! */
				}
			}
			std::wstring s=gs->text_state.Tf->extract_text(str, &w);
			accumulated_text+=s;
			w+=s.length() * gs->text_state.Tc; // XXX TODO Take Th into account!
			total_width+=w;
		}
		void Kerning(double k) { // offset next char back by k glyph space units
			k/=1000;
			if(k > 0.5 || k < -0.5) { // XXX TODO caompare with avg charwidth or so
				Flush();
				tm.offset_unscaled(-k, 0);
			} else {
				total_width-=k;
			}
		}
		void NewLine() {
			accumulated_text+='\n';
			total_width = 0;
			lm.offset_unscaled(0, gs->text_state.Tl);
			tm = lm;
		}
		void Offset(const Point & p) {
//		CTM m(1, 0, 0, 1, op->number(0), op->number(1)); tm = lm = m * lm;
			Flush();
			lm.offset_unscaled(p);
			tm = lm;
		}
		void Flush() {
			if(accumulated_text.empty()) return;
			CTM m = tm * gs->ctm; // Construct text rendering matrix
			media->Text(
				m.translate(Point(0,0)),
				m.get_rotation_angle(),
				accumulated_text
			);
			tm.offset_unscaled(total_width, 0);
			accumulated_text.resize(0);
			total_width = 0;
		}
};
const bool Page::TextObject::use_advanced_algorithm = true;

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
			m->Debug(ss.str());
		}
//std::clog << "Operator " << operator_index << ": " << op->dump() << std::endl;

		/* Check for any-mode-operators */
		if(op->name() == "cm") {
			gs->ctm *= op->matrix();
			continue;
		}

    switch(mode)
    {
      case M_PAGE:
        if(op->name() == "m" || op->name() == "re") { mode=M_PATH; /* NO BREAK! */ }
        else if(op->name() == "BT") { mode=M_TEXT; break; }
        else if(op->name() == "BI") { mode=M_IMAGE; break; }
        else if(op->name() == "Do" || op->name() == "sh") { /* ignore it */ break; }
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
        }
        else
				{
					m->Debug(std::string("Ignoring operator ") + op->dump() + " in page mode");
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
							m->Line(gs->ctm.translate(curpath->at(i)), gs->ctm.translate(curpath->at(i+1)));
						}
						if(op->name() == "s" || op->name() == "f")
							m->Line(gs->ctm.translate(curpath->at(curpath->size()-1)), gs->ctm.translate(curpath->at(0)));
						delete curpath; curpath=NULL;
					} else {
						std::cerr << "Attempted to draw (" << op->dump() << ") non-existent path" << std::endl;
					}
          mode=M_PAGE;
        }
        else
				{
					m->Debug(std::string("Ignoring operator ") + op->dump() + " in path construction mode");
				}
        break;
      case M_TEXT:
        if(!tobj) tobj=new TextObject(gs, m);
        /* Check text showing operators first */
        if(op->name() == "Tj") // output text
        {
          if(!gs->text_state.Tf) throw std::string("No font set");
          const String * str=dynamic_cast<const String *>(op->arg(0));
          if(str)
          {
						tobj->Append(str);
          }
          break;
        }
        else if(op->name() == "TJ")
        {
          if(!gs->text_state.Tf) throw std::string("No font set");
          const Object * o=op->arg(0);
          const Array * a=dynamic_cast<const Array *>(o);
          if(a)
          {
            for(Array::ConstIterator it=a->get_const_iterator(); a->check_iterator(it); it++)
            {
              const String * str=dynamic_cast<const String *>(*it);
              if(str) tobj->Append(str);
							else {
								const Integer * i = dynamic_cast<const Integer *>(*it);
								const Real * r = dynamic_cast<const Real *>(*it);
								if(i) tobj->Kerning(i->value());
								else if(r) tobj->Kerning(r->value());
								else {
									std::cerr << "Unexpected object " << (*it)->type() << " in string (TJ)" << std::endl;
								}
							}
            }
          }
          break;
        }
        else if(op->name() == "\'" || op->name() == "\"")
        {
          if(!gs->text_state.Tf) throw std::string("No font set");
          const Object * o=op->arg((op->name()=="\"")?2:0);
          const String * str=dynamic_cast<const String *>(o);
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
        else if(op->name() == "Tc") {} // not very useful char spacing
        else if(op->name() == "Ts" || op->name() == "Tz") {} // also, rise and scaling
        else if(op->name() == "Tr") // char spacing
				{
					gs->text_state.Tc=op->number(0);
				}
				else if(op->name() == "Tw") // set word spacing - useful if text begins with space(s)
				{
					gs->text_state.Tw=op->number(0);
				}
        else if(op->name() == "Tf") // set current font
        {
          const Object * o=op->arg(0);
          const Name * n=dynamic_cast<const Name *>(o);
//          std::clog << "Set current font to " << n->value() << std::endl;
          if(n) gs->text_state.Tf=fonts.find(n->value())->second;
          gs->text_state.Tfs=op->number(1);
					m->SetFont(gs->text_state.Tf, gs->text_state.Tfs);
        }
        else if(op->name() == "Tm")
        {
					tobj->Flush();
					tobj->tm = tobj->lm = op->matrix();
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
					m->Debug(std::string("Ignoring operator ") + op->dump() + " in text mode");
				}
        break;
      case M_IMAGE:
        if(op->name() == "EI") mode=M_PAGE;
        else /* skip everything about inline image */;
        break;
      default:
        throw std::string("Default reached in switch(mode){...} while drawing page");
        break;
    } // switch(mode)
  }
	/* Output some debugging information */
	if(m_operators_number_limit) {
		std::clog << "=== Page drawing finished (" << operators_num << " operators executed) ===" << std::endl;
		if(gs)
			std::clog << "CTM:" << std::endl << gs->ctm.dump();
		if(tobj) {
			std::clog << "Line matrix:" << std::endl << tobj->lm.dump();
			std::clog << "Text matrix:" << std::endl << tobj->tm.dump();
		}
		if(curpath)
			std::clog << "Current path: " << curpath->dump() << std::endl;
		if(m_operators_number_limit < operators.size()) {
			std::clog << "Next operator: " << operators[m_operators_number_limit]->dump() << std::endl;
		}
	}
	if(tobj) delete tobj;
	if(curpath) delete curpath;
}

}; // namespace PDF

