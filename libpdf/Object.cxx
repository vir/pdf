
#include <iostream>
#include <string>
#include <cstring>
#include <map>

//#include "PDF.hpp"
#include "Object.hpp"
#include "Exceptions.hpp"
#include "Filter.hpp" // for stream
#include "File.hpp" // for loading indirect object in stream's dictionary
#include <string.h>

#define NOT_IMPLEMENTED(a) \
 throw std::string(__FILE__ ": Not implemented: "a);

namespace PDF {

inline bool is_a_whitespace(char c)
{
	return strchr("\x09\x0A\x0C\x0D\x20", c) != NULL; // plus '0x00' char!
}

inline bool is_a_delimiter(char c)
{
	return (bool)strchr("()<>[]{}/%", c) != NULL;
}

unsigned int Object::m_debug=0;

/* 
 * Done:       Null ObjRef Boolean Integer Real Reference Name Dictionary Array Stream
 * ToDo:       String( () and hex ), test Array
 */

// static helper functions, visible in this file only
static std::string read_o_string(std::istream & f, bool alt=false);
static Object *    read_o_digits(std::istream & f, bool alt=false);
static Object *     read_o_chars(std::istream & f, bool alt=false);
inline void skip_whitespace(std::istream & f, bool robust=false);
static std::string read_token(std::istream & f, bool robust=false);
/// convert one hex digit to integer 0..15
inline int hex_to_int(char c) { return ((c>=0 && c<='9')?c-'0':(c&0xCF)-'A'+10)&0x0F; }

#define OBJECT_TYPE_ID_TRY(T) if(dynamic_cast<const T *>(this)) return #T;
std::string Object::type() const
{
  OBJECT_TYPE_ID_TRY(Null);
  OBJECT_TYPE_ID_TRY(Real);
  OBJECT_TYPE_ID_TRY(Integer);
  OBJECT_TYPE_ID_TRY(String);
  OBJECT_TYPE_ID_TRY(Name);
  OBJECT_TYPE_ID_TRY(Array);
  OBJECT_TYPE_ID_TRY(Dictionary);
  OBJECT_TYPE_ID_TRY(Stream);
  OBJECT_TYPE_ID_TRY(ObjRef);
  OBJECT_TYPE_ID_TRY(Keyword);
  OBJECT_TYPE_ID_TRY(Object);
  return "Somthing strange";
}

/** \brief Reads object from current position in a given stream
 *
 * Main object reading functions. Reads direct objects from a given stream
 * at current position. Input stream \a f must be seekable.
 * 
 * \arg f   - input stream to read object.
 * \arg alt - alternative end-of-file behavior. Default behavior is to throw
 *      an "Unexpected end of file" exception. If \p alt == \c true, function will
 *      return \c NULL to indicate end of input stream.
 */
/*static*/ Object * Object::read(std::istream & f, bool alt/*=false*/)
{
  skip_whitespace(f, alt);
  
  Object * r=NULL;
  
  long objstart=f.tellg();
  
  char c=f.peek();
  if(alt && c == EOF) return r;
  if(is_a_delimiter(c)) r=read_delimited(f, alt); // string/name/array/dict/stream
  else if((c>='0' && c<='9') || c=='.' || c=='-' || c=='+') r=read_o_digits(f, alt); // int/real/reference
  else r=read_o_chars(f, alt); // boolean/error
  if(!r) throw FormatException("Error reading object at offset ", objstart);

//  r->indirect=false;
  r->m_offset=objstart; // overwrited in read_indirect !

  if(m_debug) {
		std::clog << "Read object: " << r->type();
		if(!dynamic_cast<String *>(r) || m_debug>1) std::clog << "(" << r->dump() << ")";
		std::clog << std::endl;
	}
  
  return r;
}

/// read [NNN GG obj OBJ endobj] structure, return OBJ
/*static*/ Object * Object::read_indirect(std::istream & f)
{
  std::string s;
  skip_whitespace(f);
  // read object number
  long start=f.tellg();
  s=read_token(f);
  if(static_cast<int>(s.find_first_not_of("0123456789"))>=0) throw FormatException("Invalid object number", start);
  long objnum=atol(s.c_str());
  
  // read generation
  skip_whitespace(f);
  s=read_token(f);
  if(static_cast<int>(s.find_first_not_of("0123456789"))>=0) throw FormatException("Invalid object generation", start);
  long objgen=atol(s.c_str());
  
  skip_whitespace(f);
  s=read_token(f);
  if(s!="obj") throw FormatException("Invalid object", start);

  Object * r=read(f); // read the object
  
	if(dynamic_cast<Stream *>(r) == NULL) // skip some checks for stream object as we do not read them
	{
		skip_whitespace(f);

		s=read_token(f);
		if(s!="endobj") throw FormatException("Error at object end", f.tellg());
	}

//  std::cerr << "Read indirect object (" << objnum << "," << objgen << ")" << std::endl;
  r->indirect=true;
  r->m_id.num=objnum;
  r->m_id.gen=objgen;
  
  return r;
}

/** \brief Reads object, which starts with delimiter character
 * Reads delimited objects (String, Name, Array, Dictionary, Stream)
 * \arg f - input stream to read object
 * \arg alt - alternative end-of-file behavior
 */
/*static*/ Object * Object::read_delimited(std::istream & f, bool alt)
{
  long start=f.tellg();
  char c; f>>c;
  switch(c)
  {
    case '(':
      {
        String * s=new String(read_o_string(f));
        f.ignore();
        return s;
      }
      break;
    case '[':
			{
				Array * a=new Array();
				skip_whitespace(f, alt);
        do {
					Object * o=read(f, alt);
					if(o) a->push(o);
					else if(alt) throw FormatException("EOF inside an array", start);
					skip_whitespace(f, alt);
				} while(f.peek() != ']');
        f.ignore(); // skip ']'
        return a;
      }
      break;
    case '<':
      {
        c=f.get();
        if(c == '<') // second '<' => Dictionary
        {
          Dictionary * r=new Dictionary();
          while(1)
          {
            //std::cout << "Reading dictionary..." << std::endl;
            skip_whitespace(f, alt);
            if(f.peek() == '>')
            {
              f.ignore();
              if(f.peek() == '>')
              {
                f.ignore();
                break;
              }
              else throw FormatException("Invalid dictionary end", f.tellg());
            }
            
            Object * o=read(f, alt);
            if(!o && alt) throw FormatException("EOF instide a dictionary", start);
            
            Name * key=dynamic_cast<Name *>(o);
            if(!key) throw FormatException("Dictionary key is not a Name", start);

            Object * value=read(f, alt);
            if(!o && alt) throw FormatException("EOF instide a dictionary", start);

            //std::cout << "Inserting dict. key=" << key->value() << ", value=" << value->dump() << std::endl;
            r->set(key->value(), value);
            delete key; // string from it copyed in prev. line
          }
//          std::cout << "Checking for stream object in " << r->dump() << std::endl;
          Object * o=r->find("Length");
          if(o)
          {
            // check for stream object
            long savepos=f.tellg();
            skip_whitespace(f, alt);
            std::string s=read_token(f, alt);
            if(s == "stream")
            {
              c=f.get(); if(c=='\x0D') c=f.get();
              if(c!='\x0A') throw FormatException("No newline after 'stream' keyword?", f.tellg());

# if 0  // we no longer read whole stream contents at once
              Integer * intref=dynamic_cast<Integer *>(o);
              if(!intref) throw FormatException("Stream length is not an integer", savepos);
              long length=intref->value();

              std::vector<char> buf;
              buf.resize(length);
              f.read(&buf[0], length);

              skip_whitespace(f, alt);
              s=read_token(f, alt);
              if(s!="endstream") throw FormatException("Invlaid stream end", f.tellg());
              return new Stream(r, buf);
#endif
              return new Stream(r, &f, f.tellg());
            } else f.seekg(savepos);
          }
          return r;
        }
        else // hex string
        {
          std::string s;
	  s+=c; // put back first character, extracted in dict. check
          do {
            skip_whitespace(f, alt);
            s+=read_token(f, alt);
            skip_whitespace(f, alt);
          } while(f.peek() != '>' && f.peek() != EOF);
          f.ignore(); // check for errors?
          if(s.length() % 2) s+='0'; // pad to even length
          std::string r;
          for(unsigned int i=0; i<s.length(); i+=2)
          {
            char c=(hex_to_int(s[i]) << 4) | hex_to_int(s[i+1]);
            r+=c;
          }
          return new String(r);
        }
      }
      break;
    case '/':
      {
        std::string n=read_token(f, alt);
        //std::cout << ". Read name: " << n << std::endl;
        return new Name(n);
      }
      break;
    default:
      std::cerr << "Unhandled delimiter '" << c << "' at offset " << start << std::endl;
      NOT_IMPLEMENTED("Unhandled delimiter");
      break;
  }
  return NULL;
}

//====================== static helper functions =====================

/** \brief Read String from a given stream
 * 
 * \todo add all valid escape sequences
 * \todo implement () couning
 */
static std::string read_o_string(std::istream & f, bool alt)
{
  std::string s; char c; bool escape=false;
  do {
    c=f.get();
    if(c=='\\' && !escape) escape=true;
    else
    {
      if(escape)
      {
        escape=false;
        switch(c)
        {
          case 'n': c='\n'; break;
					case '(': case ')': break;
          default: std::cerr << "Unknown escaped char " << c << std::endl; break;
        }
        s+=c; // XXX
      }
      else
      {
        // XXX handle () counting etc.
        s+=c;
      }
			if(f.peek()==')') break;
    }
  } while(f.peek() != EOF);
//  } while(f.peek() != EOF && (escape || f.peek() != ')'));
//  } while(f.peek() != EOF && f.peek() != ')');
//  if(f.peek() == ')') f.ignore();
  return s;
}

static Object * read_o_digits(std::istream & f, bool alt)
{
  std::string s=read_token(f, alt);
  if(static_cast<int>(s.find_first_of("."))>=0) return new Real(strtod(s.c_str(), NULL)); // real

  long firstint=atol(s.c_str());
  if(s[0] == '-' || s[0] == '+') return new Integer(firstint);

  // look what is there (another number + R => reference), else => integer
  long savepos=f.tellg();
  skip_whitespace(f, alt);

  // check out next token --- must be an integer, and after it 'R'
  s=read_token(f, alt);
  if(s.find_first_not_of("0123456789")>=0) // must be only digits
  {
    long secondint=atol(s.c_str());
    skip_whitespace(f, alt);
    s=read_token(f, alt);
    if(s == "R") // Reference
    {
//std::cerr << "Read Reference (" << firstint << "," << secondint << ")" << std::endl;
      return new ObjRef(firstint, secondint);
    }
  }
  
  // not a reference
  f.seekg(savepos);
  return new Integer(firstint); // integer
}

/// \todo enable keywords reading only when they are allowed
static Object * read_o_chars(std::istream & f, bool alt)
{
  long startpos=f.tellg();
  std::string b=read_token(f);
  if(b == "true") return new Boolean(true);
  if(b == "false") return new Boolean(false);
  if(b == "null") return new Null();
  if(alt)
    return new Keyword(b);
  else
  {
    std::cerr << "invalid boolean (or garbage?) object at offset " << startpos << std::endl;
    return new Null();
  }
}

/// skip whitespace characters in a given stream
inline void skip_whitespace(std::istream & f, bool robust)
{
  while(f.peek()!=EOF
      && (is_a_whitespace(f.peek()) || f.peek()=='\0'))
  {
    f.ignore();
  }
  if(!robust && f.peek() == EOF) throw FormatException("Unexpected EOF");
}

/// read single "word" from stream
static std::string read_token(std::istream & f, bool robust)
{
  std::string s;
  char c;
  do {
    f >> c;
    s+=c;
  } while(! is_a_delimiter(f.peek())
      && !is_a_whitespace(f.peek())
      && f.peek()!='\0'
      && f.peek()!=EOF);
  if(!robust && f.peek() == EOF) throw FormatException("Unexpected EOF");
  return s;
}


/// Fetches unencoded stream data
bool Stream::get_data(std::vector<char> & buf)
{
	unsigned long length;
	Object * o;
	Integer * integer;
	ObjRef * oref;
	if(!(o=dict->find("Length"))) throw FormatException("Stream object without Length attribute", soffset);
	if((oref = dynamic_cast<ObjRef *>(o))) {
		if(!source) throw std::string("Fount ObjRef inside stream dictionary and no source is defined.");
		o = source->load_object(oref->ref());
	}
	if((integer = dynamic_cast<Integer *>(o))) {
		length = integer->value();
		data.resize(length);
		file->seekg(soffset);
		file->read(&data[0], length);

		skip_whitespace(*file, false);
		if(read_token(*file, false) != "endstream") throw FormatException("Invlaid token at stream end", file->tellg());
	} else {
		throw FormatException(std::string("Invalid stream Length type ") + o->type(), soffset);
	}
	if(oref) delete o;

	if(!(o=dict->find("Filter"))) {
		std::cerr << "no filter in stream object" << std::endl;
		buf=data;
		return true;
	}

	std::vector<Filter *> filters;

	Array * a;
	if(( a = dynamic_cast<Array *>(o) )) { // filter chain
		for(Array::ConstIterator it = a->get_const_iterator(); a->check_iterator(it); it++) {
			o = *it;

			Name * n=dynamic_cast<Name *>(o);
			if(!n) throw std::string("Stream filter " + o->dump() + " is not implemented");

			Filter * filter = Filter::Create(n->value());
			if(!filter) throw std::string("Unimplemented stream filter ")+n->value();

			filters.push_back(filter);
		}
	} else { // single filter
		Name * n=dynamic_cast<Name *>(o);
		if(!n) throw std::string("Stream filter " + o->dump() + " is not implemented");

		Filter * filter = Filter::Create(n->value());
		if(!filter) throw std::string("Unimplemented stream filter ")+n->value();

		filters.push_back(filter);
	}

	// pass data thrugh all filters
	
	std::vector<char> *s, *d;
	s = NULL;
	for(unsigned int i = 0; i < filters.size(); i++)
	{
		if(i == filters.size() - 1) d = &buf;
		else d = new std::vector<char>;

		bool r = filters[i]->Decode(i?*s:data, *d);
		if(!r) throw std::string("Stream filter error: ") + filters[i]->Name();

		delete s;
		s = d;
		d = NULL;
	}

	// delete all filters
	for(unsigned int i = 0; i < filters.size(); i++)
		delete filters[i]; 

  return true;
}

} // namespace PDF

