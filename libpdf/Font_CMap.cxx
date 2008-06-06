#include "Font_CMap.hpp"

namespace PDF {

inline unsigned long s2c(const std::string & s) {
	unsigned long r=0;
	for(unsigned int i=0; i<s.length(); i++) {
		r<<=8;
		r|=(unsigned char)s[i];
	}
	return r;
}

bool Font::CMap::Range::load(std::istream & s)
{
	int i;
	String * sp[3]={NULL, NULL, NULL};
	Object * o;
	for(i=0; i<3; i++) {
		o=Object::read(s, true);
		if(!o) { std::cerr << "Error loading bfrange" << std::endl; return false; }
		sp[i]=dynamic_cast<String *>(o);
		if(!sp[i]) { std::cerr << "Found non-string in bfrange" << std::endl; return false; }
	}
	begin=s2c(sp[0]->value());
	end=s2c(sp[1]->value());
	res=(wchar_t)s2c(sp[2]->value());
	for(i=0; i<3; i++) delete sp[i];
	return true;
}

bool Font::CMap::load_bfchar(std::istream & s)
{
	Object * o;
	while((o=Object::read(s, true)))
	{
		Keyword * kw=dynamic_cast<Keyword *>(o);
		if(kw && kw->value()=="endbfchar") { delete o; break; }

		String * os1=dynamic_cast<String *>(o);
		if(!os1) { std::cerr << "Unexpected object " << o->type() << " in cmap" << std::endl; delete o; }

		o=Object::read(s, true);
		if(!o) { std::cerr << "Unexpected EOF in cmap" << std::endl; delete os1; return false; }
		String * os2=dynamic_cast<String *>(o);
		if(!os2) { std::cerr << "Unexpected object " << o->type() << " in cmap" << std::endl; delete o; }

		if(os1 && os2)
		{
			std::string s1=os1->value(); delete os1;
			std::string s2=os2->value(); delete os2;
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

bool Font::CMap::load_bfrange(std::istream & s)
{
	Object * o;
	String * str;
	unsigned long rsp[3];
	int i=0;
	while((o=Object::read(s, true)))
	{
		Keyword * kw=dynamic_cast<Keyword *>(o);
		if(kw && kw->value()=="endbfrange") { break; }

		str=dynamic_cast<String *>(o);
		if(!str) { std::cerr << "Found non-string in bfrange" << std::endl; break; }

		rsp[i++]=s2c(str->value());

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

unsigned int Font::CMap::load_codespacerange(std::istream & s)
{
	Object * o;
	String * str;
	unsigned int len = 0;
	while((o=Object::read(s, true)))
	{
		Keyword * kw=dynamic_cast<Keyword *>(o);
		if(kw && kw->value()=="endcodespacerange") { break; }

		str=dynamic_cast<String *>(o);
		if(!str) { std::cerr << "Found non-string in codespacerange" << std::endl; break; }

		if(str->value().length() > len)
			len = str->value().length();
	}
	if(o) delete o;
	return len;
}

bool Font::CMap::load(std::istream & s)
{
  Object * o=reinterpret_cast<Object *>(0x1);
  while(o)
  {
    while((o=Object::read(s, true)))
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
	Stream * map_stream;
	cmapnode.put(map_stream, "CMap is not a Stream Object: ");
	std::vector<char> tv;
	map_stream->get_data(tv);
	std::string str(tv.begin(), tv.end());
	std::stringstream ss(str);
	return load(ss);
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


} // namespace PDF


