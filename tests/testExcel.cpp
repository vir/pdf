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
	bool reused;
	if(	e.get_active() ) {
		reused = true;
		//TRACE("Open successed");
	} else {
		//TRACE("Open failed");
		e.start_new();
		e.add_workbook();
		reused = false;
	}
	e.set_visible(true);

	e.move_cursor(L"B2");
	e.set_cell_value(reused?L"Reused already running excel":L"Started new excel");

	e.move_cursor(-1, 2);
	e.set_cell_value(L"Hello!");

	e.move_cursor(3, 0);
	e.set_cell_value(L"D4 i guess?");

	e.move_cursor(-3, 2);
	e.join_cells(4, 2);
	e.set_cell_value(L"Joined 4x2");

	e.move_cursor(0, 1);
	e.set_cell_format(45);
	e.set_cell_value(L"Rotated 45");
	e.move_cursor(1, 0);
	e.set_cell_format(90);
	e.set_cell_value(L"Rotated 90");
	e.move_cursor(1, 0);
	e.set_cell_format(-90);
	e.set_cell_value(L"Rotated -90");
	e.move_cursor(-2, 2);

	std::wstring long_text = L"Very very long text. Realy long. And even longer.";
	e.set_cell_format(0, false, true);
	e.set_cell_value(long_text);

	e.move_cursor(1, 0);
	e.set_cell_format(0, true, false);
	e.set_cell_value(long_text);

	e.set_cell_format(-85, true, false, 3, 0);
	e.set_cell_value(L"Joined at offset", 3, 0);
	e.join_cells(1, 5, 3, 0);

	return 0;
}



