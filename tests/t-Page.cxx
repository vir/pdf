#include "../PDF.hpp"
#include <iostream>
#include <fstream>
#include <vector>

int main()
{
	try {
		std::set_terminate (__gnu_cxx::__verbose_terminate_handler);
		std::fstream f;
		f.open("page1.dump", std::ios::in);
		std::vector<char> pt((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>()); // read whole file into string var

		PDF::Page p;
		p.parse(pt);
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
		std::cerr << "Format exception:\n  " << e.what() << std::endl;
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




