#include "ObjStrm.hpp"
#include <string.h>
#include <stdlib.h>
#include "Object.hpp"
#include "Exceptions.hpp"
#include "SecHandler.hpp"

namespace PDF {

/// convert one hex digit to integer 0..15
inline int hex_to_int(char c) { return ((c>=0 && c<='9')?c-'0':(c&0xCF)-'A'+10)&0x0F; }
inline bool is_a_whitespace(char c) { return ::strchr("\x09\x0A\x0C\x0D\x20", c) != NULL; /* plus '0x00' char */ }
inline bool is_a_delimiter(char c) { return ::strchr("()<>[]{}/%", c) != NULL; }

/** \brief Reads object from current position in a given stream
 *
 * Main object reading functions. Reads direct objects at current position.
 */
Object * ObjIStream::read_direct_object(const ObjId * decrypt_info)
{
	skip_whitespace();

	Object * r = NULL;

	long objstart = f->tellg();

	char c = f->peek();
	if(!will_throw_eof && c == EOF)
		return r;
	if(is_a_delimiter(c)) {
		r=read_delimited(decrypt_info); // string/name/array/dict/stream
	} else if((c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+') {
		r=read_o_digits(); // int/real/reference
	} else
		r = read_o_chars(); // boolean/error
	if(!r)
		throw FormatException("Error reading object at offset ", objstart);

	r->m_offset = objstart; // overwrited in read_indirect !

#if 0
	if(m_debug) {
		std::clog << "Read object: " << r->type();
		if(!dynamic_cast<String *>(r) || m_debug>1) std::clog << "(" << r->dump() << ")";
		std::clog << std::endl;
	}
#endif

	return r;
}

/// read [NNN GG obj OBJ endobj] structure, return OBJ
Object * ObjIStream::read_indirect_object(bool need_decrypt)
{
	ObjId objid;
	std::string s;
  skip_whitespace();

	/* read object number */
	long start = f->tellg();
	s = read_token();
	if(static_cast<int>(s.find_first_not_of("0123456789")) >= 0)
		throw FormatException("Invalid object number", start);
	objid.num = ::atol(s.c_str());

	/* read generation */
	skip_whitespace();
	s = read_token();
	if(static_cast<int>(s.find_first_not_of("0123456789")) >= 0)
		throw FormatException("Invalid object generation", start);
	objid.gen = ::atol(s.c_str());

	skip_whitespace();
	s = read_token();
	if(s != "obj")
		throw FormatException("Invalid object", start);

	Object * r = read_direct_object(need_decrypt?&objid:NULL); // read the object

	Stream * ss = dynamic_cast<Stream *>(r);
	if(!(ss && ss->delayed_load())) { // skip check if delayed stream loading as we don't know it's length
		skip_whitespace();
		s = read_token();
		if(s!="endobj")
			throw FormatException("Error at object end", f->tellg());
	}

	r->indirect = true;
	r->m_id = objid;
	return r;
}

/** \brief Reads object, which starts with delimiter character
 * Reads delimited objects (String, Name, Array, Dictionary, Stream)
 */
Object * ObjIStream::read_delimited(const ObjId * decrypt_info)
{
	long start = f->tellg();
	char c; *f >> c;
	Object * o;
	switch(c)
	{
		case '(':
			{
				std::vector<char> str = read_o_string();
				if(decrypt_info && sechandler) {
					sechandler->decrypt_object(decrypt_info->num, decrypt_info->gen, (char*)&str[0], str.size());
				}
				String * s = new String( str );
				f->ignore(); // skip ')'
				return s;
			}
			break;
		case '[':
			{
				Array * a = new Array();
				skip_whitespace();
				while(f->peek() != ']')
        {
					Object * o=read_direct_object(decrypt_info);
					if(o) a->push(o);
					else if(!will_throw_eof) // still throw if inside array
						throw FormatException("EOF inside an array", start);
					skip_whitespace();
				}
				f->ignore(); // skip ']'
				return a;
			}
			break;
		case '<':
			{
				c = f->get();
				if(c == '<') // second '<' => Dictionary
				{
					Dictionary * r = new Dictionary();
					while(1) {
						skip_whitespace();
						if(f->peek() == '>') {
							f->ignore();
							if(f->peek() == '>') { f->ignore(); break; } // normal dict. end
							else
								throw FormatException("Invalid dictionary end", f->tellg());
						}

						o = read_direct_object(decrypt_info);
						Object * value = read_direct_object(decrypt_info);
						if((!o || !value) && !will_throw_eof) // Still throw if eof is inside dict
							throw FormatException("EOF instide a dictionary", start);

						Name * key = dynamic_cast<Name *>(o);
						if(!key)
							throw FormatException("Dictionary key is not a Name", start);

						r->set(key->value(), value);
						delete key; // string from it copyed in prev. line
					}

					if(( o = r->find("Length") )) { // check for stream object
						long savepos = f->tellg();
						skip_whitespace();
						std::string s = read_token();
						if(s == "stream") {
							/* find stream data */
							c = f->get(); if(c=='\x0D') c = f->get();
							if(c != '\x0A')
								throw FormatException("No newline after 'stream' keyword?", f->tellg());
							long stream_begin = f->tellg();

							/* extract stream length */
							Integer * ii = dynamic_cast<Integer *>(o);
							if(ii) {
								long stream_length = ii->value();

								/* skip to stream tail */
								f->seekg(stream_begin + stream_length);
								skip_whitespace();
								s = read_token();
								if(s != "endstream")
									throw FormatException("Invlaid stream end", f->tellg());
								return new Stream(r, this, stream_begin, stream_length);
							} else {
								ObjRef * rr = dynamic_cast<ObjRef *>(o);
								if(!rr)
									throw FormatException("Stream length is not an integer and not a reference - giving up!", savepos);
								std::cerr << "Stream length is a reference - delaing length extraction (pos: " << savepos << ")" << std::endl;
								return new Stream(r, this, stream_begin, rr);
							}
						} else
							f->seekg(savepos);
					}
					return r;
				}
				else // hex string
				{
					std::string s;
					s += c; // put back first character, extracted in dict. check
					do {
						skip_whitespace();
						s+=read_token();
						skip_whitespace();
					} while(f->peek() != '>' && f->peek() != EOF);
					f->ignore(); // check for errors?
					if(s.length() % 2) s+='0'; // pad to even length
					std::vector<char> r;
					for(unsigned int i=0; i<s.length(); i+=2) {
						char c=(hex_to_int(s[i]) << 4) | hex_to_int(s[i+1]);
						r.push_back(c);
					}
					if(sechandler && decrypt_info)
						sechandler->decrypt_object(decrypt_info->num, decrypt_info->gen, (char*)&r[0], r.size());
					return new String(r);
				}
			}
			break;
		case '/':
			return new Name( read_token() );
			break;
		default:
			std::cerr << "Unhandled delimiter '" << c << "' at offset " << start << std::endl;
			break;
	}
	return NULL;
}

//====================== helper functions =====================

/** \brief Read String from a given stream
 * 
 * \todo add all valid escape sequences
 */
std::vector<char> ObjIStream::read_o_string()
{
  std::vector<char> s; char c; bool escape=false;
	int plevel = 0;
	do {
		c = f->get();
		if(c == '\\' && !escape) escape=true;
		else
		{
			if(escape) {
				escape = false;
				switch(c) {
					case 'n': c = '\n'; break;
					case 'r': c = '\r'; break;
					case 'b': c = '\x08'; break;
					case '(': case ')': case '\\': break;
					default: std::cerr << "Unknown escaped char " << c << std::endl; break;
				}
			} else {
				switch(c) {
					case '(': plevel++; break;
					case ')': plevel--; break;
					default: break;
				}
			}
			s.push_back(c);
			if(f->peek() == ')' && plevel == 0) break;
		}
	} while(f->peek() != EOF);
	return s;
}

Object * ObjIStream::read_o_digits()
{
	std::string s = read_token();
	std::istringstream ss(s);
	long firstint;
	if(s.find_first_of(".") != std::string::npos) {
		double v;
		ss >> v;
		return new Real(v); // real
	} else {
		ss >> firstint;
	}

	if(s[0] == '-' || s[0] == '+')
		return new Integer(firstint);

	// look what is there (another number + R => reference), else => integer
	long savepos = f->tellg();
	skip_whitespace();

	// check out next token --- must be an integer, and after it 'R'
	s = read_token();
	if(s.find_first_not_of("0123456789") > 0) { // must be only digits
		long secondint = ::atol(s.c_str()); // XXX replace with istringstream
		skip_whitespace();
		s = read_token();
		if(s == "R") // Reference
			return new ObjRef(firstint, secondint);
	}

	// not a reference, get back...
	f->seekg(savepos);
	return new Integer(firstint); // integer
}

/// \todo enable keywords reading only when they are allowed
Object * ObjIStream::read_o_chars()
{
	long startpos = f->tellg();
	std::string b=read_token();
	if(b == "true") return new Boolean(true);
	if(b == "false") return new Boolean(false);
	if(b == "null") return new Null();
	if(will_allow_keywords) return new Keyword(b);
	else {
		std::cerr << "invalid boolean (or garbage?) object at offset " << startpos << std::endl;
		return new Null();
	}
}

/// skip whitespace characters
void ObjIStream::skip_whitespace()
{
	while(f->peek() != EOF && (is_a_whitespace(f->peek()) || f->peek()=='\0')) {
		f->ignore();
	}
	if(will_throw_eof && f->peek() == EOF) throw FormatException("Unexpected EOF");
}

/// read a single "word"
std::string ObjIStream::read_token()
{
	std::string s;
	char c;
	do {
		*f >> c;
		s+=c;
	} while(! is_a_delimiter(f->peek())
			&& !is_a_whitespace(f->peek())
			&& f->peek()!='\0'
			&& f->peek()!=EOF);
	if(will_throw_eof && f->peek() == EOF) throw FormatException("Unexpected EOF");
	return s;
}

void ObjIStream::read_chunk(unsigned long offset, char * buf, unsigned int len, long obj_id_num, long obj_id_gen)
{
	f->seekg(offset);
	f->read(buf, len);
	if(sechandler && obj_id_num)
		sechandler->decrypt_object(obj_id_num, obj_id_gen, buf, len);
}

std::streambuf::int_type StreamBuffer::underflow()
{
	if(m_data.empty() && m_source) {
		m_source->get_data(m_data);
		setg(&*m_data.begin(), &*m_data.begin(), &*m_data.begin() + m_data.size());
	}
	return gptr() == egptr() ? traits_type::eof() : traits_type::to_int_type(*gptr());
}

ObjectStream::ObjectStream(Stream * s): m_sis(new StreamBuffer(s, true)), m_ois(m_sis, false)
{
	Dictionary * d = s->get_dict();
	d->find("N")->to_number(m_objsnum);
	unsigned long first_offset;
	d->find("First")->to_number(first_offset);
	for(unsigned int i = 0; i < m_objsnum; ++i) {
		unsigned long offset;
		Object * o;
		o = m_ois.read_direct_object(); // object number - ignore it
		delete o;
		o = m_ois.read_direct_object(); // 
		o->to_number(offset);
		delete o;
		m_offsets.push_back(first_offset + offset);
	}
}



} /* namespace PDF */


