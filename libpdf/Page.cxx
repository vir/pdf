

//#include <fstream>
//#include <string>
//#include <map>
//#include <vector>
//#include <sstream>
#include <iostream>
#include <string>

#include "Page.hpp"
#include "OH.hpp"
#include "Font.hpp"
#include "Object.hpp"
#include "Media.hpp"

namespace PDF {

Page::Page():m_debug(0)
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
	boxnode.expand();
	Array * a=boxnode.cast<Array *>("*Box is not an array ?!?");
	if(a->size()!=4) throw std::string("Rect. array must contin 4 elements");
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
//  std::clog << "@Page::load()" << std::endl;
//  std::cerr << "Page node type: " << pagenode.obj()->type() << std::endl;
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
    std::clog << "Contents object: " << contents_h->dump() << std::endl;
    contents_h.expand();
    if(!contents_h)
    {
      std::clog << "? Page: no contents" << std::endl;
    }
    else
		{
			std::vector<char> pagedata;
			Stream * stream;
			if((stream=dynamic_cast<Stream *>(contents_h.obj()))) {
				stream->get_data(pagedata);
			} else {
				contents_h.cast<Array *>("Page content is not a Stream and not an Array. That is wrong. I give up.");
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
//      std::clog << "Found resources:" << std::endl;
      // load fonts
      resources_h.expand();
      OH fonts_h=resources_h.find("Font");
      if(!!fonts_h)
      {
        Dictionary * fonts_d=fonts_h.cast<Dictionary *>("Fonts node is not a Dictionary");
        if(m_debug>1) std::clog << "Page fonts: " << fonts_d->dump() << std::endl;

#if 1 // load fonts
        // load all fonts' objects
        for( Dictionary::Iterator it=fonts_d->get_iterator(); fonts_d->check_iterator(it); it++)
        {
          OH font_h=fonts_h.dig(it->second);
          Font * f=new Font(it->first);
          f->load(font_h);
          fonts[it->first]=f;
        }
#endif
      }
      else
      {
        std::clog << "No fonts found" << std::endl;
      }
    }
  }

//  std::clog << "@leaving Page::load()" << std::endl;
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
    ss << "Font " << it->first << ":" << std::endl;
    ss << it->second->dump();
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
	public:
		CTM tm;
		CTM lm;
		std::wstring accumulated_text; // work around multiple Tj in one text object
		double total_width;

		TextObject(const Page::GraphicsState * g, Media * m):gs(g),media(m),total_width(0) {}
		void Append(const String * str)
		{
			double w;
			String tmp;
			if(!gs->text_state.Tf->is_multibyte() && gs->text_state.Tw != 0) { /* output words separately appending Tw space between */
				tmp = *str;
				String * sp;
				while((sp = tmp.cut_word())) {
					std::wstring s=gs->text_state.Tf->extract_text(sp, &w);
					//w+=s.length() * gs->text_state.Tc; // XXX TODO Implement Tc
					accumulated_text+=s;
					total_width+=w;
					Flush();
					tm.offset_unscaled(gs->text_state.Tw, 0);
					delete sp;
				}
				str = &tmp;
			}
			std::wstring s=gs->text_state.Tf->extract_text(str, &w);
			accumulated_text+=s;
			total_width+=w;
		 // XXX update text matrix!!!
		}
		void Kerning(double k) {}
		void NewLine() {
			accumulated_text+='\n';
			lm.offset_unscaled(0, gs->text_state.Tl);
			tm = lm;
		}
		void Flush() {
			if(accumulated_text.empty()) return;
			double x_offs=0;
#if 0
			while(accumulated_text[0] == (wchar_t)' ') { /* append Tw*number_of_initial_spaces */
				x_offs+=gs->text_state.Tw;
				if(accumulated_text.length()<=1) break;
				accumulated_text.erase(0,1);
			}
#endif
			media->Text(gs->ctm.translate(tm.translate(Point(x_offs,0))), gs->text_state.Tf, accumulated_text);
			tm.offset(total_width, 0);
			accumulated_text.resize(0);
			total_width = 0;
		}
};

std::wstring extract_text(const String * so, const Font * f);
void Page::draw(Media * m)
{
  Path * curpath=NULL;
  TextObject * tobj=NULL;

  enum { M_PAGE, M_PATH, M_TEXT, M_CLIP, M_IMAGE } mode=M_PAGE;
  
	m->Size(media_box.size()); /* XXX use returned value to set up ctm or, better, pass ref to ctm into Media::Size() */
  for(std::vector<Operator *>::iterator it=operators.begin(); it!=operators.end(); it++)
  {
    Operator * op=*it;
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
          else { gs=new GraphicsState(); std::cerr << "GraphicsState stack underrun" << std::endl; }
          break;
        }
        else { std::clog << "Ignoring operator " << op->dump() << " in page mode" << std::endl; }
        /* NO BREAK!!! */
      case M_CLIP:
      case M_PATH: /*=============== path construction =====================*/
        if(op->name() == "m") // moveto
        {
          if(curpath)
          {
            /// \todo Implement full path with subpaths
            std::cerr << "Deleting live path!!!" << std::endl;
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
        else if(op->name() == "W" || op->name() == "W*")
        {
          mode=M_CLIP;
        }
        else if(op->name() == "s" || op->name() == "S"
            || op->name() == "f" || op->name() == "F" || op->name() == "f*"
            || op->name() == "B" || op->name() == "B*"
            || op->name() == "b" || op->name() == "b*")
        {
					if(curpath) {
						/* paint path */
#if 0
						std::clog << "Drawing path: ";
						for(unsigned int i=0; i<curpath->size()-1; i++)
							std::clog << "p" << curpath->at(i).dump();
						std::clog << std::endl;
#endif
						for(unsigned int i=0; i<curpath->size()-1; i++)
						{
							m->Line(gs->ctm.translate(curpath->at(i)), gs->ctm.translate(curpath->at(i+1)));
						}
						if(op->name() == "s" || op->name() == "f")
							m->Line(gs->ctm.translate(curpath->at(curpath->size()-1)), gs->ctm.translate(curpath->at(0)));
						delete curpath; curpath=NULL;
					} else {
						std::clog << "Attempted to draw (" << op->dump() << ") non-existent path" << std::endl;
					}
          mode=M_PAGE;
        }
        else { std::clog << "Ignoring operator " << op->dump() << " in path construction mode" << std::endl; }
        break;
      case M_TEXT:
        if(!tobj) tobj=new TextObject(gs, m);
        /* Check text showing operators first */
        if(op->name() == "Tj") // output text
        {
          if(!gs->text_state.Tf) throw std::string("No font set");
          const Object * o=op->arg(0);
          const String * str=dynamic_cast<const String *>(o);
          if(str)
          {
            /*m->Text(gs->ctm.translate(tobj->tm.translate(Point(0,0))), s);
            // XXX update text matrix!!! */
						tobj->Append(str);
          }
          break;
        }
        else if(op->name() == "TJ")
        {
          if(!gs->text_state.Tf) throw std::string("No font set");
    //      std::clog << "Found TJ!" << std::endl;
          const Object * o=op->arg(0);
          const Array * a=dynamic_cast<const Array *>(o);
          if(a)
          {
            //std::wstring s;
            for(Array::ConstIterator it=a->get_const_iterator(); a->check_iterator(it); it++)
            {
    //        std::clog << "Arg" << i << ": " << o->type() << std::endl;
              const String * str=dynamic_cast<const String *>(*it);
              if(str) tobj->Append(str);
							else {
								const Integer * i = dynamic_cast<const Integer *>(*it);
								const Real * r = dynamic_cast<const Real *>(*it);
								if(i) tobj->Kerning(i->value());
								else if(r) tobj->Kerning(r->value());
								else {
									std::clog << "Unexpected object " << (*it)->type() << " in string (TJ)" << std::endl;
								}
							}
            }
            /*m->Text(gs->ctm.translate(tobj->tm.translate(Point(0,0))), s);
            // XXX update text matrix!!!*/
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
        else if(op->name() == "Tr") {} // also, render mode
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
        }
        else if(op->name() == "Tm")
        {
          // XXX set whole matrix!
          tobj->lm.set_unity();
          tobj->lm.scale(op->number(0), op->number(3));
          tobj->lm.offset(op->point(4));
          tobj->tm=tobj->lm;
        }
        else if(op->name() == "Td") // XXX SEE ON PAGE 368 XXX Scaled or unscaled??
        {
          tobj->lm.offset_unscaled(op->point(0));
          tobj->tm=tobj->lm;
        }
				else if(op->name() == "TL") /* TL and TD added 2007-05-18 and untested! */
				{
					gs->text_state.Tl=op->number(0);
				}
				else if(op->name() == "TD") // tx ty TD === -ty TL; tx ty TD
				{
					gs->text_state.Tl=-op->number(1);
          tobj->lm.offset_unscaled(op->point(0));
          tobj->tm=tobj->lm;
				}
        else { std::clog << "Ignoring operator " << op->dump() << " in text mode" << std::endl; }
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
}

std::wstring extract_text(const String * so, const Font * f)
{
#if 0
  std::string s=so->value();
  std::wstring ws;
  if(s.length() % 2) throw std::string("String lenth is not even number");
  for(unsigned int i=0; i<s.length(); i+=2)
  {
    unsigned long c=((unsigned char)s[i])<<8 | (unsigned char)s[i+1];
    wchar_t wc=f->to_unicode(c);
    ws.push_back(wc);
  }
  return ws;
#else
	return f->extract_text(so);
#endif
}

}; // namespace PDF

