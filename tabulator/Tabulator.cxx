// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include <iostream>
#include <string>

#include <libpdf/PDF.hpp>
#include "Tabulator.hpp"

bool Tabulator::debug = false;

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

	if(metafile.h_lines.empty() || metafile.v_lines.empty())
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

	if(debug)
		std::clog << "Lowest H-line: " << lowest_h_line << ", lowest V-line: " << lowest_v_line << std::endl;
	grid.margins.bottom = lowest_h_line;

	// 3. Add final hor. line at table end (at vert. lines end)
	if(lowest_v_line != lowest_h_line) {
		if(debug)
			std::clog << "Adding final hor line at " << lowest_v_line << std::endl;
		grid.v_knots.insert(Grid::KnotsMap::value_type(lowest_v_line, Grid::Line()));
		grid.margins.bottom = lowest_v_line;
	}

	if(options.find_table_header) {
		if(lowest_h_line < lowest_v_line/2.0) {
			grid.headers_end = lowest_h_line;
			if(debug)
				std::clog << "Headers end seems to be on last horizontal line, at " << grid.headers_end << std::endl;
		} else {
			Grid::KnotsMap::reverse_iterator rit = grid.v_knots.rbegin();
			rit++;
			if(rit->first < lowest_v_line/2.0) {
				grid.headers_end = rit->first;
				if(debug)
					std::clog << "Headers end seems to be on pre-last horizontal line, at " << grid.headers_end << std::endl;
			}
		}
	}

	if(options.find_more_rows_column && grid.h_knots.size() >= 2) {
	/* 
	 * We have no horizontal lines, so let there be some invisible
	 * lines above text blocks, that falls into specified column
	 */
		Grid::KnotsIterator kit;
		double x1, x2;
		kit = grid.h_knots.begin();
		for(unsigned int i = 0; i < options.find_more_rows_column - 1; i++)
			kit++;
		x1 = kit->first;
		kit++; /* pointer to second verical line */
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
			if(tit->pos.y < grid.headers_end) // skip header
				continue;
			if(tit->pos.x >= x1 && tit->pos.x < x2 && tit->pos.y != cur_y) {
				if(debug)
					std::clog << "Adding line above text string @" << tit->pos.dump() << std::endl;
				prev_y = cur_y;
				cur_y = tit->pos.y;
				double half_interval = (prev_y < 0) ? Coord(tit->height)/10.0 : (cur_y - tit->height - prev_y)/2.0;
				grid.add_horizontal_line(cur_y - tit->height - half_interval);
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

	std::sort(options.split_columns.begin(), options.split_columns.end());
	for(std::vector<int>::const_reverse_iterator it = options.split_columns.rbegin(); it != options.split_columns.rend(); ++it) {
		grid.auto_split_column(*it, &metafile);
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
				if(table.is_hidden(col, row))
					continue; // skip cells, hidden by precedin spans
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
					break; // XXX \todo Add rowspan calc
				} // r
//if(cs || rs) std::clog << "Spans: row " << row << ", col " << col << ": " << cs+1 << "x" << rs+1 << std::endl;
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

	if(table.nrows() == 0 || table.ncols() == 0)
		return;

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

		col = grid.find_col(tit->pos.x);

		if(row>=0 && col>=0) {
//			std::wclog << L"Text in table(" << col << L',' << row << "): \"" << tit->second << "\"" << std::endl;
			if(tit->text.length())
				table.cell(col, row)->addtext(*tit);
			if(tit->pos.y < grid.headers_end)
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
	if(debug)
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

