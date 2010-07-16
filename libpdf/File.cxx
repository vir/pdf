
// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include <fstream>
#include <iostream>
#include <string>
//#include <cstring>
#include <map>
#include <list>

#include <stdlib.h> // for atof
#include <string.h> // for strstr, strspn

#include "File.hpp"

#define GETLINE_ENDL '\n'

namespace PDF {
// PDF::File class members ==========================================================
// ============================================================================
File::File(std::string fn)
	:strm(file)
	,m_debug(0)
	,m_security(NULL)
{
	if(!fn.empty())
		open(fn);
}

File::~File()
{
	delete m_security;
}

bool File::open(std::string fn)
{
	if(!fn.empty())
		filename = fn;
	if(filename.empty())
		throw std::exception("No filename to open");
	file.open(filename.c_str(), std::ios::in|std::ios::binary);
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
	if(ver.length()) {
		pdf_version = (float)::atof(ver.c_str());
		if(m_debug) std::clog << "PDF version: " << version() << "\n";
	} else
		throw FormatException("Invalid header", 0);

	// load xref table
	long xref_off = get_first_xreftable_offset();
	while(xref_off)
	{
		Object * o;
		Dictionary * trailer = NULL;
		Object * victim = NULL; // Will be delete()d after segment is read
		file.seekg(xref_off);
		if(file.peek() == 'x') { // Normal xref table
			if(m_debug)
				std::clog << "Xref table is at " << xref_off << "\n";
			read_xref_table_part(xref_off);
			if(m_debug)
				std::clog << "Reading trailer at " << file.tellg() << "\n";
			victim = trailer = read_trailer();
		} else {
			o = strm.read_indirect_object(false);
			Stream * s = dynamic_cast<Stream *>(o);
			if(!s)
				throw FormatException("Error reading xsref stream", xref_off);
			read_xref_stream(s);
			trailer = s->get_dict();
			victim = s;
		}
		//    trailer->dump();

		if(( o=trailer->find("Root") ))
		{
			ObjRef * ref = dynamic_cast<ObjRef *>(o);
			if(!ref)
				throw FormatException("Root must be an Indirect Reference", xref_off);
			root_refs.push_back(ref->ref());
		}

		if(( o = trailer->find("ID") )) {
			Array * a = dynamic_cast<Array *>(o);
			if(!a)
				throw FormatException("'ID' trailer entry must be an array of strings");
			load_file_ids(a);
		}

		if(( o = trailer->find("Encrypt") )) {
			bool need_destroy = false;
			Dictionary * d = dynamic_cast<Dictionary *>(o);
			if(!d) { // may be it is a ObjRef
				ObjRef * ref = dynamic_cast<ObjRef *>(o);
				if(ref) {
					o = load_object(ref->ref(), false);
					d = dynamic_cast<Dictionary *>(o);
					need_destroy = true;
				}
			}
			if(!d)
				throw FormatException("Can not find 'Encrypt' dictionary");

			delete m_security;
			m_security = SecHandler::create(d, this);
			strm.set_security_handler(m_security);
			if(need_destroy)
				delete d;
		}

		if(( o = trailer->find("Prev") )) {
			Integer * i = dynamic_cast<Integer *>(o);
			if(!i)
				throw FormatException("Invalid 'Prev' type in xref table trailer");
			xref_off = i->value();
		} else
			xref_off = 0; // break out of the loop
		delete victim;
	} // for each xref table segment
	return true;
}






/// load object from file
Object * File::load_object(const ObjId & oi, bool decrypt)
{
	if(m_debug>1) std::clog << "Loading object " << oi.dump() << std::endl;
	long offset = xref_table.get_offset(oi);
	Object * r;
	if(offset) {
		file.seekg(offset);
		r = strm.read_indirect_object(decrypt);

		Stream * str;
		if((str = dynamic_cast<Stream *>(r)))
			str->set_source(this);
	} else
		r = new Null();
	if(m_debug > 2)
		std::clog << "Loaded object: " << r->dump() << std::endl;
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
	const char * WHITESPACE = " \t\r\n\f";
	const long to_read = 256;
	char b[to_read+1]; b[to_read] = '\0';
	file.seekg(-to_read, std::ios::end);
	file.read(b, to_read);
	char * p;
	for(p = &b[to_read - 9]; p >= b; p--) {
		if(0 == ::memcmp(p, "startxref", 9))
			break;
	}
	if(p < b)
		return 0;
	p += ::strcspn(p, WHITESPACE);
	p += ::strspn(p, WHITESPACE);
	return atol(p);
}

void File::dump() const
{
  std::clog << "Xref table for " << filename << std::endl;
  xref_table.dump(std::clog);
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
	if(s.substr(0, 4) != "xref") {
		std::cerr << "Error in xref table" << std::endl;
		return false;
	}
	std::getline(file, s, GETLINE_ENDL); // get numbers
	//  std::clog << "Read first line of xref: " << s << std::endl;
	int sep=s.find_first_of(" \t");
	int objnum=atoi(s.substr(0,sep).c_str());
	sep=s.find_first_not_of(" \t", sep);
	int count=atoi(s.substr(sep).c_str());
	//  std::clog << "First object in this table: " << objnum << ", number of objects: " << count << std::endl;
	for(;count>0;count--,objnum++)
	{
		// XXX We can read 20byte chunks here!
		std::getline(file, s, GETLINE_ENDL);
		//std::clog << "XRef table line " << objnum << "/" << count << ": " << s << std::endl;
		s.erase(0, s.find_first_not_of("\r\n\t "));
		if(s.length() >= 17 && s[17] == 'n') // read unly "used" refs
		{
			ObjId oi;
			oi.num=objnum;
			oi.gen=atol(s.substr(11,5).c_str());
			xref_table.insert_normal(oi, atol(s.substr(0,10).c_str()));
		}
	}

	return true;
}

Dictionary * File::read_trailer()
{
	std::string s;
	size_t pos = file.tellg();
	std::getline(file, s, GETLINE_ENDL);
	s.erase(0, s.find_first_not_of("\r\n\t "));
	if(s.substr(0,7) != "trailer")
		throw FormatException("No trailer", pos);

	Object * r = strm.read_direct_object();
	Dictionary * dic = dynamic_cast<Dictionary *>(r);
	if(!dic) {
		delete r;
		throw FormatException("Error in trailer dictionary", pos);
	}
	return dic;
}

void File::read_xref_stream(Stream * s)
{
	// Determine table fields number and byte widths
	std::vector<int> widths;
	Array * a;
	if(!s->get_dict()->find("W", a))
		throw FormatException("No 'W' array in xref stream");
	unsigned long rowsize = 0;
	for(Array::ConstIterator it = a->get_const_iterator(); a->check_iterator(it); ++it) {
		long w;
		if(!(*it)->to_number(w))
			throw FormatException("Xref stream: W array must contain numbers");
		widths.push_back(w);
		rowsize += w;
	}

	// Get object numbers
	std::list< std::pair<unsigned long, unsigned int> > indexes;
	if(s->get_dict()->find("Index", a)) {
		for(unsigned int i = 0; i < a->size(); i += 2) {
			unsigned long start;
			unsigned int num;
			a->at(i)->to_number(start);
			a->at(i + 1)->to_number(num);
			indexes.push_back(std::make_pair(start, num));
		}
	} else {
		indexes.push_back(std::make_pair(0, (unsigned int)-1));
	}
	ObjId objid;
	objid.num = indexes.begin()->first;
	unsigned int subsection_entries = indexes.begin()->second;

	// Load xref table
	std::vector<char> buf;
	s->get_data(buf);

	for(unsigned int pos = 0; pos < buf.size(); pos += rowsize) {
		std::vector<unsigned long> row;
		char * cur = &buf[pos];
		for(std::vector<int>::const_iterator it = widths.begin(); it != widths.end(); ++it) {
			if(*it > 4)
				throw UnimplementedException("Xref stream table field width more than 4 bytes");
			int i = *it;
			unsigned long t = 0;
			while(i--) {
				t <<= 8;
				t |= *cur & 0xFF;
				cur++;
			}
			row.push_back(t);
		}

		switch(row[0]) {
			case 0: // free objid list
				objid.gen = row[2];
				xref_table.insert_empty(objid, row[1]);
				break;
			case 1: // Normal object reference
				objid.gen = row[2];
				xref_table.insert_normal(objid, row[1]);
				break;
			case 2: // Compressed object reference
				objid.gen = 0;
				xref_table.insert_compressed(objid, row[1], row[2]);
				break;
		}

		
		if(--subsection_entries)
			objid.num++;
		else {
			indexes.pop_front();
			if(indexes.empty())
				break;
			objid.num = indexes.begin()->first;
			subsection_entries = indexes.begin()->second;
		}
	} // for each row in stream data
}

long File::offset_lookup(const ObjId & oi) const
{
  return xref_table.get_offset(oi);
}

void File::load_crypto_dict(const Dictionary * d)
{
}

void File::load_file_ids(const Array * a)
{
	Array::ConstIterator it;
	for(it = a->get_const_iterator(); a->check_iterator(it); it++) {
		String * s = dynamic_cast<String *>(*it);
		if(!s)
			continue;
		m_file_ids.push_back( s->str() );
	}
}

} // namespace PDF


