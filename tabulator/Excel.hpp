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
		void set_cell_value(std::wstring s, int offset_c = 0, int offset_r = 0);
		bool save_as(std::wstring fname);
		bool quit();
};



#endif /* EXCEL_HPP_INCLUDED */

