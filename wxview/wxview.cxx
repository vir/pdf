#include <wx/wx.h>
#include "MyFrame.hpp"
 
class MyApp : public wxApp
{
	public:
		virtual bool OnInit();
		virtual int OnExit() { return 0; }
};

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also declares the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

// `Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
	MyFrame *frame = new MyFrame(_T("My Buggy PDF Viewer"), wxPoint(10, 10), wxSize(700, 600));
	frame->Show(true);
	SetTopWindow(frame);
	return true;
}

