#ifndef WXTABTABULATOR_HPP_INCLUDED
#define WXTABTABULATOR_HPP_INCLUDED

#include "Tabulator.hpp"
#include <wx/wx.h>

class wxDC;
class WxTabTabulator:public Tabulator
{
	public:
		void Draw(wxDC * dc);
		void ShowOptionsDialog(wxWindow * parent);
		bool ok() const { return (table.ncols() && table.nrows()); }
};

extern WxTabTabulator * theTabulator;

#endif /* WXTABTABULATOR_HPP_INCLUDED */

