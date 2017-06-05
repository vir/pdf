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

#include "Content_TextObject.hpp"
#include "Exceptions.hpp"
#include "Media.hpp"
#include "Object.hpp"

namespace PDF {

Content::Render::Render(GraphicsStateStack& gs, Page::ResourceProvider& res, Media& m)
	: gs(gs), res(res), m(m)
	, mode(M_PAGE)
	, curpath(NULL)
	, tobj(NULL)
{
}

Content::Render::~Render()
{
	if(tobj) delete tobj;
	if(curpath) delete curpath;
}

bool Content::Render::draw(const Page::Operator& op)
{
	/* Check for any-mode-operators */
	if(op.name() == "cm") {
		if(tobj)
			tobj->Flush();
		gs->ctm *= op.matrix();
		if(tobj)
			tobj->FontChanged();
		return true;
	}
	switch(mode)
	{
	case M_PAGE: return draw_page_mode(op);
	case M_PATH: return draw_path_mode(op);
	case M_TEXT: return draw_text_mode(op);
	case M_IMAGE: return draw_image_mode(op);
	default: throw UnimplementedException("Default reached in Page::Render::draw()");
	}
}

void Content::Render::dump(std::ostream & s)
{
	if(tobj) {
		s << "Line matrix:" << std::endl << tobj->lm.dump();
		s << "Text matrix:" << std::endl << tobj->tm.dump();
	}
	if(curpath)
		s << "Current path: " << curpath->dump() << std::endl;
}

const char * Content::Render::mode_string() const
{
	switch(mode)
	{
	case M_PAGE: return "M_PAGE";
	case M_PATH: return "M_PATH";
	case M_TEXT: return "M_TEXT";
	case M_IMAGE: return "M_IMAGE";
	default: return "Some unknown mode";
	}
}

bool Content::Render::draw_page_mode(const Content::Operator& op)
{
	if(op.name() == "m" || op.name() == "re") { mode = M_PATH; return draw_path_mode(op); }
	if(op.name() == "BT") { mode = M_TEXT; return true; }
	if(op.name() == "BI") { mode = M_IMAGE; return true; }
	if(op.name() == "Do")
	{
		const Name * n = dynamic_cast<const Name *>(op.arg(0));
		if(!n)
			throw WrongPageException("XObject id is not a name");
		XObject* x = res.get_xobject(n->value());
		if(!x)
			return false;
			//throw UnimplementedException("XXX may be this xobject was not loaded");
		// 1. Saves the current graphics state, as if by invoking the q operator
		gs.push();
		// 2. Concatenates the matrix from the form dictionary’s Matrix entry with the current transformation matrix (CTM)
		x->update_ctm(gs->ctm);
		// 3. Clips according to the form dictionary’s BBox entry
		// XXX
		// 4. Paints the graphics objects specified in the form’s content stream
		x->draw(gs, res, m, *this);
		// 5. Restores the saved graphics state, as if by invoking the Q operator
		gs.pop();
		return true;
	}
	if(op.name() == "sh") { /* ignore it */ return true; }
	if(op.name() == "q") { gs.push(); return true; }
	if(op.name() == "Q") { gs.pop(); return true; }
	if(op.name() == "Tf") // set current font (HACK!!! It should be done in text mode!!!)
	{
		std::clog << "Bad PDF: Tf outside text object!" << std::endl;
		const Name * n = dynamic_cast<const Name *>(op.arg(0));
		if(n)
			gs->text_state.Tf = res.get_font(n->value());
		gs->text_state.Tfs = op.number(1);
		//gs->text_state.Tfs=abs(op.number(1)); // XXX WTF? Negative font sizes? Really?!?
		return true;
	}
	return false;
}

bool Content::Render::draw_path_mode(const Content::Operator& op)
{
	if(op.name() == "m") // moveto
	{
		if(curpath)
		{
			/// \todo Implement full path with subpaths
			std::cerr << "Deleting live path!!!" << curpath->dump() << std::endl;
			delete curpath;
		}
		curpath = new Path;
		curpath->push_back(op.point(0));
		return true;
	}
	if(op.name() == "l") // lineto
	{
		if(curpath) curpath->push_back(op.point(0));
		else { std::cerr << "Line without path??" << std::endl; }
		return true;
	}
	if(op.name() == "re") // rectangle
	{
		if(!curpath) curpath = new Path;
		Point b = op.point(0);
		curpath->push_back(b);
		curpath->push_back(Point(b.x + op.number(2), b.y));
		curpath->push_back(Point(b.x + op.number(2), b.y + op.number(3)));
		curpath->push_back(Point(b.x, b.y + op.number(3)));
		curpath->push_back(b);
		return true;
	}
	if(op.name() == "W") // modify clipping path
	{
		gs->modify_clipping_path(curpath);
		return true;
	}
	if(op.name() == "h") // home
	{
		if(curpath && curpath->size()) curpath->push_back(curpath->at(0));
		return true;
	}
	if(op.name() == "c" || op.name() == "v" || op.name() == "y") // curves
	{
		std::cerr << "Beizer curves not implemented" << std::endl;
		return false;
	}
	if(op.name() == "n") // no-op
	{
		delete curpath; curpath = NULL;
		mode = M_PAGE;
		return true;
	}
	if(op.name() == "s" || op.name() == "S" // stroke path
		|| op.name() == "f" || op.name() == "F" || op.name() == "f*" // fill path 
		|| op.name() == "B" || op.name() == "B*"
		|| op.name() == "b" || op.name() == "b*")
	{
		if(curpath) {
			if(op.name() == "s" || op.name() == "b" || op.name() == "b*")
				curpath->push_back(curpath->at(0));
			Media::PathFillMode fm = Media::PATH_NO_FILL;
			bool stroke = false;
			// fill
			if(op.name() == "f" || op.name() == "F" || op.name() == "b" || op.name() == "B")
				fm = Media::PATH_FILL_NZWNR;
			else if(op.name() == "f*" || op.name() == "b*" || op.name() == "B*")
				fm = Media::PATH_FILL_EOR;
			//stroke
			if(op.name() == "s" || op.name() == "S" || op.name() == "b" || op.name() == "b*" || op.name() == "B" || op.name() == "B*")
				stroke = true;
			m.DrawPath(*curpath, fm, stroke, gs);

			/* paint path */
			for(unsigned int i = 0; i<curpath->size() - 1; i++)
			{
				m.Line(gs->ctm.translate(curpath->at(i)), gs->ctm.translate(curpath->at(i + 1)), gs);
			}
			delete curpath; curpath = NULL;
		}
		else {
			std::cerr << "Attempted to draw (" << op.dump() << ") non-existent path" << std::endl;
		}
		mode = M_PAGE;
		return true;
	}
	if(op.name() == "w")
	{
		gs->line_width = op.number(0);
		return true;
	}
	return false;
}

bool Content::Render::draw_text_mode(const Content::Operator& op)
{
	if(!tobj) tobj = new TextObject(gs, &m);
	/* Check text showing operators first */
	if(op.name() == "Tj") // output text
	{
		if(!gs->text_state.Tf)
			throw WrongPageException("No font set");
		const String * str = dynamic_cast<const String *>(op.arg(0));
		if(str)
			tobj->Append(str);
		return true;
	}
	if(op.name() == "TJ")
	{
		const Array * a = dynamic_cast<const Array *>(op.arg(0));
		if(!a)
			throw WrongPageException("TJ with no array");
		for(Array::ConstIterator it = a->get_const_iterator(); a->check_iterator(it); it++) {
			const String * str = dynamic_cast<const String *>(*it);
			if(str) tobj->Append(str);
			else {
				const Integer * i = dynamic_cast<const Integer *>(*it);
				const Real * r = dynamic_cast<const Real *>(*it);
				if(i) tobj->Kerning(i->value() / 1000.0);
				else if(r) tobj->Kerning(r->value() / 1000.0);
				else
					std::cerr << "Unexpected object " << (*it)->type() << " in string (TJ)" << std::endl;
			}
		}
		return true;
	} // TJ
	if(op.name() == "\'" || op.name() == "\"")
	{
		const String * str = dynamic_cast<const String *>(op.arg((op.name() == "\"") ? 2 : 0));
		if(op.name() == "\"") {
			gs->text_state.Tw = op.number(0);
			gs->text_state.Tc = op.number(1);
		}
		if(str)
		{
			tobj->NewLine();
			tobj->Append(str);
		}
		return true;
	}
	if(op.name() == "T*") // XXX it is not a text-showing operator!
	{
		tobj->NewLine();
		return true;
	}

	tobj->Flush();
	// ... and executete other operators

	/* check other text object operators */
	if(op.name() == "ET") { if(tobj) delete tobj; tobj = NULL; mode = M_PAGE; return true; }
	if(op.name() == "Tc") { gs->text_state.Tc = op.number(0); return true; } // char spacing
	if(op.name() == "Ts") { gs->text_state.Trise = op.number(0); return true; } // rise
	if(op.name() == "Tz") { gs->text_state.Th = op.number(0); return true; } // hor. scaling
	if(op.name() == "Tw") { gs->text_state.Tw = op.number(0); return true; } // word spacing - useful if text begins with space(s)
	if(op.name() == "Tf") // set current font
	{
		tobj->Flush();
		const Name * n = dynamic_cast<const Name *>(op.arg(0));
		if(n)
			gs->text_state.Tf = res.get_font(n->value());
		gs->text_state.Tfs = op.number(1);
		tobj->FontChanged();
		return true;
	}
	else if(op.name() == "Tm") { tobj->SetMatrix(op.matrix()); return true; }
	else if(op.name() == "Td") { tobj->Offset(op.point(0)); return true; }
	else if(op.name() == "TL") { gs->text_state.Tl = op.number(0); return true; }
	else if(op.name() == "TD") // tx ty TD === -ty TL; tx ty Td
	{
		tobj->Offset(op.point(0));
		gs->text_state.Tl = -op.number(1);
		return true;
	}
	return false;
}

bool Content::Render::draw_image_mode(const Content::Operator& op)
{
	if(op.name() == "EI") { mode = M_PAGE; return true; }
	return false;
}

} /* namespace PDF */

