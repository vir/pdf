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
			Integer * i;
			fc.put(i, "FirstChar is not an integer?!?");
			firstchar = i->value();
		}

		for(unsigned int i = 0; i < widths.size(); i++) {
			OH aeh = widths[i];
			Integer * ip;
			aeh.put(ip, "Non-integer char width not supported");
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

/** Extracts unicode text from a pdf string object.
 * \param so pointer to a pdf string object to work on
 * \param ws reference to a wide string to receive extracted text
 * \param twid reference to a double variable to receive extracted text width
 * \param pos reference to the index variable, containing byte position in a pdf
 *   string object to start extraction. It will be updated upon exit.
 * \param delimiter unicode character to search for. Extraction stops just
 *   before that character and position of _next_ character is returned.
 * \return true if delimiter was found and function has to be called again to
 *   extract the rest of the string, false if whole string content is extracted.
 *
 * Upon exit twid contains extracted string's width (in text space units), pos
 * contains position of the next byte in pdf string object (possible, the
 * position of the end of string if false returned), ws contains extracted
 * string in unicode.
 **/
bool Font::extract_text(const String * so, std::wstring & ws, double & twid, unsigned int & pos, wchar_t delimiter) const
{
	std::string s=so->value();
	if(s.length() % charbytes)
		throw std::string("String length is not a multiple of charbytes: ") + dump();
	twid = 0.0;
	while( pos < s.length() )
	{
		// Get glyph index
		unsigned long c=0;
		for(unsigned int k=0; k<charbytes; k++) {
			c<<=8;
			c|=(unsigned char)s[pos++];
		}

		// Get glyph width
		std::map<int, unsigned long>::const_iterator it = charwidths.find(c);
		twid += (it != charwidths.end()) ? it->second : defcharwidth;

		// Convert to unicode
		wchar_t res = to_unicode(c);
//printf("Map(%s)(%d): %04X -> %04X\n", fontname.c_str(), charbytes, (unsigned int)c, (unsigned int)res);

		// Check for end of token
		// XXX May be we shuld append delimiter to resulting string????
		if(delimiter && res == delimiter) {
			twid/=1000.0; /* font units is 1/1000 of text units */
			return true;
		}

		// Append to result
		if(res) {
			ws += res;
		} else {
			fprintf(stderr, "Can not translate char %04lX (font: %s)\n", c, fontname.c_str());
			ws += L'@';
		}
	}
	twid/=1000.0; /* font units is 1/1000 of text units */
	return false;
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
		Integer * i;
		defw.put(i, "Glyph width is not integer");
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
			if(h2.put(a)) {
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


