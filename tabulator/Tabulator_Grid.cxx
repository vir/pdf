#include "Tabulator.hpp"

std::string Tabulator::Grid::Line::dump() const
{
	std::stringstream ss;
	for(unsigned int i = 0; i < size(); i++) {
		ss << (at(i)?'=':'.');
	}
	return ss.str();
}


/** Find table column number by x coordinate */
unsigned int Tabulator::Grid::find_col(double x) const
{
	KnotsIterator it;
	unsigned int r = 0;
	for(it = h_knots.begin(); it != h_knots.end(); it++, r++) {
		if(x < it->first) {
			if(r > 0) // in table
				return r - 1;
			else
				break;
		}
	}
	throw std::string("Point not in table");
}

/** Find table row number by y coordinate */
unsigned int Tabulator::Grid::find_row(double y) const
{
	KnotsIterator it = v_knots.begin();
	unsigned int r = 0;
	for(it++; it != v_knots.end(); it++, r++) {
		if(y < it->first) {
			if(r > 0) // in table
				return r - 1;
			else
				break;
		}
	}
	throw std::string("Point not in table");
}

/** Return object dump for debugging purposes */
std::string Tabulator::Grid::dump() const
{
	std::stringstream ss;
	KnotsIterator it;
	ss << "Grid: " << h_knots.size() << " h-knots, " << v_knots.size() << " v-knots" << std::endl;
	ss << " H-knots is at";
	for(it = h_knots.begin(); it != h_knots.end(); it++)
		ss << ' ' << (double)it->first << "(" << it->second.dump() << ")";
	ss << std::endl;
	ss << " V-knots is at";
	for(it = v_knots.begin(); it != v_knots.end(); it++)
		ss << ' ' << (double)it->first << "(" << it->second.dump() << ")";
	ss << std::endl;
	return ss.str();
}



