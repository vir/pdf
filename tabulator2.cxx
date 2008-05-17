#include <iostream>
#include <string>

#include "PDF.hpp"
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

inline int compare(double d1, double d2, double delta=3)
{
  if(d1+delta < d2) return -1;
  if(d1-delta > d2) return +1;
  return 0;
}

class Tabulator:public PDF::Media
{
  private:
    double pheight;
  public:
		bool output_csv;
    typedef PDF::Point Point;
    class PLine // we already have "Line" :(
    {
      public:
        Point p1, p2;
        PLine(Point z1, Point z2):p1(z1),p2(z2) {}
    };
    vector<PLine> h_lines, v_lines;
    typedef map<Point, string> TextLines;
    TextLines all_text;

    Tabulator():pheight(0),output_csv(true) {  }
    virtual ~Tabulator()   {  };
    virtual void Size(Point unity)
    {
      clog << "SetPageSize(" << unity.dump() << endl;
      pheight=unity.y;
    }
    virtual void Text(Point pos, std::wstring text);
    virtual void Line(const Point & p1, const Point & p2);
    void dump() const;
    void chew(bool skip_headers=false);
};

void Tabulator::dump() const
{
  clog << "Tabulator dump:" << endl;
  clog << "\t" << "Page height: " << pheight << endl;
  clog << "\t" << h_lines.size() << " horizontal lines" << endl;
  clog << "\t" << v_lines.size() << " vertical lines" << endl;
  clog << "\t" << all_text.size() << " text chunks" << endl;
}

class Coord
{
  private:
    double v;
  public:
    Coord(double d):v(d) {}
    bool operator < (const Coord & c) const { return compare(v, c.v) < 0; }
    bool operator == (const Coord & c) const { return compare(v, c.v) == 0; }
    operator double() const { return v; }
};

class Cell
{
	private:
		map<PDF::Point, string> t;
	public:
		bool is_header;
		Cell():is_header(false) {}
		void addtext(const PDF::Point & p, const string & s) { t[p]=s; }
		string celltext()
		{
      string r;
			map<PDF::Point, string>::const_iterator tpit;
      for(tpit=t.begin(); tpit!=t.end(); tpit++)
      {
        // remove hyphenation XXX \todo mess with unicode hypheation character (if any?)
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
	if(h_knots.size() >= 2) {
		xit=h_knots.begin(); xit++; /* pointer to second verical line */
		clog << "Second vertical line is at x " << xit->first << endl;
		map<Point, string>::iterator tit=all_text.begin(); // text iterator
		while(tit!=all_text.end()) { /* check all text */
			if(tit->first.x < double(xit->first)) {
				clog << "Adding line above text string @" << tit->first.dump() << "(" << tit->second << ")" << endl;
				v_knots[tit->first.y-small_offset].push_back(PLine(Point(tit->first.x, tit->first.y-small_offset), Point(tit->first.x+500, tit->first.y-small_offset)));
			}
			tit++;
		}
	}
  clog << "Found " << v_knots.size() << " vertical and " << h_knots.size() << " horizontal knots" << endl;

  /// \todo found joined cells

  map<Point, string>::iterator tit=all_text.begin(); // text iterator
  map<Coord, vector<PLine> >::iterator rit; // table rows iterator
  for(rit=v_knots.begin(); rit!=v_knots.end(); rit++)
  {
    vector< Cell > row; row.resize(h_knots.size()-1);
    
    //while(tit!=all_text.end() && tit->first.y < reinterpret_cast<double>(rit->first))
    while(tit!=all_text.end() && tit->first.y < double(rit->first))
    {
//      cout << "[" << tit->second << "]" << endl;
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

void Tabulator::Text(Point pos, std::wstring text)
{
  pos.y=pheight-pos.y;// upsidedown
//  clog << "Text" << pos.dump() << "[" << ws2utf8(text) << "]" << endl;
  all_text[pos]=ws2utf8(text);
}

/**
 * Sort lines to horizontal/vertical/short, store them in h_lines and v_lines
 * and make sure that p1.[xy] < p2.[xy]
 */
void Tabulator::Line(const Point & xp1, const Point & xp2)
{
  Point p1=xp1; Point p2=xp2;
  p1.y=pheight-p1.y; p2.y=pheight-p2.y; // upsidedown
  
  if(compare(p1.x, p2.x) == 0)
  {
    if(compare(p1.y, p2.y) == 0) { /* skip too short line segments */ }
    else
    {
      PLine l(p1, p2); if(p2.y < p1.y) l=PLine(p2, p1);
      v_lines.push_back(l);
      clog << "V-Line" << p1.dump() << "-" << p2.dump() << endl;
    }
  }
  else if(compare(p1.y, p2.y) == 0)
  {
    PLine l(p1, p2); if(p2.x < p1.x) l=PLine(p2, p1);
    h_lines.push_back(l);
    clog << "H-Line" << p1.dump() << "-" << p2.dump() << endl;
  }
}

void convert_page(PDF::Document & doc, unsigned int pagenum)
{
  PDF::Page * p=new PDF::Page();
  if(p)
  {
    clog << "Page " << pagenum << endl;
    p->load(doc.get_page_node(pagenum));
//      std::cout << p->dump() << std::endl;

    Tabulator * mf=new Tabulator();
		clog << "Drqwing page " << pagenum << endl;
    p->draw(mf);
    // lets see what we've got...
    mf->dump();
    mf->chew(true); // do the final pretty structure construction!
    delete mf;

    delete p;
  }

}

static void do_it(const char * fname)
{
  PDF::Object::m_debug=false;
//  PDF::Object::m_debug=true;
  
  PDF::File f;
  f.debug(1);
  f.open(fname);
  if(!f.load()) { f.close(); return; }

  clog << "+ File " << fname << " loaded" << endl;
//  f.dump();

/*
  PDF::ObjectsCache * oc=new PDF::ObjectsCache(f);
  clog << "+ Objects cache created" << endl;
  
  PDF::Document doc(oc, f.get_root(), 10);
*/
  PDF::Document doc(f);
  clog << "+ Document loaded" << endl;

//    PDF::Object::m_debug=true;
//    doc.dump();

	cout << "<html><body><table>" << endl;
	for(unsigned int page=0; page < doc.get_pages_count(); page++) // all needed
//	for(unsigned int page=0; page < 2; page++)
	{
		convert_page(doc, page);
	}
	cout << "</table></body></html>" << endl;

//  delete oc;

  f.close();
}

int main(int argc, char * argv[])
{
  std::set_terminate (__gnu_cxx::__verbose_terminate_handler);
  try {
    do_it((argc>1)?argv[1]:"gng.pdf");
  }
  catch(string s)
  {
    cerr << "!Exception: " << s << endl;
  }
  catch(PDF::DocumentStructureException e)
  {
    cerr << "DocumentStructureException:\n  " << e.what() << endl;
  }
  catch(PDF::FormatException e)
  {
    cerr << "Format excertion:\n  " << e.what() << endl;
  }
  catch(exception e)
  {
    cerr << "Unknown exception:\n  " << e.what() << endl;
  }
  catch(...)
  {
    cerr << "Unknown exception!" << endl;
  }
  return 0;
}



