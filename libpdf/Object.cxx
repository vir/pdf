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
  OBJECT_TYPE_ID_TRY(Object);
  return "Somthing strange";
}
#undef OBJECT_TYPE_ID_TRY

/// Fetches unencoded stream data
bool Stream::get_data(std::vector<char> & buf)
{
	Object * o;

	ostrm->load_stream_data(this);

	if(!(o = dict->find("Filter"))) {
		std::cerr << "no filter in stream object" << std::endl;
		buf = m_data;
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

