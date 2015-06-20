#include "Page_TextObject.h"
#include "Font.hpp"
#include "Media.hpp"
#include <assert.h>

#define DRAW_CHAR_BY_CHAR 0

void PDF::Page::TextObject::Append( const String * str )
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

void PDF::Page::TextObject::Kerning( double k )
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

void PDF::Page::TextObject::Flush()
{
	if(accumulated_text.empty()) return;
	CTM m(gs->text_state.Tfs * gs->text_state.Th/100.0, 0, 0, gs->text_state.Tfs, 0, gs->text_state.Trise);
	CTM Trm = m * tm * gs->ctm; // Construct text rendering matrix
	if(update_font) {
		media->SetFont(gs->text_state.Tf, Trm.get_scale_v());
		update_font = false;
	}

	// build translated clipping path
	Path result_clip;
	Point tmp_point;
	for(Path::const_iterator it = gs->clipping_path.begin(); it != gs->clipping_path.end(); ++it)
	{
		tmp_point = gs->ctm.translate(*it);
		result_clip.push_back(tmp_point);
	}

	Rect result_rect(Trm.translate(Point(0,0)), total_width*Trm.get_scale_h(), Trm.get_scale_v());
	bool is_visible = result_clip.clip(result_rect);
	//assert(is_visible);

	media->Text(
		result_rect.offset(),
		Trm.get_rotation_angle(),
		accumulated_text,
		result_rect.size().x,
		result_rect.size().y
	);
#if 0 // draw clipping path
	for(Path::const_iterator it = result_clip.begin(); it != result_clip.end(); ++it)
	{
		media->Line(tmp_point, *it);
		tmp_point = *it;
	}
#endif
	double offset = total_width * gs->text_state.Tfs * gs->text_state.Th/100.0;
	tm.offset_unscaled(offset, 0);
	accumulated_text.resize(0);
	total_width = 0;
}
