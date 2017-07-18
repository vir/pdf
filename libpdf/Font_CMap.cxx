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

#include "Font_CMap.hpp"
#include "ObjStrm.hpp"
#include <stdio.h> // for sprintf in dump()

namespace PDF {

inline unsigned long s2c(const std::string & s) {
	unsigned long r=0;
	for(unsigned int i=0; i<s.length(); i++) {
		r<<=8;
		r|=(unsigned char)s[i];
	}
	return r;
}

bool Font::CMap::Range::load(ObjIStream & s)
{
	int i;
	String * sp[3]={NULL, NULL, NULL};
	Object * o;
	for(i=0; i<3; i++) {
		o = s.read_direct_object();
		if(!o) { std::cerr << "Error loading bfrange" << std::endl; return false; }
		sp[i]=dynamic_cast<String *>(o);
		if(!sp[i]) { std::cerr << "Found non-string in bfrange" << std::endl; return false; }
	}
	begin=s2c(sp[0]->str());
	end=s2c(sp[1]->str());
	res=(wchar_t)s2c(sp[2]->str());
	for(i=0; i<3; i++) delete sp[i];
	return true;
}

bool Font::CMap::load_bfchar(ObjIStream & s)
{
	Object * o;
	while((o = s.read_direct_object()))
	{
		Keyword * kw=dynamic_cast<Keyword *>(o);
		if(kw && kw->value()=="endbfchar") { delete o; break; }

		String * os1=dynamic_cast<String *>(o);
		if(!os1) { std::cerr << "Unexpected object " << o->type() << " in cmap" << std::endl; delete o; }

		o = s.read_direct_object();
		if(!o) { std::cerr << "Unexpected EOF in cmap" << std::endl; delete os1; return false; }
		String * os2=dynamic_cast<String *>(o);
		if(!os2) { std::cerr << "Unexpected object " << o->type() << " in cmap" << std::endl; delete o; }

		if(os1 && os2)
		{
			std::string s1=os1->str(); delete os1;
			std::string s2=os2->str(); delete os2;
#if 0
			std::clog << "s1: " << std::hex << (int)((unsigned char)s1[0]) << " " << std::hex << (int)((unsigned char)s1[1]) << std::endl;
			std::clog << "s2: " << std::hex << (int)((unsigned char)s2[0]) << " " << std::hex << (int)((unsigned char)s2[1]) << std::endl;
#endif
			unsigned long n=((unsigned char)s1[0])<<8|((unsigned char)s1[1]);
			wchar_t c=((unsigned char)s2[0])<<8|((unsigned char)s2[1]);
//			printf("Appending translation from %04X to %04X\n", (unsigned int)n, (unsigned int)c);
			charmap[n]=c;
		}
	}
	return true;
}

bool Font::CMap::load_bfrange(ObjIStream & s)
{
	Object * o;
	String * str;
	unsigned long rsp[3];
	int i=0;
	while((o = s.read_direct_object()))
	{
		Keyword * kw=dynamic_cast<Keyword *>(o);
		if(kw && kw->value()=="endbfrange") { break; }

		str=dynamic_cast<String *>(o);
		if(!str) { std::cerr << "Found non-string in bfrange" << std::endl; break; }

		rsp[i++]=s2c(str->str());

		delete o; o=NULL;

		if(i==3) {
			charranges.push_back(Range(rsp[0], rsp[1], (wchar_t)rsp[2]));
//			printf("Appending range translation from %04X--%04X to %04X--%04X\n", (unsigned int)rsp[0], (unsigned int)rsp[0], (unsigned int)rsp[2], (unsigned int)(rsp[2]+(rsp[1]-rsp[0])));
			i=0;
		}
	}
	if(o) delete o;
	return true;
}

unsigned int Font::CMap::load_codespacerange(ObjIStream & s)
{
	Object * o;
	String * str;
	unsigned int len = 0;
	while((o = s.read_direct_object()))
	{
		Keyword * kw=dynamic_cast<Keyword *>(o);
		if(kw && kw->value() == "endcodespacerange") { delete o;  break; }

		str=dynamic_cast<String *>(o);
		if(!str) { std::cerr << "Found non-string in codespacerange" << std::endl; break; }

		if(str->str().length() > len) // XXX optimize it!!
			len = str->str().length();
		delete o;
	}
	return len;
}

bool Font::CMap::load(ObjIStream & s)
{
  Object * o=reinterpret_cast<Object *>(0x1);
  while(o)
  {
    while((o = s.read_direct_object()))
    {
      std::string str;
      Keyword * kw=dynamic_cast<Keyword *>(o);
			if(kw) {
				if     (kw->value() == "begincodespacerange") { charbytes = load_codespacerange(s); }
				else if(kw->value() == "beginbfchar") { load_bfchar(s); }
				else if(kw->value() == "beginbfrange") { load_bfrange(s); }
			}
      delete o;
    }
  }
  return true;
}

bool Font::CMap::load(OH cmapnode)
{
	if(!cmapnode)
		return false;
	Name * name;
	if(cmapnode.put(name)) {
		return load(name->value());
	}
	Stream * map_stream;
	cmapnode.put(map_stream, "CMap is not a Stream Object: ");
	std::vector<char> tv;
	map_stream->get_data(tv);
	std::string str(tv.begin(), tv.end());
	std::stringstream ss(str);
	ObjIStream strm(ss);
	strm.throw_eof(false);
	strm.allow_keywords(true);
	return load(strm);
}

/// Load named CMap
bool Font::CMap::load(const std::string & s)
{
	if(s == "Identity-H" || s == "Identity-V") {
		charmap.clear();
		charranges.clear();
		charbytes = 2;
		m_identity = true;
		return true;
	} else
		throw UnimplementedException("Named CMaps (other than Identity)");
}

wchar_t Font::CMap::map(unsigned long c, bool no_fallback) const
{
	std::map<long,wchar_t>::const_iterator it=charmap.find(c);
	if(! (it==charmap.end())) return it->second;

	for(unsigned int i=0; i<charranges.size(); i++)
		if(charranges[i].yours(c))
			return charranges[i].map(c);
	return no_fallback?L'\0':(wchar_t)c; /* unmapped */
}

std::string Font::CMap::dump() const
{
	if(m_identity)
		return "Identity";
	std::stringstream ss;
	for(std::map<long,wchar_t>::const_iterator it=charmap.begin(); it!=charmap.end(); ++it)
	{
//            ss << "Map " << it->first << " to " << it->second << std::endl;
		char buf[1024];
		sprintf(buf, "Map %04lX to %04X\n", it->first, it->second);
		ss << buf;
	}
	return ss.str();
}

} // namespace PDF


