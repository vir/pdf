#ifndef WXTABTABULATOR_HPP_INCLUDED
#define WXTABTABULATOR_HPP_INCLUDED

#include "Tabulator.hpp"

class wxDC;
class WxTabTabulator:public Tabulator
{
	public:
		void Draw(wxDC * dc);
};

extern WxTabTabulator * theTabulator;

#endif /* WXTABTABULATOR_HPP_INCLUDED */

