// testExcel.cpp : Defines the entry point for the application.
//

#include "../tabulator/Excel.hpp"

using namespace std;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	Excel e;
	if(	e.get_active() ) {
		//TRACE("Open successed");
	} else {
		//TRACE("Open failed");
		e.start_new();
		e.add_workbook();
	}
	e.set_visible(true);
	e.move_cursor(2, 1);
	e.set_cell_value(L"Hello!");
	e.move_cursor(-1, 0);
	return 0;
}



