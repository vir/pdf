
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
#include <ctype.h>

#include <stdlib.h> // for atof
#include <string.h> // for strstr, strspn

#include "File.hpp"

namespace PDF {

void XRefTable::dump( std::ostream & strm /*= std::clog*/ ) const
{
	for(std::map<ObjId, Entry>::const_iterator it = m_table.begin(); it != m_table.end(); ++it) {
		strm << "  object (" << it->first.num << "," << it->first.gen << ") is ";
		switch(it->second.type) {
		case Entry::Free:
			strm << "a free object";
			break;
		case Entry::Compressed:
			strm << "in stream " << it->second.offset << ", index: " << it->second.obj_stream_index;
			break;
		case Entry::Normal:
			strm << "at " << it->second.offset;
			break;
		}
		strm << std::endl;
	}
}

void XRefTable::clear()
{
	m_table.clear();
}

// PDF::File class members ==========================================================
// ============================================================================

/** Constructs PDF::File object */
File::File(std::string fn, OpenMode mode)
	: open_mode(MODE_READ)
	, istrm(NULL), ostrm(NULL)
	, m_debug(0)
	, m_security(NULL), m_streams(NULL)
{
	if(!fn.empty())
		open(fn, mode);
}

/** Constructs PDF::File object for new PDF file creation */
File::File(double pdf_version)
	: open_mode(MODE_CLOSED)
	, istrm(NULL), ostrm(NULL)
	, pdf_version(pdf_version)
	, m_debug(0)
	, m_security(NULL), m_streams(NULL)
{
}

File::~File()
{
	delete m_security;
	delete istrm;
	delete ostrm;
}

std::ios_base::open_mode File::open_prepare(std::string fname, OpenMode mode)
{
	if(! filename.empty())
		throw LogicException("File already opened");
	filename = fname;
	open_mode = mode;
	if(filename.empty())
		throw LogicException("No filename to open");
	switch(open_mode) {
		case MODE_READ:
			m_streams = new ObjectStreamsCache(*this);
			istrm = new ObjIStream(file);
			return std::ios::in|std::ios::binary;
		case MODE_CREATE:
			m_streams = new ObjectStreamsCache(*this);
			ostrm = new ObjOStream(file);
			return std::ios::out|std::ios::trunc|std::ios::binary;
		case MODE_UPDATE:
		default:
			throw UnimplementedException("Unimplemented open mode");
	}
}

bool File::open(std::string fn, OpenMode mode)
{
	std::ios_base::open_mode fom = open_prepare(fn, mode);
	file.open(filename.c_str(), fom);

	if(open_mode == MODE_CREATE && file.good())
			write_header();
	return file.good();
}

#ifdef _MSC_VER
bool File::open(std::wstring fn, OpenMode mode)
{
	std::ios_base::open_mode fom = open_prepare(std::string(fn.begin(), fn.end()), mode);
	file.open(fn.c_str(), fom);
	if(open_mode == MODE_CREATE && file.good())
		write_header();
	return file.good();
}
#endif

bool File::close()
{
	m_streams->flush();
	if(open_mode == MODE_CREATE || open_mode == MODE_UPDATE) {
		file.seekg(0, std::ios_base::end);
		long offs = write_xref_table();
		Dictionary * d = new Dictionary();
		long generation = 0;
		d->set("Root", new ObjRef(root_refs[generation]));
		write_trailer(d, offs);
		delete d;
	}
	delete m_security;
	m_security = NULL;
	m_file_ids.clear();
	delete istrm;
	istrm = NULL;
	delete ostrm;
	ostrm = NULL;
	root_refs.clear();
	xref_table.clear();
	file.close();
	filename.clear();
	pdf_version = 0;
	return true;
}

// load all indeexes and document directory
bool File::load()
{
	if(!file.good())
		return false;

	std::string ver=check_header();
	if(ver.length()) {
		pdf_version = (float)::atof(ver.c_str());
		if(m_debug) std::clog << "PDF version: " << version() << "\n";
	} else
		throw FormatException("Invalid header", 0);

	// load xref table
	long xref_off = get_first_xreftable_offset();
	if(xref_off)
		LoadXRefTable(xref_off);
	else
		ReconstructXRefTable();
	return true;
}






/// load object from file
Object * File::load_object(const ObjId & oi, bool decrypt)
{
	if(m_debug>1) std::clog << "Loading object " << oi.dump() << std::endl;
	const XRefTable::Entry * xe = xref_table.find(oi);

	if(!xe)
		return new MissingObjectPlaceholder;

	if(xe->free())
		return new FreeObjectPlaceholder;

	if(xe->compressed())
		return m_streams->load_object(xe->offset, xe->obj_stream_index);

	long offset = xe->offset;
	Object * r;
	if(offset) {
		file.seekg(offset);
		r = istrm->read_indirect_object(decrypt);

		Stream * str;
		if((str = dynamic_cast<Stream *>(r)))
			str->set_source(this);
	} else
		r = new Null();
	if(m_debug > 2)
		std::clog << "Loaded object: " << r->dump() << std::endl;
	return r;
}

void File::save_object(Object * o)
{
	if(! o->indirect)
		throw LogicException("File::save_object called with direct object");
	long pos = file.tellg();
	const ObjId & oi = o->m_id;;
	if(m_debug>1)
		std::clog << "Saving object " << oi.dump() << std::endl;
	ostrm->write_object(o);
	xref_table.insert_normal(oi, pos);
}


//////////////////// helper functions /////////////////////////////////////

// Returns version or empty string if not a PDF
std::string File::check_header()
{
	std::string line, version;
	file.seekg(0, std::ios::beg);
	getline(line);
	if(line.substr(0,5) == "%PDF-")
		version=line.substr(5,3);
	else
		throw FormatException("No PDF header", 0);
	// Find headers end in case we will read linearized pdfs one day
	do {
		m_header_end_offset = file.tellg();
		getline(line);
	} while(line[0] == '%');
	return version;
}

void File::write_header()
{
	file << "%PDF-" << std::setw(3) << pdf_version << "\r\n%\xE2\xE3\xCF\xD3\r\n";
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

void File::dump(std::ostream & s) const
{
	s << "File: " << filename << " Version: " << version() << std::endl;
	if(m_security)
		s << "Security handler present" << std::endl;
	s << "Xref table:" << std::endl;
	xref_table.dump(s);
	for(unsigned int i = 0; i < root_refs.size(); i++)
		s << "Root Element " << i << ": " << root_refs[i].dump() << std::endl;
}

/// Read segment of "xref table"
bool File::read_xref_table_part(bool try_recover)
{
	if(! isdigit(file.peek()))
		return false;

	std::string s;
	getline(s); // get numbers
	//  std::clog << "Read first line of xref: " << s << std::endl;
	int sep=s.find_first_of(" \t");
	int objnum=atoi(s.substr(0,sep).c_str());
	sep=s.find_first_not_of(" \t", sep);
	int count=atoi(s.substr(sep).c_str());
	//  std::clog << "First object in this table: " << objnum << ", number of objects: " << count << std::endl;
	for(; count>0 && !file.eof(); count--, objnum++)
	{
		long linestart = file.tellg();
		// XXX We can read 20byte chunks here!
		getline(s);
		//std::clog << "XRef table line " << objnum << "/" << count << ": " << s << std::endl;
		s.erase(0, s.find_first_not_of("\r\n\t "));
		if(s.length() < 17) {
			if(!try_recover)
				throw FormatException("Too short line in xref table", linestart);
			else {
				std::cerr << "Ignoring too short line in xref table at " << linestart << std::endl;
				continue;
			}
		}
		ObjId oi;
		oi.num=objnum;
		oi.gen=atol(s.substr(11,5).c_str());
		unsigned long offset = atol(s.substr(0,10).c_str());

		if(s[17] == 'n')
			xref_table.insert_normal(oi, offset);
		else if(s[17] == 'f')
			xref_table.insert_empty(oi, offset);
		else {
			if(!try_recover)
				throw FormatException("Unknown object type in xref", linestart);
			else
				std::cerr << "Ignoring unknown object type in xref " << linestart << std::endl;
		}
	}

	return true;
}

long File::write_xref_table()
{
	long r = file.tellg();
	//std::pair<ObjId, XRefTable::Entry>* iter = xref_table.get_next(true);
	std::map<ObjId, XRefTable::Entry>::value_type* iter = xref_table.get_next(true);
	file << "xref\r\n0 " << 1 + xref_table.count() << "\r\n";
	file << "0000000000 65535 f\r\n";
	while(iter) {
		char ot = iter->second.free() ? 'f' : 'n';
		file << std::setw(10) << std::setfill('0') << iter->second.offset << " " << std::setw(5) << std::setfill('0') << iter->first.gen << " " << ot << "\r\n";
		iter = xref_table.get_next(false);
	}
	return r;
}

Dictionary * File::read_trailer()
{
	size_t pos = file.tellg();
	std::string s(istrm->read_token());
	if(s != "trailer")
		throw FormatException("No trailer", pos);

	Object * r = istrm->read_direct_object();
	Dictionary * dic = dynamic_cast<Dictionary *>(r);
	if(!dic) {
		delete r;
		throw FormatException("Error in trailer dictionary", pos);
	}
	return dic;
}

void File::write_trailer(Dictionary * trailerdict, long xrefoffs)
{
	file << "trailer\r\n";
	trailerdict->indirect = false;
	ostrm->write_object(trailerdict);
	file << "\r\nstartxref\r\n" << xrefoffs << "\r\n%%EOF\r\n";
}

void File::read_xref_stream(Stream * s)
{
	// Determine table fields number and byte widths
	std::vector<int> widths;
	Array * a;
	if(!s->dict()->find("W", a))
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
	if(s->dict()->find("Index", a)) {
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
	s->encryption(false);
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

long File::LoadXRefTable( long xref_off, bool try_recover )
{
	while(xref_off)
	{
		Object * o;
		Dictionary * trailer = NULL;
		Object * victim = NULL; // Will be delete()d after segment is read
		file.seekg(xref_off);
		while(isspace(file.peek()))
			file.ignore();
		if(file.eof()) {
			if(try_recover) {
				std::cerr << "Ignoring EOF in XRef table" << std::endl;
				break;
			} else
				throw FormatException("EOF in XRef table", xref_off);
		}
		if(file.peek() == 'x') { // Normal xref table
			if(m_debug)
				std::clog << "Xref table is at " << xref_off << "\n";

			std::string s;
			getline(s); // get 'xref' header
			if(s.substr(0, 4) == "xref") {
				while( read_xref_table_part(try_recover) ) ;
			} else
				std::clog << "Somthing wrong with xref table" << std::endl;

			try {
				if(m_debug)
					std::clog << "Reading trailer at " << file.tellg() << "\n";
				victim = trailer = read_trailer();
			}
			catch(...) {
				if(try_recover) {
					std::cerr << "Error reading trailer, ignoring" << std::endl;
					victim = trailer = NULL;
				} else
					throw;
 			}
		} else {
			o = istrm->read_indirect_object(false);
			Stream * s = dynamic_cast<Stream *>(o);
			if(!s)
				throw FormatException("Error reading xref stream", xref_off);
			read_xref_stream(s);
			trailer = s->dict();
			victim = s;
		}
		if(!trailer) // may occur in recover mode
			continue;
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
			if(! m_security->set_password("")) // default empty password
				throw WrongPasswordException();
			istrm->set_security_handler(m_security);
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
	return xref_off;
}

void File::ReconstructXRefTable()
{
	std::fstream::pos_type objstart = m_header_end_offset;
	std::cerr << "Trying to reconstruct missing XRef table" << std::endl;
	file.seekg(objstart);
	// Here should be linearization dictionary
	Object * o = istrm->read_indirect_object(false);
	Dictionary * lindict = dynamic_cast<Dictionary *>(o);
	if(lindict && lindict->find("Linearized")) {
		std::clog << "Found Linearization dictionary, good" << std::clog;
		// Load "first page xref table" and may be some parts of other(s)
		std::fstream::pos_type xreftablestart = file.tellg();
		LoadXRefTable(file.tellg(), true);

#if 0
		Integer * offs;
		if(lindict->find("T", offs)) {
			// In case of "normal" table, T points to fitsh table row, not to "xref"
			// But in case of srefstream we can grt somthing useful here...
			// XXX TODO LoadXRefTable(offs->value(), true);
		}
#endif
		// LoadXRefTable may reposition file anywhere
		file.seekg(xreftablestart);

		// Skip all linearization stuff
		std::string line;
		do {
			getline(line);
		} while(line.substr(0, 5) != "%%EOF" && !file.eof());
	} // if linearized

	unsigned int count = 0;
	try {
		while(o) {
			xref_table.insert_normal(o->m_id, objstart);
			delete o;
			++count;
			objstart = file.tellg();
			o = istrm->read_indirect_object(true);
		}
	}
	catch(...) { }
	std::clog << "XRef table recovery: found " << count << " objects" << std::endl;
}

void File::getline( std::string & s )
{
#if 1 // Use my own getline to handle all weird line endings
	char c;
	s.clear();
	for(;;)
	{
		c = file.get();
		if(c == '\r' || c == '\n' || file.eof())
			break;
		s += c;
	}
	if(c == '\r' && file.peek() == '\n')
		file.ignore();
#else
	std::getline(file, s, '\n');
#endif
}

Object * ObjectStreamsCache::load_object(long obj_stream_num, unsigned int obj_stream_index)
{
	std::map<long, Entry>::iterator it = m_stash.find(obj_stream_num);
	if(it == m_stash.end()) { // Load object
		ObjId oid;
		oid.num = obj_stream_num;
		oid.gen = 0;
		Object * o = m_file.load_object(oid);
		Stream * s = dynamic_cast<Stream *>(o);
		if(!s)
			throw FormatException("Object Stream is not a Stream");
		ObjectStream * os = new ObjectStream(s);
		delete s;
		it = m_stash.insert(std::make_pair(obj_stream_num, Entry(os))).first;
	}
	return it->second.s->load_object(obj_stream_index);
}

} // namespace PDF


