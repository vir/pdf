#include "Tabulator_Exporter.hpp"

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
				cbuf[x++]=n&0x3F; n>>=6;
				header>>=1; header|=0x80;
			}

			s.push_back(n | header); x--;
			while(x>=0) { s.push_back(cbuf[x--]|0x80); }
		}
		else { s.push_back(n); }
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

void ExporterHTML::cell(const Tabulator::Table::Cell * cptr, unsigned int c, unsigned int r)
{
	if(!cptr) {
		s << "<td></td>";
		return;
	}
	s << (cptr->is_header?"<th>":"<td>");
	s << quote_html( ws2utf8( cptr->celltext() ) );
	s << (cptr->is_header?"</th>":"</td>");
}

void ExporterHTML::row_end()
{
	s << "</tr>" << std::endl;
}

void ExporterHTML::table_end()
{
	s << "</table>" << std::endl;
}

