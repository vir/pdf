#include <iostream>
#include <string>

#include "PDF.hpp"
#include "utf8.hpp"

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

    Tabulator():pheight(0) {}
    virtual ~Tabulator() { };
    virtual Point Size(Point unity)
    {
      clog << "SetPageSize(" << unity.dump() << endl;
      pheight=unity.y; return unity;
    }
    virtual void Text(Point pos, const PDF::Font * font, std::wstring text);
    virtual void Line(const Point & p1, const Point & p2);
    void dump() const;
    void chew();
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

void Tabulator::chew()
{
  clog << "Chewing..." << endl;
  map<Coord, vector<PLine> > h_knots, v_knots;
  vector<PLine>::iterator it;
  
  // get an array of y coords of horizontal lines
  for(it=h_lines.begin(); it!=h_lines.end(); it++) { v_knots[it->p1.y].push_back(*it); }
  // get an array of x coords of vertical lines
  for(it=v_lines.begin(); it!=v_lines.end(); it++) { h_knots[it->p1.x].push_back(*it); }
  clog << "Found " << v_knots.size() << " vertical and " << h_knots.size() << " horizontal knots" << endl;

  /// \todo found joined cells

  map<Point, string>::iterator tit=all_text.begin(); // text iterator
  map<Coord, vector<PLine> >::iterator rit; // table rows iterator
  for(rit=v_knots.begin(); rit!=v_knots.end(); rit++)
  {
    vector< TextLines* > row; row.resize(h_knots.size()-1);
    for(unsigned int i=0; i<h_knots.size()-1; i++) row[i]=new TextLines;
    
    //while(tit!=all_text.end() && tit->first.y < reinterpret_cast<double>(rit->first))
    while(tit!=all_text.end() && tit->first.y < double(rit->first))
    {
//      cout << "[" << tit->second << "]" << endl;
      map<Coord, vector<PLine> >::iterator xit=h_knots.begin();
      unsigned int col;
      for(xit++, col=0; xit!=h_knots.end(); xit++, col++)
      {
        TextLines * celltext=row[col];
        if(tit->first.x < double(xit->first))
        {
          (*celltext)[tit->first]=tit->second;
          break;
        }
      }
      tit++;
    }
    
    cout << "<tr>";
//    cout << "<!-- row size=" << row.size() << " -->";
    for(unsigned int col=0; col<row.size(); col++)
    {
      cout << "<td>";
      TextLines * textpart=row[col];
//      cout << "<!-- column=" << col << "-->";
      string celltext;
      for(TextLines::iterator tpit=textpart->begin(); tpit!=textpart->end(); tpit++)
      {
        // remove hyphenation XXX \todo mess with unicode hypheation character (if any?)
        if(!celltext.empty() && celltext[celltext.size()-1]=='-') celltext.resize(celltext.size()-1);
        celltext+=tpit->second;
      }
      cout << celltext << "</td>";
    }
    cout << "</tr>" << endl;
    
//    cout << "-----------------------------------------------" << endl;
    for(unsigned int i=0; i<h_knots.size()-1; i++) delete row[i];
  }
}

void Tabulator::Text(Point pos, const PDF::Font *, std::wstring text)
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
//      clog << "V-Line" << p1.dump() << "-" << p2.dump() << endl;
    }
  }
  else if(compare(p1.y, p2.y) == 0)
  {
    PLine l(p1, p2); if(p2.x < p1.x) l=PLine(p2, p1);
    h_lines.push_back(l);
//    clog << "H-Line" << p1.dump() << "-" << p2.dump() << endl;
  }
}

void convert_page(PDF::Document & doc, unsigned int pagenum)
{
  PDF::Page * p=new PDF::Page();
  if(p)
  {
    clog << "Page " << pagenum << endl;
		PDF::OH pn = doc.get_page_node(pagenum);
		cout << "<!-- Page " << pagenum << " - " << pn->dump() << " -->" << endl;
    p->load(pn);
//      std::cout << p->dump() << std::endl;

    Tabulator * mf=new Tabulator();
		clog << "Drqwing page " << pagenum << endl;
    p->draw(mf);
    // lets see what we've got...
    mf->dump();
    mf->chew(); // do the final pretty structure construction!
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

	cout << "<table>" << endl;
  for(unsigned int page=1; page < doc.get_pages_count(); page++) // all needed
//  for(unsigned int page=1; page < 10; page++)
//  for(unsigned int page=10; page < 83; page++)
  {
    convert_page(doc, page);
  }
	cout << "</table>" << endl;

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



