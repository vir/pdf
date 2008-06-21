#include <iostream>
#include <string>

#include "Page.hpp"
#include "OH.hpp"
#include "Font.hpp"
#include "Object.hpp"
#include "Media.hpp"

#define TC_IS_KERNING 1

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
					throw std::string("Page content is not a Stream and not an Array. That is wrong. I give up.");
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
		const Page::GraphicsState * gs;
		Media * media;
		double kerning_too_big;
	public:
		CTM tm;
		CTM lm;
		std::wstring accumulated_text; // work around multiple Tj in one text object
		double total_width;
		bool update_font;

		TextObject(const Page::GraphicsState * g, Media * m):gs(g),media(m),total_width(0),update_font(false)
		{
			kerning_too_big = 0.3; //0.4;
		}
		void SetMatrix(const CTM & m)
		{
			tm = lm = m;
			update_font = true;
		}
		void Append(const String * str)
		{
			unsigned int pos = 0;
#if TC_IS_KERNING
			while(gs->text_state.Tf->extract_one_char(str, accumulated_text, total_width, pos)) {
				double plusw = gs->text_state.Tc;
				if(accumulated_text[accumulated_text.length()-1] == L' ')
					plusw += gs->text_state.Tw;
				if(plusw)
					Kerning(-(plusw / gs->text_state.Tfs));
			}
#else
			double w = 0;
			std::wstring s;
			if(!gs->text_state.Tf)
				throw std::string("No font set");
			if(gs->text_state.Tw != 0) { /* output words separately appending Tw space between */
				while( gs->text_state.Tf->extract_text(str, s, w, pos, L' ') ) {
					accumulated_text+=s;
					total_width+=w;
					Flush();
					s.resize(0);
					tm.offset_unscaled(gs->text_state.Tw*gs->text_state.Th/100.0, 0);
				}
			} else {
				gs->text_state.Tf->extract_text(str, s, w, pos);
			}
			accumulated_text+=s;
			total_width+=w;
#endif
		}
		void Kerning(double k) { // offset next char back by k glyph space units
			if(k > kerning_too_big || k < -kerning_too_big) { // XXX TODO caompare with avg charwidth or so
				Flush();
				double offset = -k * gs->text_state.Tfs * gs->text_state.Th/100.0;
				tm.offset_unscaled(offset, 0);
			} else {
				total_width-=k;
			}
		}
		void NewLine() {
			Flush();
			lm.offset_unscaled(0, -gs->text_state.Tl); // XXX +Tl in spec!!! BUG?
			tm = lm;
		}
		void Offset(const Point & p) {
			Flush();
			lm.offset_unscaled(p);
			tm = lm;
		}
		void Flush() {
			if(accumulated_text.empty()) return;
			CTM m(gs->text_state.Tfs * gs->text_state.Th/100.0, 0, 0, gs->text_state.Tfs, 0, gs->text_state.Trise);
			CTM Trm = m * tm * gs->ctm; // Construct text rendering matrix
			if(update_font) {
				media->SetFont(gs->text_state.Tf, Trm.get_scale_v());
				update_font = false;
			}
#if TC_IS_KERNING
			double offset = (total_width*gs->text_state.Tfs) * gs->text_state.Th/100.0;
#else
			double offset = (total_width*gs->text_state.Tfs + gs->text_state.Tc*accumulated_text.length()) * gs->text_state.Th/100.0;
#endif
			media->Text(
				Trm.translate(Point(0,0)),
				Trm.get_rotation_angle(),
				accumulated_text,
				total_width*Trm.get_scale_h(),
				Trm.get_scale_v()
			);
			tm.offset_unscaled(offset, 0);
			accumulated_text.resize(0);
			total_width = 0;
		}
		void FontChanged()
		{
			update_font = true;
		}
};

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
						tobj->Append(str);
					break;
				}
				else if(op->name() == "TJ")
				{
					const Array * a=dynamic_cast<const Array *>(op->arg(0));
					if(!a)
						throw std::string("TJ with no array");
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
          const Name * n=dynamic_cast<const Name *>(op->arg(0));
          if(n) gs->text_state.Tf=fonts.find(n->value())->second;
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
			if(gs) {
				std::clog << "Text state: ";
				gs->text_state.dump(std::clog);
				std::clog << std::endl;
			}
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

