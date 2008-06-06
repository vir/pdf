
// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include <fstream>
#include <iostream>
#include <string>
//#include <cstring>
#include <map>

#include "File.hpp"
#include "Exceptions.hpp"

#define GETLINE_ENDL '\r'

namespace PDF {
// PDF::File class members ==========================================================
// ============================================================================
File::File(std::string fn):m_debug(0)
{
  if(!fn.empty()) { open(fn); }
}

File::~File()
{
}

bool File::open(std::string fn)
{
  if(!fn.empty()) filename=fn;
  if(filename.empty()) throw std::string("No filename to open");
  file.open(filename.c_str(), std::ios::in);
  return file.good();
}

bool File::close()
{
  file.close();
  return true;
}

// load all indeexes and document directory
bool File::load()
{
  std::string ver=check_header();
  if(ver.length())
  {
    pdf_version=atof(ver.c_str());
    if(m_debug) std::clog << "PDF version: " << version() << "\n";
  }
  else throw FormatException("Invalid header", 0);

// load xref table
  long xref_off=get_first_xreftable_offset();
  while(1)
  {
    if(m_debug) std::clog << "Xref table is at " << xref_off << "\n";
    read_xref_table_part(xref_off);
    Dictionary * trailer=read_trailer();
//    trailer->dump();
    Object * o;

    if(( o=trailer->find("Root") ))
    {
      ObjRef * ref=dynamic_cast<ObjRef *>(o);
      if(!ref) throw FormatException("Root must be an Indirect Reference");
      root_refs.push_back(ref->ref());
    }

    if(!( o=trailer->find("Prev") )) { delete trailer; break; }
    
    Integer * i=dynamic_cast<Integer *>(o);
    if(!i) throw FormatException("Invalid 'Prev' type");
    xref_off=i->value();
    delete trailer; // no need it anymore
  }
  
  return true;
}






/// load object from file
Object * File::load_object(const ObjId & oi)
{
	if(m_debug>1) std::clog << "Loading object " << oi.dump() << std::endl;
  XrefTableType::const_iterator it=xref_table.find(oi);
  Object * r;
  if(it != xref_table.end())
  {
    file.seekg(it->second);
    r=Object::read_indirect(file);

		Stream * str;
		if((str = dynamic_cast<Stream *>(r))) { str->set_source(this); }
  }
  else r=new Null();
	if(m_debug>2) std::clog << "Loaded object: " << r->dump() << std::endl;
  return r;
}


//////////////////// helper functions /////////////////////////////////////

// Returns version or empty string if not a PDF
std::string File::check_header()
{
  if(!file.good()) { throw std::string("Bad file"); }
  std::string line, version;
  file.seekg(0, std::ios::beg);
	std::getline(file, line, GETLINE_ENDL);
  if(line.substr(0,5) == "%PDF-") { version=line.substr(5,3); }
  return version;
}

// Returns first (last in file) xref table offset
long File::get_first_xreftable_offset()
{
  const char * WHITESPACE=" \t\r\n\f"; // XXX
  const long to_read=256;
  char b[to_read+1]; b[to_read]='\0';
  file.seekg(-to_read, std::ios::end);
  file.read(b,to_read);
  char * p=::strstr(b, "startxref");
//  std::clog << "Xref string @ " << (int)p << std::endl;
  if(!p) return 0;
  p+=::strcspn(p, WHITESPACE);
  p+=::strspn(p, WHITESPACE);
  return atol(p);
}

void File::dump() const
{
  std::clog << "Xref table for " << filename << std::endl;
  for(XrefTableType::const_iterator it=xref_table.begin();!(it == xref_table.end()); ++it)
  {
    std::clog << "  object (" << it->first.num << "," << it->first.gen << ") is at " << it->second << std::endl;
  }
  for(unsigned int i = 0; i < root_refs.size(); i++)
  {
    std::clog << "Root Element: (" << root_refs[i].num << "," << root_refs[i].gen << ")" << std::endl;
  }
}

/// Read segment of "xref table"
/// \todo Read "unused" refs too?
bool File::read_xref_table_part(long off)
{
  std::string s;
  file.seekg(off, std::ios::beg);
	std::getline(file, s, GETLINE_ENDL); // get 'xref' header
//  std::clog << "Read first xref header: " << s << std::endl;
  if(s.find("xref")!=0) { std::cerr << "Error in xref table" << std::endl; return false; }
	std::getline(file, s, GETLINE_ENDL); // get numbers
//  std::clog << "Read first line of xref: " << s << std::endl;
  int sep=s.find_first_of(" \t");
  int objnum=atoi(s.substr(0,sep).c_str());
  sep=s.find_first_not_of(" \t", sep);
  int count=atoi(s.substr(sep).c_str());
//  std::clog << "First object in this table: " << objnum << ", number of objects: " << count << std::endl;
  for(;count>0;count--,objnum++)
  {
		std::getline(file, s, GETLINE_ENDL);
		s.erase(0, s.find_first_not_of("\r\n\t "));
    if(s.length() && s[17] == 'n') // read unly "used" refs
    {
      ObjId oi;
      oi.num=objnum;
      oi.gen=atol(s.substr(11,5).c_str());
      xref_table[oi]=atol(s.substr(0,10).c_str());
    }
  }

  return true;
}

Dictionary * File::read_trailer()
{
  std::string s;
	std::getline(file, s, GETLINE_ENDL);
	s.erase(0, s.find_first_not_of("\r\n\t "));
  if(s.substr(0,7) != "trailer") throw std::string("No trailer");

  Object * r=Object::read(file);
  Dictionary * dic=dynamic_cast<Dictionary *>(r);
  if(!dic) { delete r; throw std::string("Error in trailer dictionary"); }
  return dic;
}

long File::offset_lookup(const ObjId & oi) const
{
  XrefTableType::const_iterator it=xref_table.find(oi);
  return (it!=xref_table.end())?it->second:-1;
}


} // namespace PDF


