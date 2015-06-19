#include "Tabulator_Exporter.hpp"
#include <string>

static std::string quote_csv(std::string s)
{
	unsigned int l=s.find_first_not_of(" ");
	if(l) s.erase(0, l);
	l=s.find_last_not_of(" ");
	if(l!=s.npos) s.resize(l+1);
	l=0;
	while((l=s.find("\"", l))!=s.npos) {
		s.insert(l, "\\");
		l+=2;
	}
	s.insert(0, "\"");
	s.append("\"");
	return s;
}

static void replace_in_place(std::string & s, std::string what, std::string with)
{
	size_t pos = 0;
	while((pos = s.find(what, pos)) != std::string::npos) {
		s.replace(pos, what.length(), with);
		pos += with.length();
	}
}

static std::string quote_html(std::string s)
{
	replace_in_place(s, "<", "&lt;");
	replace_in_place(s, ">", "&gt;");
	replace_in_place(s, "&", "&amp;");
	replace_in_place(s, "\"", "&quot;");
	unsigned int p;
#if 0
	p = s.find_first_not_of(" \t\r\n");
	if(p != std::string::npos)
		s.erase(0, p-1);
#endif
	p = s.find_last_not_of(" \t\r\n");
	if(p != std::string::npos)
		s.erase(p+1, s.length());
	return s;
}

static std::string ws2utf8(const std::wstring & ws)
{
	std::string s;
	for(unsigned int i=0; i<ws.length(); i++)
	{
		unsigned long n=ws[i];

		if(n & ~0x7F)
		{
			int x=0;
			unsigned char cbuf[6];
			unsigned char header=0x80;

			while(n & ~0x3F)
			{
				cbuf[x++]=static_cast<unsigned char>(n&0x3F); n>>=6;
				header>>=1; header|=0x80;
			}

			s+=char(n | header); x--;
			while(x>=0) { s+=char(cbuf[x--]|0x80); }
		}
		else { s+=char(n); }
	}
	return s;
}

void ExporterCSV::cell(std::wstring text, unsigned int c, unsigned int r)
{
	if(need_delimiter)
		s << delimiter;
	s << quote_csv( ws2utf8( text ) );
	need_delimiter = true;
}

void ExporterCSV::row_end()
{
	s << std::endl;
	need_delimiter = false;
}

/*= ExporterHTML =======================================================*/

void ExporterHTML::table_begin(unsigned int ncols, unsigned int nrows)
{
	s << "<table>" << std::endl;
}

void ExporterHTML::row_begin(unsigned int r)
{
	s << "<tr>";
}

void ExporterHTML::cell(const Tabulator::Table::Cell * cptr, bool hidden, unsigned int c, unsigned int r, unsigned int cs, unsigned int rs)
{
	if(hidden)
		return;
	if(!cptr) {
		s << "<td></td>";
		return;
	}
	std::string tag = (cptr->is_header)?"th":"td";
	s << '<' << tag;
	if(cs > 1) 
		s << " colspan=\"" << cs << "\"";
	if(rs > 1)
		s << " rowspan=\"" << rs << "\"";
	s << ">";
	s << quote_html( ws2utf8( cptr->celltext() ) );
	s << "</" << tag << '>';
}

void ExporterHTML::row_end()
{
	s << "</tr>" << std::endl;
}

void ExporterHTML::table_end()
{
	s << "</table>" << std::endl;
}

#ifdef _WIN32 /*======= Excel =================================================*/

bool ExporterExcel::set_params(std::string pstr)
{
	std::stringstream ss(pstr);
	ss >> sheets_number;
	std::clog << "ExporterExcel: sheets_number set to " << sheets_number << std::endl;
	return true;
}

void ExporterExcel::table_begin(unsigned int ncols, unsigned int nrows)
{
	cols_number = ncols;
	if(!cur_sheet || nrows > rows_number)
		rows_number = nrows;
}

void ExporterExcel::cell(const Tabulator::Table::Cell * cptr, bool hidden, unsigned int c, unsigned int r, unsigned int cs, unsigned int rs)
{
	if(!cptr || hidden)
		return;
	if(do_join_cells && (cs || rs))
			join_cells(cs, rs, c, r);
	set_cell_value(cptr->celltext(), c, r);
}

void ExporterExcel::table_end()
{
	cur_sheet++;
	if(!sheets_number || cur_sheet == sheets_number) { // move down and cur_sheet times left
		move_cursor(-(int)cur_col, rows_number);
		cur_col = 0;
		cur_sheet = 0;
	} else { // move right
		move_cursor(cols_number, 0);
		cur_col+=cols_number;
	}
}

#endif /* _WIN32 */


