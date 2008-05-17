#include "Font.hpp"
#include "OH.hpp"

#include <sstream>


namespace PDF {

inline unsigned long s2c(std::string s) {
	unsigned long r=0;
	for(unsigned int i=0; i<s.length(); i++) {
		r<<=8;
		r|=(unsigned char)s[i];
	}
	return r;
}

bool Font::CMap::Range::load(std::istream & s)
{
	String * sp[3]={NULL, NULL, NULL};
	Object * o;
	for(int i=0; i<3; i++) {
		o=Object::read(s, true);
		if(!o) { std::cerr << "Error loading bfrange" << std::endl; return false; }
		sp[i]=dynamic_cast<String *>(o);
		if(!sp[i]) { std::cerr << "Found non-string in bfrange" << std::endl; return false; }
	}
	begin=s2c(sp[0]->value());
	end=s2c(sp[1]->value());
	res=(wchar_t)s2c(sp[2]->value());
	for(int i=0; i<3; i++) delete sp[i];
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
      std::string str;
      Keyword * kw=dynamic_cast<Keyword *>(o);
			if(kw) {
				if     (kw->value() == "beginbfchar") { load_bfchar(s); }
				else if(kw->value() == "beginbfrange") { load_bfrange(s); }
			}
      delete o;
    }
  }
	maptype=RealMap;
	charwidth=2; /* XXX this depends on begincodespacerange* */
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
			if(! (it==charmap.end())) return (wchar_t)it->second;
			for(unsigned int i=0; i<charranges.size(); i++) {
				if(charranges[i].yours(c)) return charranges[i].map(c);
			}
			return c; /* unmapped */
			}
			break;
		case WinAnsiEncoding:
			switch(c) {
				case 0xA8: return 0x401;
				case 0xB8: return 0x451;
				default:
					if(c<0x7F) return c;
//					printf("WinAnsiEncoding: %04X -> %04X\n", (unsigned int)c, (unsigned int)(0x410 + (c - 0xC0)));
//					return 0x410 + (c - 0xC0); // Convert cp1251 to unicode
					return c;
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
	,defcharwidth(1000)
	,fontflags(0)
{
}

Font::~Font()
{
}

bool Font::load(OH fontnode)
{
  fontnode.expand();
//  std::clog << "Font " << fontname << " Node: " << fontnode.obj()->dump() << std::endl;
	fontobjid = fontnode.obj()->dump_objattr();

	// extract some (useful ?) info
	OH inf;
	inf = fontnode.find("Subtype");
	if(inf) {
		fonttype = inf.strvalue();
	}
	inf = fontnode.find("BaseFont");
	if(inf) {
		basefont = inf.strvalue();
	}
	if((inf = fontnode.find("FontDescriptor"))) {
		inf.expand();
		OH inf2 = inf.find("Flags");
		const Integer * i = inf2.cast<const Integer *>("Font flags must be an integer");
		if(i) fontflags = i->value();
	}

	// extract characters' widths
	OH widths = fontnode.find("Widths");
	if(widths) {
		int firstchar = 0;
		widths.expand();

		OH fc = fontnode.find("FirstChar");
		if(fc) {
			Integer * i = fc.cast<Integer *>("FirstChar is not an integer?!?");
			firstchar = i->value();
		}

		for(unsigned int i = 0; i < widths.size(); i++) {
			OH aeh = widths[i];
			Integer * ip = aeh.cast<Integer *>("Non-integer char width not supported");
			if(ip && ip->value())
				charwidths[i + firstchar] = ip->value();
		}
	}
	
	// Extract parameters of Type0 (Composite) fonts
	if(fonttype == "Type0") {
		OH dscf = fontnode.find("DescendantFonts");
		if(dscf) {
			try {
				load_type0_font_dic(dscf[0]);
			}
			catch(std::string e) {
				std::cerr << "Error loading Type0 font dyctionary: " << e << std::endl;
			}
		}
	}

	// Mess with to-Unicode-mapping
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

std::wstring Font::extract_text(const String * so, double * twid) const
{
	unsigned int cw=to_unicode_map.cbytes();
  std::string s=so->value();
  std::wstring ws;
  if(s.length() % cw) throw std::string("String lenth is not even number");
	if(twid) *twid = 0;
  for(unsigned int i=0; i<s.length(); i+=cw)
  {
    unsigned long c=0;
		for(unsigned int k=0; k<cw; k++) {
			c<<=8;
			c|=(unsigned char)s[i+k];
		}
		if(twid) {
			std::map<int, unsigned long>::const_iterator it = charwidths.find(c);
			if(it != charwidths.end())
				*twid += it->second;
			else
				*twid += defcharwidth;
		}
		wchar_t res=to_unicode_map.map(c);
//printf("Map(%s)(%d): %04X -> %04X\n", fontname.c_str(), cw, (unsigned int)c, (unsigned int)res);
		if(res) ws.push_back(res);
		else {
			fprintf(stderr, "Can not translate char %04lX (font: %s)\n", c, fontname.c_str());
			ws.push_back(0x40);
		}
  }
	if(twid) *twid/=1000; /* font units is 1/1000 of text units */
  return ws;
}

std::string Font::dump() const
{
	const char * ffl[] = {
		"FixedPitch", // bit 1 in reference
		"Serif",
		"Symbolic",
		"Script",
		NULL,
		"Nonsymbolic", // 6
		"Italic",
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		"AllCaps", // 17
		"SmallCaps",
		"ForceBold",
	};
	std::stringstream ss;
	ss << fontobjid << ' ' << fonttype << " " << to_unicode_map.cbytes() << "-byte font, " << charwidths.size() << " char widhs, base: " << basefont;
	ss << ", Flags: " << fontflags;
	if(fontflags) {
		bool comma = false;
		ss << " (";
		for(unsigned int i = 0; i < 8*sizeof(fontflags); i++) {
			if(fontflags & 1<<i) {
				if(comma) ss << ',';
				if(i < sizeof(ffl)/sizeof(ffl[0]) && ffl[i]) {
					ss << ffl[i];
				} else {
					ss << i+1;
				}
				comma = true;
			}
		}
		ss << ")";
	}
	ss << std::endl;
	return ss.str();
}

bool Font::load_type0_font_dic(OH fdic)
{
	fdic.expand();
	OH defw = fdic.find("DW");
	if(defw) {
		Integer * i = defw.cast<Integer *>("Glyph width is not integer");
		defcharwidth = i->value();
	}

	OH w = fdic.find("W");
	if(w) {
		OH h1, h2, h3;
		const Integer *i1, *i2, *i3;
		const Array * a;
		unsigned int index = 0;
		while(index < w.size()) {
			h1 = w[index++];
			i1 = h1.cast<Integer *>("First group element must be an integer");
			if(index >= w.size()) throw std::string("Not enough data");

			h2 = w[index++];
			a = h2.cast<Array *>();
			if(a) {
				for(unsigned int i = 0; i < a->size(); i++) {
					const Object * oo = a->at(i);
					const Integer * cw = dynamic_cast<const Integer *>(oo);
					if(!cw) throw std::string("Char width must be an integer");
					charwidths[i1->value() + i] = cw->value();
				}
//				std::clog << "Charwidths of chars from " << i1->value() << " shuld be read from " << a->dump() << std::endl;
				continue;
			}
			i2 = h2.cast<Integer *>("Second element must be an array or integer");
			if(index >= w.size()) throw std::string("Not enough data");

			h3 = w[index++];
			i3 = h3.cast<Integer *>("Third element must be an integer");

			for(int i = i1->value(); i <= i2->value(); i++) {
//				std::clog << "charwidth for char " << i << " is " << i3->value() << std::endl;
				charwidths[i] = i3->value();
			}

		}
	}
	return true;
}

}; // naespacs PDF


