#ifndef EXCEL_HPP_INCLUDED
#define EXCEL_HPP_INCLUDED

class Excel
{
	private:
		VARIANT app;
	public:
		Excel();
		~Excel();
		bool open();
		bool create();
		void set_visible(bool v);
		void add_workbook();
		void set_cell_value(std::wstring s, int offset_c = 0, int offset_r = 0);
		bool save_as(std::wstring fname);
		bool quit();
};



#endif /* EXCEL_HPP_INCLUDED */

