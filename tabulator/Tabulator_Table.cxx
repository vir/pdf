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

/*============ Table Cell ===========================*/
unsigned int Tabulator::Table::Cell::mergeblocks()
{
	bool flag = false;;
	std::set<Tabulator::TextBlock>::iterator it1, it2;
	for(it1 = text.begin(); it1 != text.end();) {
		flag = false;
		for(it2 = it1, it2++; it2 != text.end(); it2++) {
			if(it1->merge_ok(*it2)) {
				TextBlock combined = *it1 + *it2;
				text.erase(it1);
				text.erase(it2);
				it1 = text.insert(combined).first;
				flag = true;
				break;
			}
		}
		if(!flag)
			it1++;
	} 
	return text.size();
}

std::wstring Tabulator::Table::Cell::celltext() const
{
	std::wstring r;
	std::set<Tabulator::TextBlock>::const_iterator tpit;
	double x, y;
	if(text.empty())
		return L"";
	x = text.begin()->pos.x + text.begin()->width;
	y = text.begin()->pos.y;
	for(tpit=text.begin(); tpit!=text.end(); tpit++)
	{
		if(! r.empty()) {
			/// remove hyphenation XXX \todo mess with unicode hypheation character (if any?)
			if(0 == Coord::compare(y, tpit->pos.y)) { // On same line
				double diff = tpit->pos.x - x;
				if(abs(diff) > tpit->height / 4)
					r += L' ';
			} else {
				if(r[r.size()-1] == L'-') // remove hyphenation
					r.resize(r.size()-1);
				else
					r+=L' ';
			}
		}
		/// \todo Mess with text block width and coordinate comparison and catch newlines also

		r+=tpit->text;
		x = tpit->pos.x + tpit->width;
		y = tpit->pos.y;
	}
	return r;
}

/*============ Table ================================*/

void Tabulator::Table::resize(unsigned int cols, unsigned int rows)
{
	cells.resize(rows);
	for(unsigned int i = 0; i < cells.size(); i++) {
		cells[i].resize(cols, CellPtr(NULL));
	}
}

Tabulator::Table::Cell * Tabulator::Table::cell(unsigned int col, unsigned int row, bool create)
{
	if(row >= cells.size() || col >= cells[row].size())
		return NULL;
	Cell * r = cells[row][col].get();
	if(!r && create) {
		std::list<Cell>::iterator it = all_cells.insert(all_cells.end(), Cell());
		r = &(*it);
		cells[row][col] = r;
	}
	return r;
}

const Tabulator::Table::Cell * Tabulator::Table::cell(unsigned int col, unsigned int row) const
{
	return cells[row][col].get();
}

void Tabulator::Table::span(unsigned int col, unsigned int row, unsigned int cs, unsigned int rs)
{
	Cell * cc = cell(col, row, true);
	cc->set_spans(cs, rs);
	for(unsigned int r = 0; r < rs; r++) {
		for(unsigned int c = 0; c < cs; c++) {
			if(r || c) {
				cells[row+r][col+c] = cc;
				cells[row+r][col+c].hide();
			}
		} // c
	} // r
}

void Tabulator::Table::postprocess()
{
	std::list<Cell>::iterator it;
	for(it = all_cells.begin(); it != all_cells.end(); it++) {
		it->mergeblocks();
	}
}

void Tabulator::Table::output(Tabulator::Table::Exporter * ex) const
{
	unsigned int r, c;
	if(!ex)
		return;
	ex->table_begin(cells.size()?cells[0].size():0, cells.size());
	for(r = 0; r < cells.size(); r++) {
		ex->row_begin(r);
		for(c = 0; c < cells[r].size(); c++) {
			const Cell * cc = cell(c, r);
			ex->cell(cc, cells[r][c].hidden(), c, r, cc?cc->colspan:0, cc?cc->rowspan:0);
//			ex->cell(cc, c, r);
		}
		ex->row_end();
	}
	ex->table_end();
}

void Tabulator::Table::clear()
{
	cells.resize(0);
	all_cells.resize(0);
}

std::string Tabulator::Table::dump() const
{
	std::stringstream ss;
	unsigned int nrows = cells.size();
	ss << "Table " << (nrows?cells[0].size():0) << 'x' << nrows << ", " << all_cells.size() << " cells total" << std::endl;
	return ss.str();
}


