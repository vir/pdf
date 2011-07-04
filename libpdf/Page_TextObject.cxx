#include "Page_TextObject.h"
#include "Font.hpp"
#include "Media.hpp"

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
		Flush();
#else
		if(nextchar == L' ') {
			spaces++;
			space_width = charwidth;
			if(spaces > 0) {
				Flush();
				double offset = (charwidth * gs->text_state.Tfs) * gs->text_state.Th/100.0;
				offset += (gs->text_state.Tw / gs->text_state.Tfs) * gs->text_state.Tfs * gs->text_state.Th/100.0;
				tm.offset_unscaled(offset, 0);
				//if(false && spaces == 2) // XXX compensate first space displacement assuming that first space width is equal to second
				//	tm.offset_unscaled(offset, 0);
			}
			continue;
		} else {
			if(spaces) {
				if(false && spaces == 1 && !accumulated_text.empty()) {
					accumulated_text += L' ';
					total_width += space_width;
					Kerning(-(gs->text_state.Tw / gs->text_state.Tfs));
					Kerning(-(gs->text_state.Tc / gs->text_state.Tfs));
				}
				spaces = 0;
			}
			if(gs->text_state.Tc)
				Kerning(-(gs->text_state.Tc / gs->text_state.Tfs));
			accumulated_text += nextchar;
			total_width += charwidth;
		}
#endif
	}
}

void PDF::Page::TextObject::Kerning( double k )
{
	// offset next char back by k glyph space units
	if(k > kerning_too_big || k < -kerning_too_big) { // XXX TODO caompare with avg charwidth or so
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
#if TC_IS_KERNING
	double offset = (total_width*gs->text_state.Tfs) * gs->text_state.Th/100.0;
#else
	double offset = (total_width*gs->text_state.Tfs + gs->text_state.Tc*accumulated_text.length()) * gs->text_state.Th/100.0;
#endif


#if 0 // deal with "text1          text2              text3"
	double pos_x = 0;
	while(!accumulated_text.empty()) {
		int spaces_begin = accumulated_text.find(L"  ");
		std::wstring part = accumulated_text.substr(0, spaces_begin);
		media->Text(
			Trm_tmp.translate(Point(pos_x, 0)),
			Trm_tmp.get_rotation_angle(),
			part,
			w * Trm_tmp.get_scale_h(),
			Trm.get_scale_v()
			);
		if(spaces_begin != std::wstring::npos) {
			if(spaces) {

				media->Text(
					Trm_tmp.translate(Point(0,0)),
					Trm_tmp.get_rotation_angle(),
					accumulated_text.substr(0, spaces),
					w * Trm_tmp.get_scale_h(),
					Trm.get_scale_v()
					);
				Trm_tmp.offset_unscaled((total_width*gs->text_state.Tfs) * gs->text_state.Th/100.0, 0);
			}
		}
		int end_of_spaces = accumulated_text.find_first_not_of(L" \t", spaces);
		accumulated_text.erase(0, end_of_spaces);
	}
	if(accumulated_text.empty()) return;
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
