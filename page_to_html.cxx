/*
 * Dump contents of a given page number of a given pdf file
 * as a plain text metafile pagedump.txt
 *
 */
#include <iostream>
#include <iomanip> /* for setiosflags and so */
#include <string>

#include "PDF.hpp"
#include "utf8.hpp"

/*
 * modified page_metafile.cxx to produce css-positioned html elements
 *
 */
using namespace std;

class Metafile:public PDF::Media
{
  private:
		PDF::CTM m;
		PDF::Font * curfont;
    ofstream s;
		double abs(double x) { return x<0?-x:x; }
  public:
    typedef PDF::Point Point;
    Metafile(string fn=""):curfont(NULL)
    {
      if(fn.length()) s.open(fn.c_str(), ios::out);
      else            s.open("/dev/stderr", ios::out);
			s << "<html><head><style type=\"text/css\">" << endl;
			s << "div { display:block; position:absolute; }" << endl;
			s << "div.line { background-color:black; }" << endl;
			s << "</style></head><body>" << endl;
			s << setiosflags(ios::fixed) << setprecision(0);
    }
    virtual ~Metafile() { s << "</body></html>" << endl; s.close(); };
    virtual void Text(Point pos, std::wstring text);
    virtual void Line(const Point & p1, const Point & p2);
    virtual void Size(Point size)
		{
			m.set_unity();
#if 0
			/* Rotate landscape page 90deg CW */
			m.rotate(90.0);
			m.scale(-1, 1);
//			m.offset(size.y, 0);
#elif 1
			/* Normal page layout, invert coordinates for html's upsidedown y axis */
			m.scale(1, -1);
			m.offset(0, size.y);
#else
			/* Upsidedown html */
#endif
			m.dump();
		}
		virtual const PDF::CTM & Matrix() { return m; }
};

void Metafile::Text(Point pos, std::wstring text)
{
  s << "<div style=\"left:" << pos.x << "px; top:" << pos.y << "px;\" class=\"" << (curfont?curfont->name():"[default]") << "\">" << ws2utf8(text) << "</div>" << endl;
}

void Metafile::Line(const Point & p1, const Point & p2)
{
	const double prc = 2.0;
	double l = p1.x<p2.x?p1.x:p2.x;
	double t = p1.y<p2.y?p1.y:p2.y;
	double w = abs(p1.x - p2.x);
	double h = abs(p1.y - p2.y);
	if(h < prc || w < prc) {
		s << "<div class=\"line\" style=\"left:" << l << "px; top:" << t << "px; width:" << (w?w:1.0) << "px; height:" << (h?h:1.0) << "px;\"></div>" << endl;
	} else {
		s << "<!-- Line" << p1.dump() << "-" << p2.dump() << " -->"<< endl;
	}
}

static void do_it(const char * fname, int pagenum)
{
  PDF::Object::m_debug=0;
  
  PDF::File f;
	f.debug(0);
  f.open(fname);
  if(!f.load()) { f.close(); return; }

  clog << "+ File loaded" << endl;

//  f.dump();

  {
    PDF::Document doc(f, 5);

    clog << "+ Document loaded" << endl;
  
//    PDF::Object::m_debug=true;

//    doc.dump();

    PDF::Page * p=new PDF::Page();
    if(p)
    {
			p->debug(5);
      cout << pagenum << "th page:" << endl;
      p->load(doc.get_page_node(pagenum-1));
      std::cout << p->dump() << std::endl;

      // lets see what we've got...
//      Metafile * mf=new Metafile();
      Metafile * mf=new Metafile("pagedump.html");
      p->draw(mf);
      delete mf;

      delete p;
    }

  }

  f.close();
}

int main(int argc, char * argv[])
{
  if(argc<3 || atoi(argv[2])<=0)
  {
    cerr << "Usage: " << argv[0] << " filename.pdf PAGENUM" << endl;
    exit(1);
  }
  try {
    do_it(argv[1], atoi(argv[2]));
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



