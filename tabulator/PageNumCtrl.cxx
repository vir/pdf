#include "PageNumCtrl.hpp"

BEGIN_EVENT_TABLE(PageNumCtrl, wxTextCtrl)
	EVT_CHAR      (PageNumCtrl::OnChar)
	EVT_SET_FOCUS (PageNumCtrl::OnSetFocus)
END_EVENT_TABLE()

PageNumCtrl::PageNumCtrl(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
	: wxTextCtrl(parent, id, value, pos, size, style, wxDefaultValidator, name), num(0), min(0), max(10000)
{
}

void PageNumCtrl::OnChar(wxKeyEvent & event)
{
	wxString backup = wxTextCtrl::GetValue();
	long tmp;
	if(event.GetKeyCode() < 32 || event.GetKeyCode() == 127 || event.GetKeyCode() > 256)
		event.Skip();
	if(event.GetKeyCode() > 255 || !isdigit(event.GetKeyCode()))
		return;
	//			EmulateKeyPress(event); // causes infinite recursion on win32
	event.Skip();
	wxString newval = wxTextCtrl::GetValue();
	if(!newval.ToLong(&tmp) || tmp<min || tmp>max) {
		wxTextCtrl::SetValue(backup);
		wxBell();
	}
	else
		num = tmp;
}

void PageNumCtrl::OnSetFocus(wxFocusEvent & event)
{
	SelectAll();
	event.Skip();
}

void PageNumCtrl::SetRange(int r1, int r2)
{
	min = r1;
	max = r2;
}

void PageNumCtrl::SetValue(int i)
{
	num = i;
	wxTextCtrl::SetValue(wxString::Format(_T("%d"), i));
}

int PageNumCtrl::GetValue()
{
	long tmp;
	wxString newval = wxTextCtrl::GetValue();
	if(newval.ToLong(&tmp) || tmp<min || tmp>max) {
		num = tmp;
	}
	return num;
}
