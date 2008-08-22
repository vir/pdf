#ifndef OBJSTRM_HPP_INCLUDED
#define OBJSTRM_HPP_INCLUDED

#include <iostream>

namespace PDF {

class ObjId;
class Object;
class Stream;
class SecHandler;

class ObjIStream
{
	private:
		std::istream & f;
		SecHandler * sechandler;
		bool will_throw_eof;
		bool allow_keywords;
	protected:
		// elemental operations
		inline void skip_whitespace();
		std::string read_token();
		// some very-low-level helper functions
		std::string read_o_string();
		Object * read_o_digits();
		Object * read_o_chars();
		Object * read_delimited(const ObjId * decrypt_info = NULL);
		Object * read_direct_object(const ObjId * decrypt_info);
	public:
		ObjIStream(std::istream & sref, bool want_throw_eof = true):f(sref),sechandler(NULL),will_throw_eof(want_throw_eof),allow_keywords(false) { }
		void set_security_handler(SecHandler * h) { sechandler = h; }
		void throw_eof(bool want) { will_throw_eof = want; }
		bool throw_eof() { return will_throw_eof; }
		Object * read_direct_object() { return read_direct_object(NULL); }
		Object * read_indirect_object(bool need_decrypt = true);
		void load_stream_data(Stream * s);
};

} /* namespace PDF */

#endif /* OBJSTRM_HPP_INCLUDED */

