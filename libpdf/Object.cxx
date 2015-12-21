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
  OBJECT_TYPE_ID_TRY(Boolean);
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

static Filter *  CreateStreamFilter(Object * name, Object * params)
{
	Name * n=dynamic_cast<Name *>(name);
	if(!n)
		throw UnimplementedException("Stream filter type ") << name->dump();

	Filter * filter = Filter::Create(n->value(), dynamic_cast<Dictionary *>(params));
	if(!filter) {
		std::string err("Stream filter ");
		err += n->value();
		throw UnimplementedException(err.c_str());
	}
	return filter;
}

/// Fetches unencoded stream data
bool Stream::get_data(std::vector<char> & buf)
{
	m_data.resize(slength());
	ostrm->read_stream_body(soffset, m_data, encryption()?m_id.num:0, encryption()?m_id.gen:0);

	Object * filternode;
	if(!(filternode = m_dict->find("Filter"))) {
		std::clog << "No filter in stream object, strange" << std::endl;
		buf = m_data;
		return true;
	}

	std::vector<Filter *> filters;
#if 0
	SecHandler* sh = ostrm->get_security_handler();
	if(sh) {
		Filter* f = sh->create_stream_filter(m_dict);
		if(f)
			filters.push_back(f);
	}
#endif
	Object * filterparams = m_dict->find("DecodeParms");
	Array * filterarray = dynamic_cast<Array *>(filternode);
	if(filterarray) { // filter chain
		Array * paramsarray = dynamic_cast<Array *>(filterparams);
		for(unsigned int index = 0; index < filterarray->size(); ++index)
			filters.push_back(CreateStreamFilter(filterarray->at(index), paramsarray ? paramsarray->at(index) : NULL));
	} else { // single filter
		filters.push_back(CreateStreamFilter(filternode, filterparams));
	}

	// pass data through all filters
	try {
		std::vector<char> *s, *d;
		s = NULL;
		unsigned int i;
		for(i = 0; i < filters.size(); i++)
		{
			if(i == filters.size() - 1) d = &buf;
			else d = new std::vector<char>;

			bool r = filters[i]->Decode(i?*s:m_data, *d);
			if(!r) {
				std::string err("Stream filter failed: ");
				err += filters[i]->Name();
				throw std::runtime_error(err.c_str());
			}

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

void Stream::put_data(const std::vector<char> & buf)
{
	m_data = buf; // XXX No filters
	dict()->set("Length", new Integer(m_data.size()));
}

void Stream::save_data(ObjOStream * ostrm)
{
	if(! m_data.empty())
		ostrm->write_chunk(&m_data[0], m_data.size(), encryption()?m_id.num:0, encryption()?m_id.gen:0);
}

} // namespace PDF

