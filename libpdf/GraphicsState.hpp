#ifndef PAGE_GRAPHICSSTATE_HPP_INCLUDED
#define PAGE_GRAPHICSSTATE_HPP_INCLUDED

#include <stack>
#include "Path.hpp"
#include "Ctm.hpp"
#include "Exceptions.hpp"

namespace PDF {

class Font;

class GraphicsState {
public:
	GraphicsState():line_width(0) {}
	/*=== device-independent: ===*/
	CTM ctm;
	Path clipping_path;
	// color_space
	// color
	struct TextState {
		double Tc; // char spacing (Tc)=0
		double Tw; // word spacing (Tw)=0
		double Th; // horizontal scaling (Tz)=100
		double Tl; // leading (TL)=0
		Font * Tf; // text font (Tf)
		double Tfs; // font size (Tf)
		// Tmode // rendering mode (Tr)=0
		double Trise; // text rise (Ts)=0
		// Tk // text knockout
		TextState():Tc(0),Tw(0),Th(100),Tl(0),Tf(NULL),Tfs(1),Trise(0) {}
		void dump(std::ostream & s) const;
	} text_state;
	double line_width; // set with 'w'
	// line_cap // set with 'J'
	// line_join // set with 'j'
	// miter_limit // set with 'M'
	// dash_pattern set with 'd'
	// rendering_intent // set with 'ri'
	// stroke_adjustment
	// blend_mode
	// soft_mask
	// alpha_constant
	// alpha_source
	/*=== device-dependent: ===*/
	// overprint
	// overprint_mode
	// black_generation
	// undercolor_removal
	// transfer
	// halftone
	// flatness // set with 'i'
	// smothness
	void modify_clipping_path(const Path* p);
	Path translate_path(const Path& p) const;
	void dump(std::ostream & s) const
	{
		s << "CTM:" << std::endl << ctm.dump();
		s << "Clipping path: " << clipping_path.dump() << std::endl;
		s << "Text state: ";
		text_state.dump(s);
	}
};

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
	void clear();
	void inherit(const GraphicsStateStack& parent);
private:
	GraphicsStateStack(const GraphicsStateStack&);
	GraphicsStateStack& operator=(const GraphicsStateStack&);
private:
	GraphicsState * gs;
	std::stack<GraphicsState *> gstack;
};


}; // namespace PDF

#endif /* PAGE_GRAPHICSSTATE_HPP_INCLUDED */
