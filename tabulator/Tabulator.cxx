// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include <iostream>
#include <string>

#include <libpdf/PDF.hpp>
#include "Tabulator.hpp"

#if 0 /* unimplemented! */
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

	if(!metafile.h_lines.size() || !metafile.v_lines.size())
		return;

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

	if(options.find_more_rows && grid.h_knots.size() >= 2) {
	/* 
	 * We have no horizontal lines, so let's there be some invisible
	 * lines above text blocks, that falls into first column
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
		double table_bottom = lowest_v_line;
		if(lowest_h_line > table_bottom)
			table_bottom = lowest_h_line;
		for(tit = metafile.all_text.begin(); tit != metafile.all_text.end() && tit->pos.y < table_bottom; tit++) { /* check all text */
			if(lowest_h_line < lowest_v_line && tit->pos.y < lowest_h_line) // skip seader
				continue;
			if(tit->pos.x >= x1 && tit->pos.x < x2 && tit->pos.y != cur_y) {
				std::clog << "Adding line above text string @" << tit->pos.dump() << std::endl;
				cur_y = tit->pos.y;
				Grid::KnotsMap::iterator iit = grid.v_knots.insert(Grid::KnotsMap::value_type(cur_y - tit->height, Grid::Line(grid.h_knots.size()-1, true))).first;
			}
		}
	} // option.find_more_rows

	if(options.find_joined_cells) {
		Grid::KnotsMap::iterator kit;
		Grid::KnotsIterator kit2;

		// Fill vertical knot's arrays (line in n'th column present)
		for(lit = metafile.h_lines.begin(); lit != metafile.h_lines.end(); lit++) {
			kit = grid.v_knots.find(lit->first);
			if(kit == grid.v_knots.end())
				continue;
			kit->second.resize(grid.h_knots.size()-1, false);
			int e1 = -1;
			int e2 = -1;
			int e;
			for(kit2 = grid.h_knots.begin(), kit2++, e = 0; kit2 != grid.h_knots.end(); kit2++, e++) {
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
			kit = grid.h_knots.find(lit->first);
			if(kit == grid.h_knots.end())
				continue;
			kit->second.resize(grid.v_knots.size()-1, false);
			int e1 = -1;
			int e2 = -1;
			int e;
			for(kit2 = grid.v_knots.begin(), kit2++, e = 0; kit2 != grid.v_knots.end(); kit2++, e++) {
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
}

void Tabulator::prepare_table()
{
	table.clear();
	if(!grid.h_knots.size() || !grid.v_knots.size())
		return;
	table.resize(grid.h_knots.size()-1, grid.v_knots.size()-1);

	if(options.find_joined_cells) {
		Tabulator::Grid::KnotsIterator it_row, it_col, ri, ci;
		unsigned int row, col;
		// Check all cells
		for(it_row = grid.v_knots.begin(), it_row++, row = 0; it_row != grid.v_knots.end(); it_row++, row++) {
			for(it_col = grid.h_knots.begin(), it_col++, col = 0; it_col != grid.h_knots.end(); it_col++, col++) {
				unsigned int cs = 0;
				unsigned int rs = 0;
				unsigned int c = col;
				unsigned int r = row;
				for(ri = it_row, r = row; ri != grid.v_knots.end(); ri++, r++) {
					for(ci = it_col, c = col; ci != grid.h_knots.end(); ci++, c++) {
						if(ci->second[r]) {
							cs = c - col;
							break;
						}
					} // c
					break; // XXX Add rowspan calc
				} // r
if(cs || rs) std::clog << "Spans: row " << row << ", col " << col << ": " << cs+1 << "x" << rs+1 << std::endl;
				if(cs || rs)
					table.span(col, row, cs+1, rs+1);
			} // col
		} // row
	}
}

void Tabulator::fill_table_with_text()
{
	Tabulator::Metafile::TextMap::iterator tit; // text iterator
	Tabulator::Grid::KnotsIterator rit; // table rows iterator

	double header_y = 0;
	/* all_text is sorted by y, then x coord */
	int row = 0;
	Tabulator::Grid::KnotsIterator cit; // table cols iterator
	int col;
	rit = grid.v_knots.begin();
	row = -1;
	bool reached_end_of_table = false;
	for(tit = metafile.all_text.begin(); tit != metafile.all_text.end(); tit++) {
		if(!reached_end_of_table && tit->pos.y > double(rit->first)) { // reached next row
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

		try { col = grid.find_col(tit->pos.x); } catch(...) { col = -1; }

		if(row>=0 && col>=0) {
//			std::wclog << L"Text in table(" << col << L',' << row << "): \"" << tit->second << "\"" << std::endl;
			if(tit->text.length())
				table.cell(col, row)->addtext(*tit);
			if(tit->pos.y < header_y)
				table.cell(col, row)->is_header=true;
		} else {
			std::wclog << L"Text at " << tit->pos.x << L',' << tit->pos.y << L" is not in table: \"" << tit->text << "\"" << std::endl;
		}
	}
}

void Tabulator::full_process(PDF::Page * page)
{
	flush();
	load_page(page);

	build_grid();
	std::clog << grid.dump();

	prepare_table();

	fill_table_with_text();
	std::clog << table.dump();

	if(options.postprocess)
		table.postprocess();
}

void Tabulator::dump() const
{
	std::clog << "Tabulator dump:" << std::endl;
//	metafile.dump();
	std::clog << grid.dump();
	std::clog << table.dump();
}

unsigned int Tabulator::Table::nrows() const
{
	return cells.size();
}

unsigned int Tabulator::Table::ncols() const
{
	return cells.size()?cells[0].size():0;
}
