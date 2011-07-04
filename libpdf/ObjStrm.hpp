#ifndef OBJSTRM_HPP_INCLUDED
#define OBJSTRM_HPP_INCLUDED

#include <iostream>
#include <ios>
#include <vector>

namespace PDF {

class ObjId;
class Object;
class Stream;
class SecHandler;

class StreamBuffer:public std::streambuf // See http://www.mr-edd.co.uk/blog/beginners_guide_streambuf
{
	private:
		Stream * m_source;
		std::vector<char> m_data;
	private: // std::streambuf overrides
		std::streambuf::int_type underflow();
		virtual std::ios::pos_type __CLR_OR_THIS_CALL seekoff(std::ios::off_type off, std::ios::seekdir dir, std::ios::openmode  mode = std::ios::in | std::ios::out)
		//virtual std::ios::streampos seekoff(std::ios::streamoff off, std::ios::seek_dir dir, int mode = std::ios::in | std::ios::out)
		{
			char * pos = NULL;
			switch(dir) {
				case std::ios::beg: pos = &*m_data.begin() + off; break;
				case std::ios::cur: pos = gptr() + off; break;
				case std::ios::end: pos = &*m_data.begin() + m_data.size() + off; break;
				default: break;
			}
			setg(&*m_data.begin(), pos, &*m_data.begin() + m_data.size());
			return pos - &*m_data.begin();
		}
		virtual std::ios::pos_type __CLR_OR_THIS_CALL seekpos(std::ios::pos_type off, std::ios::openmode mode = std::ios::in | std::ios::out)
		//virtual std::ios::streampos seekpos(std::ios::streamoff off, int mode = std::ios::in | std::ios::out)
		{
			return seekoff(off, std::ios::beg, mode);
		}
	public:
		StreamBuffer(Stream * s, bool load_now = false):m_source(s)
		{
			if(load_now)
				underflow();
			else
				setg(0, 0, 0);
		}
};

class ObjIStream
{
	private:
		std::istream * f;
		bool ownstream;
		SecHandler * sechandler;
		bool will_throw_eof;
		bool will_allow_keywords;
	protected:
		// elemental operations
		inline void skip_whitespace();
		std::string read_token();
		// some very-low-level helper functions
		std::vector<char> read_o_string();
		Object * read_o_digits();
		Object * read_o_chars();
		Object * read_delimited(const ObjId * decrypt_info = NULL);
		Object * read_direct_object(const ObjId * decrypt_info);
		bool IsOctalDigit(char c) { return c >= '0' && c < '8'; }
	public:
		ObjIStream(std::istream & sref, bool want_throw_eof = true):f(&sref),ownstream(false),sechandler(NULL),will_throw_eof(want_throw_eof),will_allow_keywords(false) { }
		ObjIStream(Stream * object_stream):f(new std::istream(new StreamBuffer(object_stream))), ownstream(true), sechandler(NULL), will_throw_eof(false), will_allow_keywords(false) { }
		~ObjIStream() { if(ownstream) { f->clear(); delete f; } }
		void set_security_handler(SecHandler * h) { sechandler = h; }
		void throw_eof(bool want) { will_throw_eof = want; }
		bool throw_eof() { return will_throw_eof; }
		void allow_keywords(bool want) { will_allow_keywords = want; }
		bool allow_keywords() { return will_allow_keywords; }
		Object * read_direct_object() { return read_direct_object(NULL); }
		Object * read_indirect_object(bool need_decrypt = true);
		void read_chunk(unsigned long offset, char * buf, unsigned int len, long obj_id_num = 0, long obj_id_gen = 0);
};

class ObjectStream
{
private:
	std::istream m_sis;
	ObjIStream m_ois;
	unsigned int m_objsnum;
	std::vector<unsigned long> m_offsets;
public:
	ObjectStream(Stream * s);
	Object * load_object(unsigned int index)
	{
		if(index > m_objsnum)
			throw std::exception("Object Stream: requested object's index greater than objects number");
		m_sis.seekg(m_offsets[index]);
		return m_ois.read_direct_object();
	}
};

class ObjOStream
{
	private:
		std::ostream * f;
	public:
		ObjOStream(std::ostream & sref):f(&sref) { }
		void write_direct_object(Object * o);
		void write_indirect_object(Object * o, bool need_encrypt = true);
};

} /* namespace PDF */

#endif /* OBJSTRM_HPP_INCLUDED */

