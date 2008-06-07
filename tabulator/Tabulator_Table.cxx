#include "Tabulator.hpp"

/*============ Table Cell ===========================*/

std::wstring Tabulator::Table::Cell::celltext() const
{
	std::wstring r;
	std::map<PDF::Point, std::wstring>::const_iterator tpit;
	for(tpit=text.begin(); tpit!=text.end(); tpit++)
	{
		if(!r.empty()) {
			/// remove hyphenation XXX \todo mess with unicode hypheation character (if any?)
			if(!r.empty() && r[r.size()-1] == L'-') // remove hyphenation
				r.resize(r.size()-1);
			else
				r+=L' ';
		}
		/// \todo Mwss with text block width and coordinate comparsition and catch newlines also

		r+=tpit->second;
	}
	return r;
}

/*============ Table ================================*/

void Tabulator::Table::resize(unsigned int cols, unsigned int rows)
{
	cells.resize(rows);
	for(unsigned int i = 0; i < cells.size(); i++) {
		cells[i].resize(cols, NULL);
	}
}

Tabulator::Table::Cell * Tabulator::Table::cell(unsigned int col, unsigned int row, bool create)
{
	if(row >= cells.size() || col >= cells[row].size())
		return NULL;
	Cell * r = cells[row][col];
	if(!r && create) {
		std::list<Cell>::iterator it = all_cells.insert(all_cells.end(), Cell());
		r = &(*it);
		cells[row][col] = r;
	}
	return r;
}

const Tabulator::Table::Cell * Tabulator::Table::cell(unsigned int col, unsigned int row) const
{
	return cells[row][col];
}

void Tabulator::Table::output(Tabulator::Table::Exporter * ex) const
{
	unsigned int r, c;
	if(!ex)
		return;
	ex->table_begin(cells.size(), cells.size()?cells[0].size():0);
	for(r = 0; r < cells.size(); r++) {
		ex->row_begin(r);
		for(c = 0; c < cells[r].size(); c++) {
			ex->cell(cell(r, c), r, c);
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


