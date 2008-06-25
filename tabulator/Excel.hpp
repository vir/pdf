#ifndef EXCEL_HPP_INCLUDED
#define EXCEL_HPP_INCLUDED

#include <ole2.h>
#include <string>

class Excel
{
	private:
		IDispatch * app;
	public:
		Excel();
		virtual ~Excel();
		bool get_active();
		bool start_new();
		void set_visible(bool v);
		void add_workbook();
		void move_cursor(int offset_c, int offset_r);
		void move_cursor(std::wstring cellname);
		void join_cells(int ncols, int nrows, int col = 0, int row = 0);
//		void join_cells(int row, int col, int ncols, int nrows);
		void set_cell_value(std::wstring s, int offset_c = 0, int offset_r = 0);
		void set_cell_format(int rotation, bool wrap_text = false, bool shrink_to_fit = false, int row = 0, int col = 0);
//		void set_cell_align(int row, int col, int halign, int valign, int indent = 0);
		bool save_as(std::wstring fname);
		bool quit();
};



#endif /* EXCEL_HPP_INCLUDED */

