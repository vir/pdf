#include "Font.hpp"
#include "Font_Encoding.hpp"
#include "Font_CMap.hpp"
#include "OH.hpp"

#include <sstream>


namespace PDF {


/* ======================================================================= */


//========================================================================

Font::Font(std::string name)
	:encoding(NULL)
	,to_unicode_map(NULL)
	,fontname(name)
	,defcharwidth(1000)
	,fontflags(0)
	,charbytes(1)
{
}

Font::~Font()
{
	delete to_unicode_map;
	delete encoding;
}

bool Font::load(OH fontnode)
{
	fontnode.expand();
	fontobjid = fontnode.obj()->dump_objattr();

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
		OH inf2 = inf.find("Flags");
		const Integer * i;
		inf2.put(i, "Font flags must be an integer, not ");
		if(i) fontflags = i->value();
	}

	// extract characters' widths
	OH widths = fontnode.find("Widths");
	if(widths) {
		int firstchar = 0;

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
				std::cerr << "Error loading Type0 font dictionary: " << e << std::endl;
			}
		}
	}

	OH ucmap=fontnode.find("ToUnicode");
	if(ucmap) {
		if(!to_unicode_map)
			to_unicode_map = new Font::CMap;
		to_unicode_map->load(ucmap);
		charbytes = to_unicode_map->cbytes();
	}

	OH enc=fontnode.find("Encoding");
	if(enc) {
		if(!encoding)
			encoding = new Font::Encoding;
		encoding->load(enc);
	}

	if(encoding || to_unicode_map)
		return true;

	std::cerr << "Problems with font encoding: No 'ToUnicode' and unknown/unsupported font encoding." << std::endl;
	return false;
}

wchar_t Font::to_unicode(int c) const
{
  wchar_t r = 0;
	if(to_unicode_map)
		r = to_unicode_map->map(c, true);
	if(encoding && !r)
		r = encoding->map(c);
	return r;
}

std::wstring Font::extract_text(const String * so, double * twid) const
{
  std::string s=so->value();
  std::wstring ws;
  if(s.length() % charbytes)
		throw std::string("String length is not a multiple of charbytes: ") + dump();
	if(twid) *twid = 0;
  for(unsigned int i=0; i<s.length(); i+=charbytes)
  {
    unsigned long c=0;
		for(unsigned int k=0; k<charbytes; k++) {
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
		wchar_t res=to_unicode(c);
//printf("Map(%s)(%d): %04X -> %04X\n", fontname.c_str(), charbytes, (unsigned int)c, (unsigned int)res);
		if(res) ws += res;
		else {
			fprintf(stderr, "Can not translate char %04lX (font: %s)\n", c, fontname.c_str());
			ws += L'@';
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
	ss << fontobjid << ' ' << fonttype << " " << charbytes << "-byte font, " << charwidths.size() << " char widhs, base: " << basefont;
	if(encoding)
		ss << ", Encoding: " << encoding->name();
	if(to_unicode_map)
		ss << ", Has ToUnicodeMap";
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
			h1.put(i1, "First group element must be an integer, not a ");
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
			h2.put(i2, "Second element must be an array or integer, not a ");
			if(index >= w.size()) throw std::string("Not enough data");

			h3 = w[index++];
			h3.put(i3, "Third element must be an integer, not a ");

			for(int i = i1->value(); i <= i2->value(); i++) {
//				std::clog << "charwidth for char " << i << " is " << i3->value() << std::endl;
				charwidths[i] = i3->value();
			}

		}
	}
	return true;
}

}; // namespace PDF


