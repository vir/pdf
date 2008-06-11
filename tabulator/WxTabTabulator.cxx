#include "WxTabTabulator.hpp"
#include <wx/dc.h>
#include <wx/gdicmn.h>

WxTabTabulator * theTabulator = NULL;




void WxTabTabulator::Draw(wxDC * dc)
{
	wxSize cs = dc->GetSize();

	// Draw grid
	dc->SetPen(wxPen(*wxRED, 0, wxSOLID));
	Grid::KnotsIterator kit;
	for(kit = grid.h_knots.begin(); kit != grid.h_knots.end(); kit++) {
		dc->DrawLine(kit->first, 0, kit->first, cs.GetHeight());
	}
	for(kit = grid.v_knots.begin(); kit != grid.v_knots.end(); kit++) {
		dc->DrawLine(0, kit->first, cs.GetWidth(), kit->first);
	}
	// Draw cells
}




