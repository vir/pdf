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
		virtual void cell(const Tabulator::Table::Cell * cptr, bool hidden, unsigned int c, unsigned int r, unsigned int cs, unsigned int rs);
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
		unsigned int sheets_number;
		unsigned int cur_sheet; // number of "sheets" in current "megarow"
		unsigned int cur_col;
		unsigned int rows_number;
		unsigned int cols_number;
		bool do_join_cells;
		bool insert_page_numbers;
	public:
		ExporterExcel():sheets_number(1),cur_sheet(0),cur_col(0),do_join_cells(false),insert_page_numbers(false) {}
		virtual bool set_params(std::string pstr);
		virtual void set_sheets_number(unsigned int n) { sheets_number = n; }
		void set_insert_page_numbers(bool b) { insert_page_numbers = b; }
		virtual void table_begin(unsigned int ncols, unsigned int nrows);
		virtual void page_begin(std::string fname, unsigned int pnum);
		virtual void cell(const Tabulator::Table::Cell * cptr, bool hidden, unsigned int c, unsigned int r, unsigned int cs, unsigned int rs);
		virtual void table_end();
};
#endif /* _WIN32 */

#endif /* TABULATOR_EXPORTER_HPP_INCLUDED */

