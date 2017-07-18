#ifdef _MSC_VER
# pragma warning(disable : 4786) // get rid of "identifier was truncated to '255' characters..."
# ifdef _DEBUG
#  ifdef _CRTDBG_MAP_ALLOC
#   include <stdlib.h>  
#   include <crtdbg.h>  
#   ifndef DBG_NEW
#    define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#    define new DBG_NEW
#   endif
#  endif
# endif // _DEBUG
#endif

#include "ObjStrm.hpp"
#include <string.h>
#include <stdlib.h>
#include "Object.hpp"
#include "Exceptions.hpp"
#include "SecHandler.hpp"
#include <cstdio> // for EOF

namespace PDF {

Object * ObjectStream::load_object(unsigned int index)
{
	if(index > m_objsnum)
		throw FormatException("Object Stream: requested object's index greater than objects number");
	m_sis.seekg(m_offsets[index]);
	return m_ois.read_direct_object();
}

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

	std::streamoff objstart = f->tellg();

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
	std::streamoff start = f->tellg();
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
	std::streamoff start = f->tellg();
	char c; *f >> c;
	Object * o;
	switch(c)
	{
		case '(':
			{
				std::vector<char> str = read_o_string();
				if(decrypt_info && sechandler) {
					sechandler->decrypt_object(decrypt_info->num, decrypt_info->gen, str, SecHandler::OBJ_STRING);
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
						std::streamoff savepos = f->tellg();
						skip_whitespace();
						std::string s = read_token();
						if(s == "stream") {
							/* find stream data */
							c = f->get();
							if(c=='\x0D') {
								c = f->peek();
								if(c == '\x0A')
									f->get();
							} else if(c != '\x0A')
								throw FormatException("No newline after 'stream' keyword", f->tellg());
							std::streamoff stream_begin = f->tellg();

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
						sechandler->decrypt_object(decrypt_info->num, decrypt_info->gen, r, SecHandler::OBJ_STRING);
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
	if(f->peek() == ')') // some perverts are drawing empty strings
		return s;
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
					default:
						if(c >= '0' && c < '8') {
							// read 3 octal digits
							char buf[4];
							buf[0] = c;
							if(!IsOctalDigit(c = f->peek())) {
								std::cerr << "Invalid octal digit " << c << std::endl;
								break;
							}
							buf[1] = f->get();
							if(!IsOctalDigit(c = f->peek())) {
								std::cerr << "Invalid octal digit " << c << std::endl;
								break;
							}
							buf[2] = f->get();
							buf[3] = '\0';
							unsigned int t;
							sscanf(buf, "%o", &t);
							c = (char)t;
							break;
						}
						std::cerr << "Unknown escaped char " << c << std::endl;
						break;
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
	std::streamoff savepos = f->tellg();
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
	std::streamoff startpos = f->tellg();
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
	do {
		while(f->peek() != EOF && (is_a_whitespace(f->peek()) || f->peek() == '\0')) {
			f->ignore();
		}
		if(will_throw_eof && f->peek() == EOF) throw FormatException("Unexpected EOF");
		if(f->peek() == '%') { // skip comments - whitespace equivalent
			while(f->peek() != EOF && f->peek() != '\r' && f->peek() != '\n')
				f->ignore();
		}
	} while(is_a_whitespace(f->peek()));
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

void ObjIStream::read_stream_body(std::streamoff offset, std::vector<char>& buf, long obj_id_num, long obj_id_gen)
{
	f->seekg(offset);
	f->read(&buf[0], buf.size());
	if(sechandler && obj_id_num)
		sechandler->decrypt_object(obj_id_num, obj_id_gen, buf, SecHandler::OBJ_STREAM);
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
	Dictionary * d = s->dict();
	if(d->find("Extends"))
		throw UnimplementedException("ObjectStream 'Extends' feature");
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



/*====== ObjOStream ================================*/


void ObjOStream::write_direct_object(Object * o)
{
#define OBJECT_TYPE_ID_TRY(t) t * p_##t = dynamic_cast<t *>(o);
	OBJECT_TYPE_ID_TRY(Null);
	OBJECT_TYPE_ID_TRY(Real);
	OBJECT_TYPE_ID_TRY(Integer);
	OBJECT_TYPE_ID_TRY(Boolean);
	OBJECT_TYPE_ID_TRY(String);
	OBJECT_TYPE_ID_TRY(Name);
	OBJECT_TYPE_ID_TRY(Array);
	OBJECT_TYPE_ID_TRY(Dictionary);
	OBJECT_TYPE_ID_TRY(Stream);
	OBJECT_TYPE_ID_TRY(ObjRef);
	OBJECT_TYPE_ID_TRY(Keyword);
#undef OBJECT_TYPE_ID_TRY
	if(p_Null) {
		*f << "null";
	} else if(p_Boolean) {
		*f << (p_Boolean->value() ? "true" : "false");
	} else if(p_Real) {
		*f << p_Real->value();
	} else if(p_Integer) {
		*f << p_Integer->value();
	} else if(p_String) {
		*f << p_String->str();
	} else if(p_Name) {
		*f << "/" << p_Name->value();
	} else if(p_Array) {
		*f << "[";
		bool first = true;
		for(Array::ConstIterator it = p_Array->get_const_iterator(); p_Array->check_iterator(it); ++it) {
			if(! first)
				*f << " ";
			else
				first = false;
			write_direct_object(*it);
		}
		*f << "]";
	} else if(p_Dictionary) {
		*f << "<<";
		for(Dictionary::Iterator it = p_Dictionary->get_iterator(); p_Dictionary->check_iterator(it); ++it) {
			*f << "/" << it->first << " ";
			write_direct_object(it->second);
		}
		*f << ">>";
	} else if(p_Stream) {
		write_direct_object(p_Stream->dict());
		*f << "stream\r\n";
		p_Stream->save_data(this);
		*f << "\r\nendstream";
	} else if(p_ObjRef) {
		*f << p_ObjRef->ref().num << " " << p_ObjRef->ref().gen << " R";
	} else if(p_Keyword) {
		*f << p_Keyword->value();
	} else
		throw UnimplementedException("Can not save that object type");
}

void ObjOStream::write_object(Object * o)
{
	if(o->indirect) {
		*f << o->m_id.num << " " << o->m_id.gen << " obj";
	}
	write_direct_object(o);
	if(o->indirect)
		*f << " endobj\r\n";
}

void ObjOStream::write_chunk( const char * buf, unsigned int len, long obj_id_num /*= 0*/, long obj_id_gen /*= 0*/ )
{
	// XXX No encryption support!!
	f->write(buf, len);
}

} /* namespace PDF */


