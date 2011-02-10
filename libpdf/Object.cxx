// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include <iostream>
#include <string>
#include <cstring>
#include <map>

//#include "PDF.hpp"
#include "Object.hpp"
#include "Exceptions.hpp"
#include "Filter.hpp" // for stream
#include "ObjStrm.hpp"
#include "File.hpp" // for File::load_object
#include <string.h>

namespace PDF {

unsigned int Object::m_debug=0;

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
  OBJECT_TYPE_ID_TRY(FreeObjectPlaceholder);
  OBJECT_TYPE_ID_TRY(MissingObjectPlaceholder);
  OBJECT_TYPE_ID_TRY(Object);
  return "Somthing strange";
}
#undef OBJECT_TYPE_ID_TRY

/// Fetches stream's source file length
unsigned int Stream::slength() const
{
	if(m_slength_ref) {
		Integer * ii;
		if(!source)
			throw FormatException("Stream length is a reference and have no source - giving up!", soffset);
		Object * o = source->load_object(m_slength_ref->ref());
		ii = dynamic_cast<Integer *>(o);
		if(!ii)
			throw FormatException("Referenced Stream length is not an integer - giving up #@$#%!", soffset);
		m_slength = ii->value();
		delete o;
	}
	m_slength_ref = NULL;
	return m_slength;
}

/// Fetches unencoded stream data
bool Stream::get_data(std::vector<char> & buf, bool decrypt)
{
	Object * o;

	m_data.resize(slength());
	ostrm->read_chunk(soffset, &m_data[0], m_data.size(), decrypt?m_id.num:0, decrypt?m_id.gen:0);

	if(!(o = dict->find("Filter"))) {
		std::cerr << "no filter in stream object" << std::endl;
		buf = m_data;
		return true;
	}

	Object * po = dict->find("DecodeParms");
	std::vector<Filter *> filters;

	Array * a;
	if(( a = dynamic_cast<Array *>(o) )) { // filter chain
		Array * pa = dynamic_cast<Array *>(po);
		for(unsigned int index = 0; index < a->size(); ++index) {
			o = a->at(index);
			Dictionary * params = dynamic_cast<Dictionary *>(pa?pa->at(index):NULL);

			Name * n=dynamic_cast<Name *>(o);
			if(!n) throw std::string("Stream filter " + o->dump() + " is not implemented");

			Filter * filter = Filter::Create(n->value(), params);
			if(!filter) throw std::string("Unimplemented stream filter ")+n->value();

			filters.push_back(filter);
		}
	} else { // single filter
		Name * n=dynamic_cast<Name *>(o);
		if(!n) throw std::string("Stream filter " + o->dump() + " is not implemented");

		Dictionary * params = dynamic_cast<Dictionary *>(po);

		Filter * filter = Filter::Create(n->value(), params);
		if(!filter) throw std::string("Unimplemented stream filter ")+n->value();

		filters.push_back(filter);
	}

	// pass data thrugh all filters
	
	try {
		std::vector<char> *s, *d;
		s = NULL;
		unsigned int i;
		for(i = 0; i < filters.size(); i++)
		{
			if(i == filters.size() - 1) d = &buf;
			else d = new std::vector<char>;

			bool r = filters[i]->Decode(i?*s:m_data, *d);
			if(!r) throw std::string("Stream filter error: ") + filters[i]->Name();

			delete s;
			s = d;
			d = NULL;
		}

		// delete all filters
		for(i = 0; i < filters.size(); i++)
			delete filters[i]; 

		return true;
	}
	catch(std::string s) {
		for(unsigned int i = 0; i < filters.size(); i++)
			delete filters[i]; 
		throw FormatException(s, soffset);
	}
}

} // namespace PDF

