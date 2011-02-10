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
int Tabulator::Grid::find_col(double x) const
{
	KnotsIterator it;
	int r = 0;
	for(it = h_knots.begin(); it != h_knots.end(); it++, r++) {
		if(x < it->first) {
			return r - 1; // -1 if not in table
		}
	}
	return -1;
}

/** Find table row number by y coordinate */
int Tabulator::Grid::find_row(double y) const
{
	KnotsIterator it = v_knots.begin();
	unsigned int r = 0;
	for(it++; it != v_knots.end(); it++, r++) {
		if(y < it->first) {
			return r - 1; // -1 if not in table
		}
	}
	return -1;
}

void Tabulator::Grid::split_column(unsigned int col, double x)
{
	KnotsMap::iterator iit = h_knots.insert(KnotsMap::value_type(x, Line(v_knots.size()-1, true))).first;
	for(KnotsMap::iterator kit = v_knots.begin(); kit != v_knots.end(); ++kit) {
		if(!kit->second.empty()) {
			Line::iterator pos = kit->second.begin() + (col - 1);
			kit->second.insert(pos, *pos);
		}
	}
}

/** automatically find place for new vertical line
 *
 * This algorithm works only if "second" column is left-aligned
 * \arg col index of column to split (0 - leftmost column)
 * \return true if line was successfully added
 */
bool Tabulator::Grid::auto_split_column(unsigned int col, const Tabulator::Metafile * mf)
{
	KnotsIterator kit;
	double x1, x2;
	kit = h_knots.begin();
	for(unsigned int i = 0; i < col - 1; i++)
		kit++;
	x1 = kit->first;
	kit++; /* pointer to second verical line */
	x2 = kit->first;
	if(debug)
		std::clog << "Splitting " << col << "th column (between " << x1 << " and " << x2 << ")" << std::endl;

	// Collect some stats
	std::map<Coord, unsigned int> xpos_hits;
	unsigned int text_lines_count = 0;
	Coord last_y = 1E-10;
	Tabulator::Metafile::TextMap::const_iterator tit; // text iterator
	for(tit = mf->all_text.begin(); tit != mf->all_text.end() && tit->pos.y < margins.bottom; tit++) { /* check all text */
		if(tit->pos.y < headers_end) // skip table header
			continue;
		if(tit->pos.x >= x1 && tit->pos.x < x2) {
			if(tit->pos.y != last_y) {
				++text_lines_count;
				last_y = tit->pos.y;
			}
			std::map<Coord, unsigned int>::iterator hit = xpos_hits.insert(std::make_pair(tit->pos.x, (unsigned int)0)).first;
			++hit->second;
		}
	}

	if(debug) {
		std::clog << "xpos_hits:" << std::endl;
		for(std::map<Coord, unsigned int>::const_iterator hit = xpos_hits.begin(); hit != xpos_hits.end(); ++hit)
			std::clog << std::setw(10) << hit->first << " : " << hit->second << std::endl;
	}

	unsigned int edge = text_lines_count / 4;

	// Remove initial burst
	double look_till = x1 + (x2 - x1)/3;
	bool in_initial_burst = false;
	for(std::map<Coord, unsigned int>::const_iterator hit = xpos_hits.begin(); hit != xpos_hits.end()&& double(hit->first) < look_till; ++hit) {
		if(in_initial_burst) {
			if(hit->second < edge) {
				if(debug)
					std::clog << "Removing xpos_hits up to " << hit->first << " (edge: " << edge << ")" << std::endl;
				xpos_hits.erase(xpos_hits.begin(), hit);
				break;
			}
		} else {
			if(hit->second > edge)
				in_initial_burst = true;
		}
	}

	// Find maximum hits pos
	double x_max = -1E10; unsigned int hits_max = 0;
	for(std::map<Coord, unsigned int>::const_iterator hit = xpos_hits.begin(); hit != xpos_hits.end(); ++hit) {
		if(hit->second > hits_max) {
			hits_max = hit->second;
			x_max = hit->first;
		}
	}

	// Finally, add line at found pos
	if(x_max > 0) {
		const double adjust = -2.5;
		if(debug)
			std::clog << "Adding vertical line at " << x_max - adjust << " (maximum at " << x_max << ")" << std::endl;
		split_column(col, x_max + adjust);
		return true;
	}
	if(debug)
		std::clog << "Can not split column " << col << " :-(" << std::endl;
	return false;
}

class IntervalCounter
{
	class Interval:public std::pair<double, double>
	{
	public:
		bool operator < (const Interval& other) { return first < other.first; }
		Interval(double x1, double x2):pair(x1, x2) { }
	};
};

#if 0
bool Tabulator::Grid::auto_split_column_new(unsigned int col, const Tabulator::Metafile * mf)
{
	KnotsIterator kit;
	double x1, x2;
	kit = h_knots.begin();
	for(unsigned int i = 0; i < col - 1; i++)
		kit++;
	x1 = kit->first;
	kit++; /* pointer to second verical line */
	x2 = kit->first;
	if(debug)
		std::clog << "Splitting " << col << "th column (between " << x1 << " and " << x2 << ")" << std::endl;

	// Collect some stats
	std::map<Coord, unsigned int> xpos_hits;
	unsigned int text_lines_count = 0;
	Coord last_y = 1E-10;
	Tabulator::Metafile::TextMap::const_iterator tit; // text iterator
	for(tit = mf->all_text.begin(); tit != mf->all_text.end() && tit->pos.y < margins.bottom; tit++) { /* check all text */
		if(tit->pos.y < headers_end) // skip table header
			continue;
		if(tit->pos.x >= x1 && tit->pos.x < x2) {
			if(tit->pos.y != last_y) {
				++text_lines_count;
				last_y = tit->pos.y;
			}
			std::map<Coord, unsigned int>::iterator hit = xpos_hits.insert(std::make_pair(tit->pos.x, (unsigned int)0)).first;
			++hit->second;
		}
	}
}
#endif

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



