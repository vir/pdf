#include "WxTabTabulator.hpp"
#include <wx/dc.h>
#include <wx/gdicmn.h>
#include <wx/spinctrl.h>
#include <wx/valgen.h>

class WxTabOptionsDialog:public wxDialog
{
	public:
		WxTabOptionsDialog( wxWindow * parent, struct Tabulator::Options * opts );
		~WxTabOptionsDialog();
		virtual bool Validate();
	private:
		wxCheckBox * ui_find_table_header;
		wxSpinCtrl * ui_find_more_rows_column, * ui_split_column;
		wxCheckBox * ui_postprocess;
		wxCheckBox * ui_find_joined_cells;
		wxStaticText * ui_find_more_rows_label, * ui_split_column_label;
		int m_split_column_num;
		struct Tabulator::Options * opts;
		virtual void EndModal(int retCode);
};

/*============================ WxTabTabulator ===================*/

void WxTabTabulator::Draw(wxDC * dc, double scale)
{
	wxSize cs = dc->GetSize();

	// Draw grid
	dc->SetPen(wxPen(*wxRED, 0, wxPENSTYLE_SOLID));
	Grid::KnotsIterator kit;
	for(kit = grid.h_knots.begin(); kit != grid.h_knots.end(); kit++) {
		dc->DrawLine(kit->first * scale, 0, kit->first * scale, cs.GetHeight() * scale);
	}
	for(kit = grid.v_knots.begin(); kit != grid.v_knots.end(); kit++) {
		dc->DrawLine(0, kit->first * scale, cs.GetWidth() * scale, kit->first * scale);
	}
	if(grid.headers_end >= 0) {
		dc->SetPen(wxPen(*wxGREEN, 3, wxPENSTYLE_SHORT_DASH));
		dc->DrawLine(0, wxCoord(grid.headers_end) * scale, cs.GetWidth() * scale, wxCoord(grid.headers_end) * scale);
	}

	// Draw cells
}

void WxTabTabulator::ShowOptionsDialog(wxWindow * parent)
{
	WxTabOptionsDialog opts(parent, &this->options);
	opts.ShowModal();
}

/*============================ Tabulator options dialog =========*/

WxTabOptionsDialog::WxTabOptionsDialog( wxWindow * parent, struct Tabulator::Options * opts )
	: wxDialog( parent, wxID_ANY, _("Tabulator options"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),opts(opts)
{
	wxBoxSizer * s = new wxBoxSizer( wxVERTICAL /* wxHORIZONTAL */ );

	ui_find_table_header = new wxCheckBox(this, wxID_ANY, _T("Find table headers"));
	s->Add(ui_find_table_header);
	ui_find_more_rows_label = new wxStaticText(this, wxID_ANY, _T("Find more rows column"));
	s->Add(ui_find_more_rows_label);
	ui_find_more_rows_column = new wxSpinCtrl(this, wxID_ANY, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -1, 20);
	s->Add(ui_find_more_rows_column);
	ui_postprocess = new wxCheckBox(this, wxID_ANY, _T("Postprocess"));
	s->Add(ui_postprocess);
	ui_find_joined_cells = new wxCheckBox(this, wxID_ANY, _T("Find joined cells"));
	s->Add(ui_find_joined_cells);
	ui_split_column_label = new wxStaticText(this, wxID_ANY, _T("Split column"));
	s->Add(ui_split_column_label);
	ui_split_column = new wxSpinCtrl(this, wxID_ANY, _T("0"));
	s->Add(ui_split_column);

	ui_find_table_header->SetValidator(wxGenericValidator(&opts->find_table_header));
	ui_find_more_rows_column->SetValidator(wxGenericValidator((int*)&opts->find_more_rows_column));
	ui_postprocess->SetValidator(wxGenericValidator(&opts->postprocess));
	ui_find_joined_cells->SetValidator(wxGenericValidator(&opts->find_joined_cells));
	m_split_column_num = opts->split_columns.size() ? opts->split_columns.at(0) : 0;
	ui_split_column->SetValidator(wxGenericValidator(&m_split_column_num));

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
	delete ui_find_table_header;
	delete ui_split_column;
	delete ui_find_more_rows_label;
	delete ui_split_column_label;
}

bool WxTabOptionsDialog::Validate()
{
	return true;
}

void WxTabOptionsDialog::EndModal( int retCode )
{
	if(retCode == wxID_OK) {
		opts->split_columns.clear();
		if(m_split_column_num)
			opts->split_columns.push_back(m_split_column_num);
	}
	wxDialog::EndModal(retCode);
}

