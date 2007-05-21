#include "Font.hpp"
#include "OH.hpp"

#include <sstream>


namespace PDF {

bool Font::CMap::load(std::istream & s)
{
  // search for 'begincodespacerange'
  // check '0000','FFFF'
  Object * o=reinterpret_cast<Object *>(0x1);
  while(o)
  {
    // search for 'beginbfchar'
    while((o=Object::read(s, true)))
    {
      std::string s;
      Keyword * kw=dynamic_cast<Keyword *>(o);
      if(kw) s=kw->value();
      delete o;
      if(s == "beginbfchar") break;
    }
    //  load 'XXXX' 'XXXX'
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
        printf("Appending translation from %04X to %04X\n", (unsigned int)n, (unsigned int)c);
        charmap[n]=c;
      }
    }
  }
	maptype=RealMap;
  // until 'endbfchar'
  return true;
}

bool Font::CMap::set_encoding(const std::string & enc)
{
	if(enc == "WinAnsiEncoding") { maptype=WinAnsiEncoding; charwidth=1; return true; }
	if(enc == "Identity-H") { maptype=IdentityH; charwidth=2; }
	return false;
}

wchar_t Font::CMap::map(unsigned long c) const
{
	switch(maptype) {
		case IdentityH:
		case RealMap:
			{
			std::map<long,wchar_t>::const_iterator it=charmap.find(c);
//			if(it == charmap.end())
//			{
//				fprintf(stderr, "Can not translate char %04lX\n", c);
//	//    std::cerr << "Unmapped character " << c << " in cmap" << std::endl;
//			}
			return (it==charmap.end())?(wchar_t)c:it->second;
			}
			break;
		case WinAnsiEncoding:
			switch(c) {
				case 0xA8: return 0x401;
				case 0xB8: return 0x451;
				default: if(c<0x7F) return c; printf("WinAnsiEncoding: %04X -> %04X\n", (unsigned int)c, (0x410 + (c - 0xC0))); return 0x410 + (c - 0xC0);
			}
			break;
		default:
//			std::cerr << "Unsupported font encoding" << std::endl;
			return c;
			break;
	}
}


//========================================================================



Font::Font(std::string name)
	:fontname(name)
{
}

Font::~Font()
{
}

bool Font::load(OH fontnode)
{
  fontnode.expand();
  std::clog << "Font " << fontname << " Node: " << fontnode.obj()->dump() << std::endl;
  OH ucmap=fontnode.find("ToUnicode");
	if(ucmap) {
		ucmap.expand();
		Stream * map_stream=ucmap.cast<Stream *>("CMap is not a Stream Object");
		std::vector<char> tv;
		map_stream->get_data(tv);
		std::string str(tv.begin(), tv.end());
		std::stringstream ss(str);
		return to_unicode_map.load(ss);
	}

	OH fontenc=fontnode.find("Encoding");
	if(fontenc) {
		Name * s = fontenc.cast<Name *>("Font encoding is not a name");
		if(to_unicode_map.set_encoding(s->value())) {
			return true;
		} else {
			std::cerr << "Unknown font encoding: " << s->value() << std::endl;
		}
	}

  std::cerr << "Problems with font encoding: No 'ToUnicode' and unknown font encoding." << std::endl;
	return false;
}

wchar_t Font::to_unicode(int c) const
{
  return to_unicode_map.map(c);
}

std::wstring Font::extract_text(const String * so) const
{
	unsigned int cw=to_unicode_map.cbytes();
  std::string s=so->value();
  std::wstring ws;
  if(s.length() % cw) throw std::string("String lenth is not even number");
  for(unsigned int i=0; i<s.length(); i+=cw)
  {
    unsigned long c=0;
		for(unsigned int k=0; k<cw; k++) {
			c<<=9;
			c|=(unsigned char)s[i+k];
		}
		wchar_t res=to_unicode_map.map(c);
		if(res) ws.push_back(res);
		else {
			fprintf(stderr, "Can not translate char %04lX (font: %s)\n", c, fontname.c_str());
			ws.push_back(0x40);
		}
  }
  return ws;
}


}; // naespacs PDF


