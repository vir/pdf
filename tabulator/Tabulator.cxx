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

	grid.build(metafile, options);
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

