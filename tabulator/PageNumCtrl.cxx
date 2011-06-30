#include "PageNumCtrl.hpp"

BEGIN_EVENT_TABLE(PageNumCtrl, wxTextCtrl)
	EVT_CHAR      (PageNumCtrl::OnChar)
	EVT_SET_FOCUS (PageNumCtrl::OnSetFocus)
END_EVENT_TABLE()
