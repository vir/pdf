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
    ofstream s;
  public:
    typedef PDF::Point Point;
    Metafile(string fn="")
    {
      if(fn.length()) s.open(fn.c_str(), ios::out);
      else            s.open("/dev/stderr", ios::out);
    }
    virtual ~Metafile() { };
    virtual Point Size(Point unity) { return unity; }
    virtual void Text(Point pos, const PDF::Font * font, std::wstring text);
    virtual void Line(const Point & p1, const Point & p2);
};

void Metafile::Text(Point pos, const PDF::Font * font, std::wstring text)
{
  s << "Text" << pos.dump() << "(" << font->name() << ")[" << ws2utf8(text) << "]" << endl;
}

void Metafile::Line(const Point & p1, const Point & p2)
{
  s << "Line" << p1.dump() << "-" << p2.dump() << endl;
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
      cout << pagenum << "th page:" << endl;
      p->load(doc.get_page_node(pagenum));
      std::cout << p->dump() << std::endl;

      // lets see what we've got...
//      Metafile * mf=new Metafile();
      Metafile * mf=new Metafile("pagedump.txt");
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



