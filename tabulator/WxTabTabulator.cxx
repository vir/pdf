#include "WxTabTabulator.hpp"
#include <wx/dc.h>
#include <wx/gdicmn.h>
#include <wx/spinctrl.h>
#include <wx/valgen.h>

WxTabTabulator * theTabulator = NULL;


class WxTabOptionsDialog:public wxDialog
{
	public:
		WxTabOptionsDialog( wxWindow * parent, struct Tabulator::Options * opts );
		~WxTabOptionsDialog();
		virtual bool Validate();
	private:
		wxCheckBox * ui_find_table_header;
		wxSpinCtrl * ui_find_more_rows_column;
		wxCheckBox * ui_postprocess;
		wxCheckBox * ui_find_joined_cells;
		struct Tabulator::Options * o;
		void OnOk( wxCommandEvent & event );
//		DECLARE_EVENT_TABLE()
};

/*============================ WxTabTabulator ===================*/

void WxTabTabulator::Draw(wxDC * dc)
{
	wxSize cs = dc->GetSize();

	// Draw grid
	dc->SetPen(wxPen(*wxRED, 0, wxSOLID));
	Grid::KnotsIterator kit;
	for(kit = grid.h_knots.begin(); kit != grid.h_knots.end(); kit++) {
		dc->DrawLine(kit->first, 0, kit->first, cs.GetHeight());
	}
	for(kit = grid.v_knots.begin(); kit != grid.v_knots.end(); kit++) {
		dc->DrawLine(0, kit->first, cs.GetWidth(), kit->first);
	}
	if(grid.headers_end >= 0) {
		dc->SetPen(wxPen(*wxGREEN, 3, wxSHORT_DASH));
		dc->DrawLine(0, grid.headers_end, cs.GetWidth(), grid.headers_end);
	}

	// Draw cells
}

void WxTabTabulator::ShowOptionsDialog(wxWindow * parent)
{
	WxTabOptionsDialog opts(parent, &theTabulator->options);
	opts.ShowModal();
}

/*============================ Tabulator options dialog =========*/

#if 0
		struct Options {
			bool find_table_header;
			unsigned int find_more_rows_column;
			bool postprocess;
			bool find_joined_cells;
		} options;
#endif

WxTabOptionsDialog::WxTabOptionsDialog( wxWindow * parent, struct Tabulator::Options * opts )
	: wxDialog( parent, wxID_ANY, _("Tabulator options"), wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER),o(opts)
{
	wxBoxSizer * s = new wxBoxSizer( wxVERTICAL /* wxHORIZONTAL */ );

	ui_find_table_header = new wxCheckBox(this, wxID_ANY, _T("Find table headers"));
	ui_find_more_rows_column = new wxSpinCtrl(this, wxID_ANY, _T("0"));
	ui_postprocess = new wxCheckBox(this, wxID_ANY, _T("Postprocess"));
	ui_find_joined_cells = new wxCheckBox(this, wxID_ANY, _T("Find joined cells"));

	ui_find_table_header->SetValidator(wxGenericValidator(&opts->find_table_header));
	ui_find_more_rows_column->SetValidator(wxGenericValidator((int*)&opts->find_more_rows_column));
	ui_postprocess->SetValidator(wxGenericValidator(&opts->postprocess));
	ui_find_joined_cells->SetValidator(wxGenericValidator(&opts->find_joined_cells));

	s->Add(ui_find_table_header);
	s->Add(ui_find_more_rows_column);
	s->Add(ui_postprocess);
	s->Add(ui_find_joined_cells);

	s->Add(new wxButton( this, wxID_OK, _("OK") ));
	SetAutoLayout( TRUE );
	SetSizer( s );
	s->Fit( this );
	s->SetSizeHints( this );
}

WxTabOptionsDialog::~WxTabOptionsDialog()
{
	delete ui_find_joined_cells;
	delete ui_postprocess;
	delete ui_find_more_rows_column;
}

bool WxTabOptionsDialog::Validate()
{
	return true;
}

void WxTabOptionsDialog::OnOk(wxCommandEvent & event)
{
	event.Skip();
}






