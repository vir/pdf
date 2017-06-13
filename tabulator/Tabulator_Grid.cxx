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

bool Tabulator::Grid::get_rect(int row, int col, PDF::Rect & rect) const
{
	// TODO: Cache map iterators in vector, indexed by row/col number
	// or maybe replace map with vector
	KnotsIterator it;
	it = v_knots.begin();
	while(row--)
		if(it != v_knots.end())
			++it;
	if(it == v_knots.end())
		return false;
	rect.y1 = it->first;
	++it;
	if(it == v_knots.end())
		return false;
	rect.y2 = it->first;

	it = h_knots.begin();
	while(col--)
		if(it != h_knots.end())
			++it;
	if(it == h_knots.end())
		return false;
	rect.x1 = it->first;
	++it;
	if(it == h_knots.end())
		return false;
	rect.x2 = it->first;

	return true;
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

/** Intervals usage counter */
class IntervalHystogram
{
private:
	std::map<double, unsigned int> m_space;
	double m_right_margin;
public:
	IntervalHystogram(double left, double right)
		: m_right_margin(right)
	{
		m_space.insert(std::make_pair(left, 0U));
	}
	void use(double left, double right)
	{
		// trim input range
		if(left < m_space.begin()->first)
			left = m_space.begin()->first;
		if(right > m_right_margin)
			right = m_right_margin;

		std::map<double, unsigned int>::iterator left_it = m_space.upper_bound(left);
		--left_it;
		if(left_it->first != left)
			left_it = m_space.insert(left_it, std::make_pair(left, left_it->second));

		std::map<double, unsigned int>::iterator right_it = m_space.upper_bound(right);
		if(right != m_right_margin) {
			--right_it;
			right_it = m_space.insert(right_it, std::make_pair(right, right_it->second));
		}
		for(; left_it != right_it; ++left_it)
			++left_it->second;
	}
	/** Removes first and last intervals if they are unused */
	void trim()
	{
		if(m_space.empty())
			return;
		if(! m_space.begin()->second)
			m_space.erase(m_space.begin());
		std::map<double, unsigned int>::iterator it = m_space.end();
		--it;
		if(! it->second)
			m_space.erase(it);
	}
	/** Finds next unused interval (on right hand respective to the current pos) */
	bool find_next_unused_space(double & pos)
	{
		std::map<double, unsigned int>::iterator it = m_space.upper_bound(pos);
		for(;;) {
			if(it == m_space.end())
				return false;
			if(it->second == 0)
				break;
			++it;
		}
		double left = it->first;
		++it;
		double right = it == m_space.end() ? m_right_margin : it->first;
		pos = (left + right) / 2;
		return true;
	}
	void dump()
	{
		std::map<double, unsigned int>::const_iterator it;
		for(it = m_space.begin(); it != m_space.end(); ++it)
			std::cout << "-- " << it->first << std::endl << " " << std::setw(3) << it->second << std::endl;
		std::cout << "-- " << m_right_margin << std::endl;
	}
};

/** automatically find place for new vertical line(s)
*
* \arg col index of column to split (0 - leftmost column)
* \return true if line was successfully added
*/
bool Tabulator::Grid::auto_split_column(unsigned int col, const Tabulator::Metafile * mf)
{
	KnotsIterator kit;
	double x1, x2;
	kit = h_knots.begin();
	for(unsigned int i = 0; i < col - 1; i++) {
		kit++;
		if(kit == h_knots.end())
			return false;
	}
	x1 = kit->first;
	kit++; /* pointer to second verical line */
	if(kit == h_knots.end())
		return false; // XXX XXX XXX should find right border
	x2 = kit->first;
	if(debug)
		std::clog << "Splitting " << col << "th column (between " << x1 << " and " << x2 << ")" << std::endl;

	IntervalHystogram h(x1, x2);

	// Collect some stats
	std::map<Coord, unsigned int> xpos_hits;
	Tabulator::Metafile::TextMap::const_iterator tit; // text iterator
	for(tit = mf->all_text.begin(); tit != mf->all_text.end() && tit->pos.y < margins.bottom; tit++) { /* check all text */
		if(tit->pos.y < headers_end) // skip table header
			continue;
		if(tit->pos.x < x1 || tit->pos.x >= x2) // skip other columns
			continue;
		h.use(tit->pos.x, tit->pos.x + tit->width);
	}
	if(debug)
		h.dump();

	h.trim();
	double x = x1;
	unsigned int count = 0;
	while(h.find_next_unused_space(x)) {
		if(debug)
			std::clog << "Adding vertical line at " << x << std::endl;
		split_column(col, x);
		++count;
	}
	return count != 0;
}

void Tabulator::Grid::build(const Tabulator::Metafile& metafile, const Tabulator::Options& options)
{
	Tabulator::Metafile::LineMap::const_iterator lit;
	Coord cur;

	Coord lowest_h_line(-1E10);
	Coord lowest_v_line(-1E10);

	if(metafile.h_lines.empty() || metafile.v_lines.empty())
		return;

	// 1. Find vertical knots (y-coords of all horisontal lines)
	for(cur = -1E20, lit = metafile.h_lines.begin(); lit != metafile.h_lines.end(); lit++) {
		if(cur != lit->first) { // add knot
								//			std::clog << "V-knot at " << (double)lit->first << std::endl;
			cur = lit->first;
			v_knots.insert(Tabulator::Grid::KnotsMap::value_type(cur, Grid::Line()));
		}
	}
	lowest_h_line = metafile.h_lines.rbegin()->first;

	// 2. Find horisontal knots (x-coords of all vertial lines)
	for(cur = -1E20, lit = metafile.v_lines.begin(); lit != metafile.v_lines.end(); lit++) {
		if(cur != lit->first) { // add knot
								//			std::clog << "H-knot at " << (double)lit->first << std::endl;
			cur = lit->first;
			h_knots.insert(Grid::KnotsMap::value_type(cur, Grid::Line()));
			// Note lowest line end
		}
		if(lit->second.second > lowest_v_line)
			lowest_v_line = lit->second.second;
	}

	if(debug)
		std::clog << "Lowest H-line: " << lowest_h_line << ", lowest V-line: " << lowest_v_line << std::endl;
	margins.bottom = lowest_h_line;

	// 3. Add final hor. line at table end (at vert. lines end)
	if(lowest_v_line != lowest_h_line) {
		if(debug)
			std::clog << "Adding final hor line at " << lowest_v_line << std::endl;
		v_knots.insert(Grid::KnotsMap::value_type(lowest_v_line, Grid::Line()));
		margins.bottom = lowest_v_line;
	}

	if(options.find_table_header) {
		if(lowest_h_line < lowest_v_line / 2.0) {
			headers_end = lowest_h_line;
			if(debug)
				std::clog << "Headers end seems to be on last horizontal line, at " << headers_end << std::endl;
		}
		else {
			Grid::KnotsMap::reverse_iterator rit = v_knots.rbegin();
			rit++;
			if(rit->first < lowest_v_line / 2.0) {
				headers_end = rit->first;
				if(debug)
					std::clog << "Headers end seems to be on pre-last horizontal line, at " << headers_end << std::endl;
			}
		}
	}

	if(options.find_more_rows_column < 0) {
		/* divide all text blocks into lines and place new knots between them */
		double table_bottom = lowest_v_line;
		if(lowest_h_line > table_bottom)
			table_bottom = lowest_h_line;
		Tabulator::Metafile::TextMap::const_iterator tit; // text iterator
		for(tit = metafile.all_text.begin(); tit != metafile.all_text.end() && tit->pos.y < table_bottom && tit->pos.y < headers_end; tit++); /* skip heades */
		double text_bottom = tit->pos.y;
		for(; tit != metafile.all_text.end() && tit->pos.y < table_bottom; tit++) { /* check all text */
			if(tit->pos.y - tit->height <= text_bottom) { // same text line
				double tbottom = tit->pos.y;
				if(tbottom > text_bottom)
					text_bottom = tbottom;
				continue;
			}
			// fount new line
			double newknot = (text_bottom + tit->pos.y - tit->height) / 2;
			add_horizontal_line(newknot);
			text_bottom = tit->pos.y;
		}
	}
	else if(options.find_more_rows_column > 0 && h_knots.size() >= 2) {
		/*
		* We have no horizontal lines, so let there be some invisible
		* lines above text blocks, that falls into specified column
		*/
		Grid::KnotsIterator kit;
		double x1, x2;
		kit = h_knots.begin();
		for(int i = 0; i < options.find_more_rows_column - 1 && kit != h_knots.end(); i++)
			kit++;
		if(kit == h_knots.end())
			throw TabulatorException("Bad options.find_more_rows_column");
		x1 = kit->first;
		kit++; /* pointer to second vertical line */
		x2 = kit->first;
		if(debug)
			std::clog << "Adding more rows: " << options.find_more_rows_column << "th column is between " << x1 << " and " << x2 << std::endl;

		Tabulator::Metafile::TextMap::const_iterator tit; // text iterator
		Coord cur_y = -1E10;
		double prev_y = -1E10;
		double table_bottom = lowest_v_line;
		if(lowest_h_line > table_bottom)
			table_bottom = lowest_h_line;
		for(tit = metafile.all_text.begin(); tit != metafile.all_text.end() && tit->pos.y < table_bottom; tit++) { /* check all text */
			if(tit->pos.y < headers_end) // skip header
				continue;
			if(tit->pos.x >= x1 && tit->pos.x < x2 && tit->pos.y != cur_y) {
				if(debug)
					std::clog << "Adding line above text string @" << tit->pos.dump() << std::endl;
				prev_y = cur_y;
				cur_y = tit->pos.y;
				double half_interval = (prev_y < 0) ? Coord(tit->height) / 10.0 : (cur_y - tit->height - prev_y) / 2.0;
				add_horizontal_line(cur_y - tit->height - half_interval);
			}
		}
	} // option.find_more_rows

	if(options.find_joined_cells) {
		Grid::KnotsMap::iterator kit;
		Grid::KnotsIterator kit2;

		// Fill vertical knot's arrays (line in n'th column present)
		for(lit = metafile.h_lines.begin(); lit != metafile.h_lines.end(); lit++) {
			kit = v_knots.find(lit->first);
			if(kit == v_knots.end())
				continue;
			kit->second.resize(h_knots.size() - 1, false);
			int e1 = -1;
			int e2 = -1;
			int e;
			for(kit2 = h_knots.begin(), kit2++, e = 0; kit2 != h_knots.end(); kit2++, e++) {
				if(lit->second.first < kit2->first && e1 < 0)
					e1 = e;
				if(e1 >= 0)
					e2 = e;
				if(lit->second.second < kit2->first)
					break;
			}
			if(e1 >= 0 && e2 >= 0)
				kit->second.set_bits(e1, e2);
		}
		// Fill horizontal knot's arrays (line in nth row present)
		for(lit = metafile.v_lines.begin(); lit != metafile.v_lines.end(); lit++) {
			kit = h_knots.find(lit->first);
			if(kit == h_knots.end())
				continue;
			kit->second.resize(v_knots.size() - 1, false);
			int e1 = -1;
			int e2 = -1;
			int e;
			for(kit2 = v_knots.begin(), kit2++, e = 0; kit2 != v_knots.end(); kit2++, e++) {
				if(lit->second.first < kit2->first && e1 < 0)
					e1 = e;
				if(e1 >= 0)
					e2 = e;
				if(lit->second.second < kit2->first)
					break;
			}
			if(e1 >= 0 && e2 >= 0)
				kit->second.set_bits(e1, e2);
		}
	}

	std::vector<int> split_columns(options.split_columns);
	std::sort(split_columns.begin(), split_columns.end());
	for(std::vector<int>::const_reverse_iterator it = split_columns.rbegin(); it != split_columns.rend(); ++it) {
		auto_split_column(*it, &metafile);
	}
}

/** Return object dump for debugging purposes */
std::string Tabulator::Grid::dump() const
{
	std::stringstream ss;
	KnotsIterator it;
	ss << "Grid: " << h_knots.size() << " h-knots, " << v_knots.size() << " v-knots" << std::endl;
	ss << " H-knots (vertical lines) is at:" << std::endl;
	for(it = h_knots.begin(); it != h_knots.end(); it++)
		ss << "  " << std::setw(6) << std::fixed << std::setprecision(1) << (double)it->first << "(" << it->second.dump() << ")" << std::endl;
	ss << std::endl;
	ss << " V-knots (horizontal lines) is at:" << std::endl;
	for(it = v_knots.begin(); it != v_knots.end(); it++)
		ss << "  " << std::setw(6) << std::fixed << std::setprecision(1) << (double)it->first << "(" << it->second.dump() << ")" << std::endl;
	ss << std::endl;
	return ss.str();
}



