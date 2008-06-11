// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
# define __need_getopt
# define __GNU_LIBRARY__
#endif

#include "Tabulator.hpp"
#include "Tabulator_Exporter.hpp"
#include <libpdf/PDF.hpp>
#include <getopt.h>
#include <stdio.h>
#include <locale>
#include <iostream>

void help(std::ostream & s);

int main(int argc, char * argv[])
{
	int page_first = 0;
	int page_last = 0;
	char format = 'c';
	int tmp;

	Tabulator t;
	const char * expparams = NULL;
	// t.set_tolerance(5.0, 5.0);

#if 0
	std::locale loc("");
	std::cout << "Locale name: " << loc.name() << std::endl;
	std::wcout.imbue(loc);
	std::wcerr.imbue(loc);
	std::wclog.imbue(loc);
	std::wclog << L"log: Привет! Работает, гадина!" << std::endl;
	std::wcerr << L"err: Привет! Работает, гадина!" << std::endl;
	std::wcout << L"out: Привет! Работает, гадина!" << std::endl;
#endif
	while(1)
	{
		int c;
		c = getopt(argc, argv, "hp:r:c:R:f:PE:");
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
				if(1==sscanf(optarg, "%d", &tmp)) {
					t.metafile.set_rotation(tmp);
					break;
				}
				std::cerr << "Error in rotation specification " << optarg << std::endl;
				help(std::cerr);
				exit(1);
			case 'R':
				t.options.find_more_rows = true;
				t.options.find_rows_column = atoi(optarg);
				break;
			case 'f':
				format = *optarg;
				break;
			case 'P':
				t.options.postprocess = true;
				break;
			case 'E':
				expparams = optarg;
				break;
			case 'c':
			default:
				std::cerr << "Unimplemented option " << c << std::endl;
				exit(-1);
		}
	}
	const char * fname = argv[optind++];

	if(!fname || !page_first || !page_last) {
		std::cerr << "PDF file name and page(s) to be processed must be specified!" << std::endl;
		help(std::cerr);
		exit(1);
	}

	std::clog << "File: " << fname << " Pages: " << page_first << '-' << page_last << std::endl;

#ifndef _MSC_VER
  std::set_terminate (__gnu_cxx::__verbose_terminate_handler);
#endif
  try {
	PDF::Object::m_debug=false;
	PDF::File f;
	f.debug(0);
	f.open(fname);
	if(!f.load()) { f.close(); return -1; }
	
	std::clog << "+ File " << fname << " loaded" << std::endl;
	
	PDF::Document doc(f);
	std::clog << "+ Document loaded" << std::endl;
	
	// PDF::Object::m_debug=true;
	// doc.dump();

	Tabulator::Table::Exporter * exporter = NULL;
	switch(format) {
		case 'c': case 'C':
			exporter = new ExporterCSV(std::cout);
			break;
		case 'h': case 'H':
			exporter = new ExporterHTML(std::cout);
			break;
#ifdef _WIN32
		case 'e': case 'E':
			exporter = new ExporterExcel();
			if(!static_cast<ExporterExcel*>(exporter)->get_active()) {
				static_cast<ExporterExcel*>(exporter)->start_new();
				static_cast<ExporterExcel*>(exporter)->add_workbook();
			}
			static_cast<ExporterExcel*>(exporter)->set_visible(true);

			break;
#endif
		default:
			std::cerr << "Unknown format <" << format << ">" << std::endl;
			return -1;
	}
	if(expparams) {
			if(!exporter->set_params(expparams)) {
				std::cerr << "Error setting exporter params" << std::endl;
				return -1;
			}
	}

	for(unsigned int pagenum = page_first; pagenum <= (page_last?page_last:doc.get_pages_count()); pagenum++) // all needed
	{
		PDF::Page * p=new PDF::Page();
		if(p)
		{
			exporter->page_begin(fname, pagenum);
			std::clog << "Page " << pagenum << std::endl;
			p->load(doc.get_page_node(pagenum-1)); // 0-based

			std::clog << "Drawing page " << pagenum << std::endl;
			t.full_process(p); // do the final pretty structure construction!
			t.output(exporter);
			exporter->page_end();
			delete p;
		}
	}
	delete exporter;

	f.close();
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
	  << "\t-cXXXX --- UNIMPLEMENTED crop rectangle" << std::endl
		<< "\t-R COL --- add rows by looking at text in col COL" << std::endl
		<< "\t-f F --- set output format to F: c h e" << std::endl
		<< "\t-P --- postprocess table - use if got many W O R D S W I T H S P A C E S" << std::endl
		<< "\t-E PARAMS --- set exporter-specific params" << std::endl
		;
}

