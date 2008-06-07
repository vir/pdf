// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include <iostream>
#include <string>

#include <libpdf/PDF.hpp>
#include "Tabulator.hpp"

/* parser of tables such as
 *  .-----------------.
 *  | h1  |  h2  | h3 |
 *  `-----------------'
 *   d11   d12    d13
 *   d21   d22    d23
 *   ...
 * (updated for gng-2007 parsing)
 */

#if 0
void Tabulator::set_area(PDF::Rect r)
void Tabulator::set_rotation(int rot)
void Tabulator::set_tolerance(double tx, double ty)
#endif

void Tabulator::load_page(PDF::Page * page)
{
	metafile.Clear();
	page->draw(&metafile);
}

void Tabulator::build_grid()
{
	Tabulator::Metafile::LineMap::const_iterator lit;
	Coord cur;

	Coord lowest_h_line(-1E10);
	Coord lowest_v_line(-1E10);

	// 1. Find vertical knots (y-coords of all horisontal lines)
	for(cur = -1E20, lit = metafile.h_lines.begin(); lit != metafile.h_lines.end(); lit++) {
		if(cur != lit->first) { // add knot
//			std::clog << "V-knot at " << (double)lit->first << std::endl;
			cur = lit->first;
			grid.v_knots.insert(Grid::KnotsMap::value_type(cur, Grid::Line()));
		}
	}
	lowest_h_line = metafile.h_lines.rbegin()->first;

	// 2. Find horisontal knots (x-coords of all vertial lines)
	for(cur = -1E20, lit = metafile.v_lines.begin(); lit != metafile.v_lines.end(); lit++) {
		if(cur != lit->first) { // add knot
//			std::clog << "H-knot at " << (double)lit->first << std::endl;
			cur = lit->first;
			grid.h_knots.insert(Grid::KnotsMap::value_type(cur, Grid::Line()));
			// Note lowest line end
		}
		if(lit->second.second > lowest_v_line)
			lowest_v_line = lit->second.second;
	}

	std::clog << "Lowest H-line: " << lowest_h_line << ", lowest V-line: " << lowest_v_line << std::endl;

	// 3. Add final hor. line at table end (at vert. lines end)
	if(lowest_v_line != lowest_h_line) {
		std::clog << "Adding final hor line at " << lowest_v_line << std::endl;
		grid.v_knots.insert(Grid::KnotsMap::value_type(lowest_v_line, Grid::Line()));
	}

	// 3. Fill vertical knot's arrays (line in n'th column present)
	// 4. Fill horizontal knot's arrays (line in nth row present)

	if(options.find_more_rows && grid.h_knots.size() >= 2) {
	/* 
	 * We have no horizontal lines, so
	 * let's assume lines some little space (eg. 10 units) above text, that falls
	 * into first column
	 */
		Grid::KnotsIterator kit;
		double x1, x2;
		kit = grid.h_knots.begin();
		for(unsigned int i = 0; i < options.find_rows_column; i++)
			kit++;
		x1 = kit->first;
		kit++; /* pointer to second verical line */
		x2 = kit->first;
		std::clog << "First column is between " << x1 << " and " << x2 << std::endl;
		std::clog << "Second vertical line is at x " << lit->first << std::endl;

		Tabulator::Metafile::TextMap::const_iterator tit; // text iterator
		Coord cur_y = -1E10;
		for(tit = metafile.all_text.begin(); tit != metafile.all_text.end(); tit++) { /* check all text */
			if(lowest_h_line < lowest_v_line && tit->first.y < lowest_h_line) // skip seader
				continue;
			if(tit->first.x >= x1 && tit->first.x < x2 && tit->first.y != cur_y) {
				std::clog << "Adding line above text string @" << tit->first.dump() << std::endl;
				cur_y = tit->first.y;
				grid.v_knots.insert(Grid::KnotsMap::value_type(cur_y - 10.0, Grid::Line()));
			}
		}
	} // option.find_more_rows

}

void Tabulator::fill_table_with_text()
{
	table.clear();
	table.resize(grid.h_knots.size()-1, grid.v_knots.size()-1);

	Tabulator::Metafile::TextMap::iterator tit; // text iterator
	Tabulator::Grid::KnotsIterator rit; // table rows iterator

	double header_y = 0;
	/* all_text is sorted by y, then x coord */
	int row = 0;
#if 1
	Tabulator::Grid::KnotsIterator cit; // table cols iterator
	int col;
	rit = grid.v_knots.begin();
	row = -1;
	bool reached_end_of_table = false;
	for(tit = metafile.all_text.begin(); tit != metafile.all_text.end(); tit++) {
		if(!reached_end_of_table && tit->first.y > double(rit->first)) { // reached next row
			rit++;
			if(rit != grid.v_knots.end()) { // still in table
//				std::clog << "Reached next row @ " << double(rit->first) << std::endl;
				row++;
			} else { // reached last line
//				std::clog << "Reached end of table" << std::endl;
				row = -1;
				reached_end_of_table = true;
			}
		}

		try { col = grid.find_col(tit->first.x); } catch(...) { col = -1; }

		if(row>=0 && col>=0) {
//			std::wclog << L"Text in table(" << col << L',' << row << "): \"" << tit->second << "\"" << std::endl;
			table.cell(col, row)->addtext(tit->first, tit->second);
			if(tit->first.y < header_y)
				table.cell(col, row)->is_header=true;
		} else {
			std::wclog << L"Text is not in table: \"" << tit->second << "\"" << std::endl;
		}
	}
#else
	bool in_table = false;
	tit = metafile.all_text.begin();
	for(rit = grid.v_knots.begin(); rit != grid.v_knots.end(); rit++, in_table=true) {
		std::clog << "Checking row " << row << " at y=" << rit->first << std::endl;
		while(tit!=metafile.all_text.end() && tit->first.y < double(rit->first)) {
			if(!in_table) {
				std::wclog << L"Text is not in table: \"" << tit->second << "\"" << std::endl;
				tit++;
				continue;
			}
			unsigned int col;
			try {
				col = grid.find_col( tit->first.x );
//				std::clog << "Text goes to (" << col << "," << row << ")" << std::endl;
				table.cell(col, row)->addtext(tit->first, tit->second);
				if(tit->first.y < header_y)
					table.cell(col, row)->is_header=true;
			}
			catch(...) {
				std::wclog << L"Text is not in table: \"" << tit->second << "\"" << std::endl;
			}
			tit++;
		} // all_text
		if(in_table)
			row++;
	}
#endif
}

void Tabulator::full_process(PDF::Page * page)
{
	load_page(page);

	build_grid();
	std::clog << grid.dump();

	fill_table_with_text();
	std::clog << table.dump();
}



void Tabulator::dump() const
{
	std::clog << "Tabulator dump:" << std::endl;
//	metafile.dump();
	std::clog << grid.dump();
	std::clog << table.dump();
}



