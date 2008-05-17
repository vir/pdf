/*
 * Dump contents of a given page number of a given pdf file
 * as a plain text metafile pagedump.txt
 *
 */
#include <iostream>
#include <string>

#include "PDF.hpp"
#include "utf8.hpp"

using namespace std;

class Metafile:public PDF::Media
{
  private:
    ostream & s;
  public:
    typedef PDF::Point Point;
    Metafile(ostream & out):s(out) { }
    virtual ~Metafile() { };
		virtual void SetFont(const PDF::Font * font, double size);
    virtual void Text(Point pos, std::wstring text);
    virtual void Line(const Point & p1, const Point & p2);
		virtual void Debug(string s);
#if 1
		PDF::CTM m;
    virtual void Size(Point size)
		{
			m.set_unity();
# if 1
			/* Rotate landscape page 90deg CW */
			m.rotate(90.0);
			m.scale(-1, 1);
//			m.offset(size.y, 0);
# elif 0
			/* Normal page layout, invert coordinates for html's upsidedown y axis */
			m.scale(1, -1);
			m.offset(0, size.y);
# else
			/* Upsidedown html */
# endif
			m.dump();
		}
		virtual const PDF::CTM & Matrix() { return m; }
#endif
};

void Metafile::SetFont(const PDF::Font * font, double size)
{
	s << "SetFone(" << font->name() << ", " << size << ")" << endl;
}

void Metafile::Text(Point pos, std::wstring text)
{
  s << "Text" << pos.dump() << "[" << ws2utf8(text) << "]" << endl;
}

void Metafile::Line(const Point & p1, const Point & p2)
{
  s << "Line" << p1.dump() << "-" << p2.dump() << endl;
}

void Metafile::Debug(string msg)
{
	s << "Debug[ " << msg << " ]" << endl;
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
      cerr << "Loading " << pagenum << "th page:" << endl;
      p->load(doc.get_page_node(pagenum-1));
      std::cerr << p->dump() << std::endl;

      // lets see what we've got...
/*
			ofstream s;
      if(fn.length()) s.open(fn.c_str(), ios::out);
      else            s.open("/dev/stderr", ios::out);
*/
      Metafile * mf=new Metafile(cout);
			clog << "Drawing page" << endl;
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



