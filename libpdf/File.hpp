#ifndef PDF_FILE_HPP
#define PDF_FILE_HPP

// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include <fstream>
#include <string>
#include <map>
#include <vector>
//#include <sstream>

#include "Object.hpp"
#include "SecHandler.hpp"
#include "ObjStrm.hpp"
#include "Exceptions.hpp"

namespace PDF {
/** \brief Cross-reference table
 * The cross-reference table contains information that permits random
 * access to indirect objects within the file so that the entire file
 * need not be read to locate any particular object.
 */
class XRefTable
{
public: // types
	struct Entry
	{
		enum { Normal, Compressed, Free } type;
		unsigned long offset;
		unsigned int obj_stream_index;
		bool compressed() const { return type == Compressed; };
		bool free() const { return type == Free; };
		explicit Entry(): offset(0), obj_stream_index(0), type(Free) { }
		Entry(unsigned long off):offset(off), obj_stream_index(0), type(Normal) { }
		Entry(unsigned long objnum, unsigned int index):offset(objnum), obj_stream_index(index), type(Compressed) { }
	};
private:
	std::map<ObjId, Entry> m_table;
	std::map<ObjId, Entry>::iterator m_table_it;
public:
	XRefTable():m_table_it(m_table.begin()) { }
	std::map<ObjId, Entry>::value_type* get_next(bool reset = false)
	{
		if(reset)
			m_table_it = m_table.begin();
		if(m_table_it == m_table.end())
			return NULL;
		return &(*m_table_it++);
	}
	size_t count() const { return m_table.size(); }
	Entry * find(const ObjId & oid)
	{
		std::map<ObjId, Entry>::iterator it = m_table.find(oid);
		return (it != m_table.end())?&it->second:NULL;
	}
	const Entry * find(const ObjId & oid) const
	{
		std::map<ObjId, Entry>::const_iterator it = m_table.find(oid);
		return (it != m_table.end())?&it->second:NULL;
	}
	long get_offset(const ObjId & oid) const
	{
		const Entry * e = find(oid);
		if(!e)
			return 0; // XXX may be throw somthing?
		if(e->free())
			throw FreeObjectUsageException(oid.dump().c_str());
		if(e->compressed())
			throw UnimplementedException("Compressed objects streams");
		return e->offset;
	}
	void dump(std::ostream & strm = std::clog) const;
	void insert_normal(const ObjId & objid, unsigned long offset)
	{
		m_table[objid] = Entry(offset);
	}
	void insert_empty(const ObjId & objid, unsigned long offset)
	{
		m_table[objid] = Entry();
	}
	void insert_compressed(const ObjId & objid, unsigned long stream_id, unsigned int index)
	{
		m_table[objid] = Entry(stream_id, index);
	}
};

class ObjectStreamsCache
{
	private:
		struct Entry
		{
			ObjectStream * s;
			Entry(ObjectStream * strm):s(strm) { }
		};
		std::map<long, Entry> m_stash;
		File & m_file;
	public:
		ObjectStreamsCache(File & f):m_file(f) { }
		Object * load_object(long obj_stream_num, unsigned int obj_stream_index);
};

/** \brief Represents pdf file
 * 
 * Used to access PDF low level file internals, such as xref table and
 * individual objects retrieval.
 *
 */
class File
{
	private:
		enum { MODE_READ, MODE_WRITE, MODE_UPDATE } open_mode;
		std::string filename;
		std::fstream file;
		ObjIStream * istrm;
		ObjOStream * ostrm;
		double pdf_version;
    /// references to all root node generations, starting with last one
    std::vector<ObjId> root_refs;
    /// full xref table
    //typedef std::map<ObjId,long> XrefTableType;
    //XrefTableType xref_table;
	XRefTable xref_table;
    int m_debug;
		SecHandler * m_security;
		std::vector< std::string > m_file_ids;
		ObjectStreamsCache m_streams;
		std::fstream::pos_type m_header_end_offset;
	protected:
		void getline(std::string & s);
		long offset_lookup(const ObjId & oi) const;

		std::string check_header();
		void write_header();

		long get_first_xreftable_offset();
		bool read_xref_table_part(bool try_recover = false);
		void read_xref_stream(Stream * s);
		void load_xref_table();
		long write_xref_table();

		Dictionary * read_trailer();
		void write_trailer(Dictionary * trailerdict, long xrefoffs);

		void load_crypto_dict(const Dictionary * d);
		void load_file_ids(const Array * a);
	public:
		File(std::string fn="");
		File(double pdf_version, std::string fn="");
		~File();
		std::string id(unsigned int n = 0) const { return (n >= m_file_ids.size())?"":m_file_ids[0]; }
    /// sets debug level, returns previous debug level
    int debug(int d) { int t=m_debug; m_debug=d; return t; }
    /// returns pdf file version
    double version() const { return pdf_version; }
    bool open(std::string fname="");
    bool close();
    bool load();

	void dump(std::ostream & s) const;
		SecHandler * security() { return m_security; };
    
		Object * load_object(const ObjId & oi, bool decrypt = true);
		void save_object(Object * o, const ObjId & oi, bool encrypt = true);

    /// returns number of root element references found in file
    long generations_num() const { return root_refs.size(); }
    /// returns root object id for a given generation
	const ObjId & get_root(long generation=0) const
	{
		if(root_refs.empty())
			throw FormatException("No root references");
		return root_refs[generation];
	}
	void set_root(ObjId id, long generation = 0)
	{
		if(root_refs.size() < (unsigned long)generation + 1)
			root_refs.resize(generation + 1);
		root_refs[generation] = id;
	}
	long LoadXRefTable( long xref_off, bool try_recover = false );
	void ReconstructXRefTable();
};


}; // namespace PDF



#endif /* PDF_FILE_HPP */

