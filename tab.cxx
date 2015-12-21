#include "Tabulator.hpp"
#include "PDF.hpp"
#include <getopt.h>
#include <stdio.h>

static void do_it(const char * fname, unsigned int p_first = 0, unsigned int p_last = 0, int rotation = 0)
{
  PDF::Object::m_debug=false;
  
  PDF::File f;
  f.debug(1);
  f.open(fname, PDF::File::MODE_READ);
  if(!f.load()) { f.close(); return; }

	std::clog << "+ File " << fname << " loaded" << std::endl;
//  f.dump();

  PDF::Document doc(f);
	std::clog << "+ Document loaded" << std::endl;

//    PDF::Object::m_debug=true;
//    doc.dump();

	std::cout << "<html><body><table>" << std::endl;
	for(unsigned int pagenum = p_first; pagenum <= (p_last?p_last:doc.get_pages_count()); pagenum++) // all needed
	{
		PDF::Page * p=new PDF::Page();
		if(p)
		{
			std::clog << "Page " << pagenum << std::endl;
			p->load(doc.get_page_node(pagenum-1)); // 0-based

			Tabulator * mf=new Tabulator(rotation);
			std::clog << "Drawing page " << pagenum << std::endl;
			p->draw(mf);
			// lets see what we've got...
			mf->dump();
//			mf->chew(true); // do the final pretty structure construction!
			mf->chew(false); // do the final pretty structure construction!
			delete mf;
			delete p;
		}
	}
	std::cout << "</table></body></html>" << std::endl;

  f.close();
}

void help(std::ostream & s);

int main(int argc, char * argv[])
{
	int page_first = 0;
	int page_last = 0;
	int rot = 0;

	static struct option longopts[] = {
		{ "help", 0, 0, 'h' },
		{ "pages", 0, 0, 'p' },
		{ "rotate", 0, 0, 'r' },
		{ "crop", 0, 0, 'c' },
	};
	while(1)
	{
		int c, optindex;
		c = getopt_long(argc, argv, "hp:r:c:", longopts, &optindex);
		if(c == -1) break;
		switch(c)
		{
			case 'h':
				help(std::cout);
				exit(0);
				break;
			case 'p':
				if(2==sscanf(optarg, "%d-%d", &page_first, &page_last)) break;
				if(1==sscanf(optarg, "%d", &page_first)) { page_last = page_first; break; }
				std::cerr << "Error in page specification " << optarg << std::endl;
				help(std::cerr);
				exit(1);
			case 'r':
				if(1==sscanf(optarg, "%d", &rot)) break;
				std::cerr << "Error in rotation specification " << optarg << std::endl;
				help(std::cerr);
				exit(1);
			case 'c':
			default:
				std::cerr << "Unimplemented option " << c << std::endl;
				exit(-1);
		}
	}
#if 0
	int page_first = 40;
	int page_last  = 62;
	const char * fname = "/home/vir/in-sync/TP_VR_2008.pdf";
	int rot = 3;
#endif
#if 0
	int page_first = 13;
	int page_last  = 17;
	const char * fname = "/home/vir/in-sync/dium_200707_BG.pdf";
	int rot = 0;
#endif
	const char * fname = argv[optind++];

	if(!fname || !page_first || !page_last) {
		std::cerr << "PDF file name and page(s) to be processed must be specified!" << std::endl;
		help(std::cerr);
		exit(1);
	}

	std::clog << "File: " << fname << " Pages: " << page_first << '-' << page_last << " Rotation: " << rot << std::endl;

  std::set_terminate (__gnu_cxx::__verbose_terminate_handler);
  try {
    do_it(fname, page_first, page_last, rot);
  }
  catch(std::string s)
  {
		std::cerr << "!Exception: " << s << std::endl;
  }
  catch(PDF::DocumentStructureException e)
  {
		std::cerr << "DocumentStructureException:\n  " << e.what() << std::endl;
  }
  catch(PDF::FormatException e)
  {
		std::cerr << "Format excertion:\n  " << e.what() << std::endl;
  }
  catch(std::exception e)
  {
		std::cerr << "Unknown exception:\n  " << e.what() << std::endl;
  }
  catch(...)
  {
		std::cerr << "Unknown exception!" << std::endl;
  }
  return 0;
}

void help(std::ostream & s)
{
	s << "Usage: rot [options] file.pdf" << std::endl
	  << "Options:" << std::endl
	  << "\t-h or --help --- this help" << std::endl
	  << "\t-pN-M or -p NNN or --page NN-MM --- pages to process" << std::endl
	  << "\t-rR --- rotate page R*90degree CCW before processing" << std::endl
	  << "\t-cXXXX --- UNIMPLEMENTED crop rectangle" << std::endl;
}

