#include "Content_TextObject.hpp"
#include "Font.hpp"
#include "Media.hpp"
#include <assert.h>

#define DRAW_CHAR_BY_CHAR 0

void PDF::Content::TextObject::Append( const PDF::String * str )
{
	unsigned int pos = 0;
	wchar_t nextchar;
	double charwidth = 0;
	while(gs->text_state.Tf->extract_one_char(str, nextchar, charwidth, pos)) {
#if DRAW_CHAR_BY_CHAR
		if(gs->text_state.Tc)
			Kerning(-(gs->text_state.Tc / gs->text_state.Tfs));
		accumulated_text += nextchar;
		total_width += charwidth;
		if(nextchar == L' ')
			total_width += gs->text_state.Tw / gs->text_state.Tfs;
		Flush();
#else
		if(nextchar == L' ') {
			Flush();
			double offset = (charwidth * gs->text_state.Tfs) * gs->text_state.Th/100.0;
			offset += (gs->text_state.Tw / gs->text_state.Tfs) * gs->text_state.Tfs * gs->text_state.Th/100.0;
			offset += gs->text_state.Tc / gs->text_state.Tfs * gs->text_state.Th/100.0;
			tm.offset_unscaled(offset, 0);
			continue;
		} else {
			accumulated_text += nextchar;
			total_width += charwidth;
			if(gs->text_state.Tc)
				Kerning(-(gs->text_state.Tc / gs->text_state.Tfs));
		}
#endif
	}
}

void PDF::Content::TextObject::Kerning( double k )
{
	// offset next char back by k glyph space units
	if(k > kerning_too_big || k < -kerning_too_big || accumulated_text.empty()) { // XXX TODO caompare with avg charwidth or so
		Flush();
		double offset = -k * gs->text_state.Tfs * gs->text_state.Th/100.0;
		tm.offset_unscaled(offset, 0);
	} else {
		total_width-=k;
	}
}

void PDF::Content::TextObject::Flush()
{
	if(accumulated_text.empty()) return;
	CTM m(abs(gs->text_state.Tfs) * gs->text_state.Th/100.0, 0, 0, abs(gs->text_state.Tfs), 0, gs->text_state.Trise);
	CTM Trm = m * tm * gs->ctm; // Construct text rendering matrix
	if(update_font) {
		media->SetFont(gs->text_state.Tf, Trm.get_scale_v());
		update_font = false;
	}

	Rect result_rect(Trm.translate(Point(0,0)), total_width*Trm.get_scale_h(), Trm.get_scale_v());
	// Calculate text matrix displacemant before applying clipping path
	double offset = total_width * abs(gs->text_state.Tfs) * gs->text_state.Th/100.0;
	// Apply clipping path to determint actual text size
	bool is_visible = gs->clipping_path.clip(result_rect);
	//assert(is_visible);

	media->Text(
		result_rect,
		Trm.get_rotation_angle(),
		accumulated_text,
		is_visible,
		*gs
	);
#if 0 // draw clipping path
	if(result_clip.size())
	{
		std::clog << "TEXT at";
		std::clog << result_rect.dump();
		std::clog << "\tCLIP ";
//		std::clog << result_clip.dump();
		std::clog << gs->clipping_path.dump();
		std::clog << "\t: ";
		std::wclog << accumulated_text;
		std::wclog << std::endl;
# if 0
		std::clog << gs->ctm.dump();
		std::clog << Trm.dump();
		std::clog << m.dump();
# endif
# if 0
		Path::const_iterator pit = result_clip.begin();
		Point tmp_point = *pit;
		++pit;
		for(; pit != result_clip.end(); ++pit)
		{
			media->Line(tmp_point, *pit, *gs);
			tmp_point = *pit;
		}
# endif
	}
#endif
	tm.offset_unscaled(offset, 0);
	accumulated_text.resize(0);
	total_width = 0;
}
