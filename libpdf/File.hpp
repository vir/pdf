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
		std::streamoff offset;
		unsigned int obj_stream_index;
		enum { Normal, Compressed, Free } type;
		bool is_compressed() const { return type == Compressed; };
		bool is_free() const { return type == Free; };
		explicit Entry(): offset(0), obj_stream_index(0), type(Free) { }
		Entry(std::streamoff off):offset(off), obj_stream_index(0), type(Normal) { }
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
	std::streamoff get_offset(const ObjId & oid) const
	{
		const Entry * e = find(oid);
		if(!e)
			return 0; // XXX may be throw somthing?
		if(e->is_free())
			throw FreeObjectUsageException(oid.dump().c_str());
		if(e->is_compressed())
			throw UnimplementedException("Compressed objects streams");
		return e->offset;
	}
	void dump(std::ostream & strm = std::clog) const;
	void insert_normal(const ObjId & objid, std::streamoff offset)
	{
		/*
		 * Temp. worakaround a bug in some pdf updaters, which do not properly
		 * inctrmrnt object generation upon update, but overwrite the same object
		 * instead.
		 * As older revisions are parsed later, we do not update object offset if
		 * it is already known.
		 */
		if(m_table.find(objid) != m_table.end())
			return; // XXX
		m_table[objid] = Entry(offset);
	}
	void insert_empty(const ObjId & objid, unsigned long offset)
	{
		if(m_table.find(objid) != m_table.end())
			return; // XXX
		m_table[objid] = Entry();
	}
	void insert_compressed(const ObjId & objid, unsigned long stream_id, unsigned int index)
	{
		if(m_table.find(objid) != m_table.end())
			return; // XXX
		m_table[objid] = Entry(stream_id, index);
	}
	bool empty() const { return m_table.empty(); }
	void clear();
};

class ObjectStreamsCache
{
	private:
		struct Entry
		{
			ObjectStream * s;
			Entry(ObjectStream * strm):s(strm) { }
			//~Entry() { delete s; } // breaks (ptr copy on return cached obj)
		};
		std::map<long, Entry> m_stash;
		File & m_file;
	public:
		ObjectStreamsCache(File & f):m_file(f) { }
		~ObjectStreamsCache() { flush(); }
		Object * load_object(long obj_stream_num, unsigned int obj_stream_index);
		void flush() { for(std::map<long, Entry>::iterator it = m_stash.begin(); it != m_stash.end(); ++it) delete it->second.s; m_stash.clear(); }
};

/** \brief Represents pdf file
 * 
 * Used to access PDF low level file internals, such as xref table and
 * individual objects retrieval.
 *
 */
class File
{
public:
	enum OpenMode { MODE_CLOSED, MODE_READ, MODE_CREATE, MODE_UPDATE };

	private:
		OpenMode open_mode;
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
		ObjectStreamsCache * m_streams;
		std::fstream::pos_type m_header_end_offset;
	protected:
		void getline(std::string & s);
		std::streamoff offset_lookup(const ObjId & oi) const;

		std::string check_header();
		void write_header();

		long get_first_xreftable_offset();
		bool read_xref_table_part(bool try_recover = false);
		void read_xref_stream(Stream * s);
		void load_xref_table();
		std::streamoff write_xref_table();

		Dictionary * read_trailer();
		void write_trailer(Dictionary * trailerdict, std::streamoff xrefoffs);

		void load_crypto_dict(const Dictionary * d);
		void load_file_ids(const Array * a);
		std::ios_base::openmode open_prepare(std::string fname, OpenMode mode);
	public:
		File(std::string fn = "", OpenMode mode = MODE_CLOSED);
		explicit File(double pdf_version);
		~File();
		std::string id(unsigned int n = 0) const { return (n >= m_file_ids.size())?"":m_file_ids[0]; }
    /// sets debug level, returns previous debug level
    int debug(int d) { int t=m_debug; m_debug=d; return t; }
    /// returns pdf file version
    double version() const { return pdf_version; }
    bool open(std::string fname, OpenMode mode);
#ifdef _MSC_VER /* microsoft has secret wide-char filename open function */
	bool open(std::wstring fname, OpenMode mode);
#endif
    bool close();
    bool load();

	void dump(std::ostream & s) const;
		SecHandler * security() { return m_security; };
    
		Object * load_object(const ObjId & oi, bool decrypt = true);
		void save_object(Object * o);

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
	std::streamoff LoadXRefTable( std::streamoff xref_off, bool try_recover = false );
	void ReconstructXRefTable();
	bool can_load() const { return open_mode != MODE_CREATE; }
	bool can_save() const { return open_mode != MODE_READ; }
};

}; // namespace PDF



#endif /* PDF_FILE_HPP */

