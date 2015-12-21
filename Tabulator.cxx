#include <iostream>
#include <string>

#include "PDF.hpp"
#include "Tabulator.hpp"
#include "utf8.hpp"

/* parser of tables such as
 *  .-----------------.
 *  | h1  |  h2  | h3 |
 *  `-----------------'
 *   d11   d12    d13
 *   d21   d22    d23
 *   ...
 * (updated for gng-2007 parsing)
 */

using namespace std;


void Tabulator::Size(Point size)
{
	if(rotation % 2) {
		pheight = size.x;
//		myarea = PDF::Rect(0, 0, size.y, size.x);
		myarea = PDF::Rect(35, 0, size.y, size.x);
	} else {
		pheight = size.y;
		myarea = PDF::Rect(0, 0, size.x, size.y);
	}
	matrix.set_unity();
	switch(rotation) {
#if 0
		case 0: break;
		case 1: matrix.rotate( 90.0); matrix.offset(size.y, 0);      break;
		case 2: matrix.rotate(180.0); matrix.offset(size.x, size.y); break;
		case 3: matrix.rotate(270.0); matrix.offset(0, size.x);      break;
#else
		case 0:                       matrix.scale(1, -1); matrix.offset(0, size.y); break;
		case 1: matrix.rotate( 90.0); matrix.scale(1, -1); matrix.offset(size.y, size.x); break;
		case 2: matrix.rotate(180.0); matrix.scale(1, -1); matrix.offset(size.x, size.y); break;
		case 3: matrix.rotate(270.0); matrix.scale(1, -1); break;
#endif
		default: break;
	}
}

class Tabulator::Coord
{
  private:
    double v;
  public:
    Coord(double d):v(d) {}
    bool operator < (const Coord & c) const { return compare(v, c.v) < 0; }
    bool operator == (const Coord & c) const { return compare(v, c.v) == 0; }
    operator double() const { return v; }
		inline static int compare(double d1, double d2, double delta=3)
		{
			if(d1+delta < d2) return -1;
			if(d1-delta > d2) return +1;
			return 0;
		}
};

class Tabulator::Cell
{
	private:
		map<PDF::Point, string> text;
	public:
		bool is_header;
		Cell():is_header(false) {}
		void addtext(const PDF::Point & p, const string & s) { text[p]=s; }
		string celltext()
		{
      string r;
			map<PDF::Point, string>::const_iterator tpit;
      for(tpit=text.begin(); tpit!=text.end(); tpit++)
      {
        /// remove hyphenation XXX \todo mess with unicode hypheation character (if any?)
        if(!r.empty() && r[r.size()-1]=='-') r.resize(r.size()-1);
        r+=tpit->second;
      }
			return r;
		}
		string html()
		{
			if(!is_header) {
				return string("<td>")+celltext()+"</td>";
			} else {
				return string("<th>")+celltext()+"</th>";
			}
		}
};

void Tabulator::dump() const
{
  clog << "Tabulator dump:" << endl;
  clog << "\t" << "Page height: " << pheight << endl;
  clog << "\t" << h_lines.size() << " horizontal lines" << endl;
  clog << "\t" << v_lines.size() << " vertical lines" << endl;
  clog << "\t" << all_text.size() << " text chunks" << endl;
}

static string optional_quote(string s)
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

void Tabulator::chew(bool skip_headers)
{
  clog << "Chewing..." << endl;
  map<Coord, vector<PLine> > h_knots, v_knots;
	map<Coord, vector<PLine> >::iterator xit;
  vector<PLine>::iterator it;
	double header_y=-1E50;
  
  // get an array of x coords of vertical lines
  for(it=v_lines.begin(); it!=v_lines.end(); it++) { h_knots[it->p1.x].push_back(*it); }
	for(xit=h_knots.begin(); xit!=h_knots.end(); xit++) {
		clog << "V-line at " << xit->first << endl;
	}
  // get an array of y coords of dhorizontal lines
  for(it=h_lines.begin(); it!=h_lines.end(); it++) {
		v_knots[it->p1.y].push_back(*it);
		if(header_y<it->p1.y) { clog << "Set header_y to " << it->p1.y << endl; header_y=it->p1.y; }
	}
	/* 
	 * We have no horizontal lines, so
	 * let's assume lines some little space (eg. 4 units) above text, that falls
	 * into first column
	 */
	const double small_offset=4;
	double cur_y;
	if(h_knots.size() >= 2) {
		xit=h_knots.begin(); xit++; /* pointer to second verical line */
		clog << "Second vertical line is at x " << xit->first << endl;
		map<Point, string>::iterator tit=all_text.begin(); // text iterator
		while(tit!=all_text.end()) { /* check all text */
			if(tit->first.x < double(xit->first)) {
				clog << "Adding line above text string @" << tit->first.dump() << "(" << tit->second << ")" << endl;
				cur_y = tit->first.y-small_offset;
				v_knots[cur_y].push_back(PLine(Point(tit->first.x, tit->first.y-small_offset), Point(tit->first.x+500, tit->first.y-small_offset)));
			}
			tit++;
		}
	}
	cur_y+=10.0;
v_knots[cur_y].push_back(PLine(Point(0, cur_y), Point(500, cur_y)));
  clog << "Found " << v_knots.size() << " vertical and " << h_knots.size() << " horizontal knots" << endl;

  /// \todo found joined cells

  map<Point, string>::iterator tit=all_text.begin(); // text iterator
  map<Coord, vector<PLine> >::iterator rit; // table rows iterator
  for(rit=v_knots.begin(); rit!=v_knots.end(); rit++)
  {
    vector< Cell > row;
		row.resize(h_knots.size()-1);
    
    while(tit!=all_text.end() && tit->first.y < double(rit->first))
    {
      xit=h_knots.begin();
      unsigned int col;
      for(xit++, col=0; xit!=h_knots.end(); xit++, col++)
      {
        if(tit->first.x < double(xit->first))
        {
//					cout << "<!-- text @ " << tit->first.dump() << " (" << tit->second << ") falls before V-line @ " << double(xit->first) << " -->" << endl;
					row[col].addtext(tit->first, tit->second);
					if(tit->first.y<header_y) row[col].is_header=true;
          break;
        }
      }
      tit++;
    }
    
		if( !(skip_headers && (row[0].is_header || row[0].celltext().length()==0)) ) {
			if(!output_csv) cout << "<tr>";
	//    cout << "<!-- row size=" << row.size() << " -->";
			for(unsigned int col=0; col<row.size(); col++)
			{
	//      cout << "<!-- column=" << col << "-->";
				if(output_csv) { cout << optional_quote(row[col].celltext()); if(col<row.size()-1) cout << ";"; }
				else cout << row[col].html();
			}
			if(output_csv) cout << endl;
			else cout << "</tr>" << endl;
		}
  }
}

void Tabulator::Text(PDF::Rect posx, double angle, std::wstring text, bool visible, const PDF::GraphicsState& gs)
{
	PDF::Point pos = posx.offset();
	if(!myarea.in(pos)) {
		std::clog << "Skipping text <<" << ws2utf8(text) << ">> - not in my area" << std::endl;
		return;
	}

	//pos.y=pheight-pos.y;// upsidedown
//	clog << "Text" << pos.dump() << "[" << ws2utf8(text) << "]" << endl;
	all_text[pos]=ws2utf8(text);
}

/**
 * Sort lines to horizontal/vertical/short, store them in h_lines and v_lines
 * and make sure that p1.[xy] < p2.[xy]
 */
void Tabulator::Line(const PDF::Point & xp1, const PDF::Point & xp2, const PDF::GraphicsState& gs)
{
	if(!myarea.in(xp1) || !myarea.in(xp2)) {
		std::clog << "Skipping line " << xp1.dump() << "-" << xp2.dump() << ">> - not in my area" << std::endl;
		return;
	}

  Point p1=xp1; Point p2=xp2;
//  p1.y=pheight-p1.y; p2.y=pheight-p2.y; // upsidedown
  
  if(Coord::compare(p1.x, p2.x) == 0)
  {
    if(Coord::compare(p1.y, p2.y) == 0) { /* skip too short line segments */ }
    else
    {
      PLine l(p1, p2); if(p2.y < p1.y) l=PLine(p2, p1);
      v_lines.push_back(l);
      clog << "V-Line" << p1.dump() << "-" << p2.dump() << endl;
    }
  }
  else if(Coord::compare(p1.y, p2.y) == 0)
  {
    PLine l(p1, p2); if(p2.x < p1.x) l=PLine(p2, p1);
    h_lines.push_back(l);
    clog << "H-Line" << p1.dump() << "-" << p2.dump() << endl;
  }
}

