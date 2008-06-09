#ifndef TABULATOR_EXPORTER_HPP_INCLUDED
#define TABULATOR_EXPORTER_HPP_INCLUDED

// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include "Tabulator.hpp"
#include <iostream>

class ExporterCSV:public Tabulator::Table::Exporter
{
	private:
		std::ostream & s;
		bool need_delimiter;
	public:
		char delimiter;
		ExporterCSV(std::ostream & os):s(os),need_delimiter(false),delimiter(';') {}
//		virtual void table_begin(unsigned int ncols, unsigned int nrows);
//		virtual void row_begin(unsigned int r);
		virtual void cell(std::wstring text, unsigned int c, unsigned int r);
//		virtual void cell(const Cell * cptr, unsigned int c, unsigned int r);
		virtual void row_end();
//		virtual void table_end();
};

class ExporterHTML:public Tabulator::Table::Exporter
{
	private:
		std::ostream & s;
	public:
		ExporterHTML(std::ostream & os):s(os) {}
		virtual void table_begin(unsigned int ncols, unsigned int nrows);
		virtual void row_begin(unsigned int r);
//		virtual void cell(std::wstring text, unsigned int c, unsigned int r);
		virtual void cell(const Tabulator::Table::Cell * cptr, unsigned int c, unsigned int r);
		virtual void row_end();
		virtual void table_end();
};

#ifdef _WIN32
#include "Excel.hpp"
class ExporterExcel
	:public Tabulator::Table::Exporter
	,public Excel
{
	private:
		unsigned int rows_number;
	public:
		virtual void table_begin(unsigned int ncols, unsigned int nrows) { rows_number = nrows; }
		virtual void cell(const Tabulator::Table::Cell * cptr, unsigned int c, unsigned int r);
		virtual void table_end() { move_cursor(0, rows_number); }
};
#endif /* _WIN32 */

#endif /* TABULATOR_EXPORTER_HPP_INCLUDED */

