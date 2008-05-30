//#include "Font.hpp"
#include "Font_Encoding.hpp"

namespace PDF {

static struct {
	const char * n;
	wchar_t c;
} charnames_table[] = {
	{"space",    L' '},
	{"zero",     L'0'},
	{"one",      L'1'},
	{"two",      L'2'},
	{"three",    L'3'},
	{"four",     L'4'},
	{"five",     L'5'},
	{"six",      L'6'},
	{"seven",    L'7'},
	{"eight",    L'8'},
	{"nine",     L'9'},
	{"period",   L'.'},
	{"comma",    L','},
	{"parenleft", L'('},
	{"parenright", L')'},
	{"hyphen",   L'-'},
	{"colon",    L':'},
	{"semicolon", L';'},
	{"slash",    L'/'},
	{"equal",    L'='},
	{"plus",     L'+'},
	{"asterisk", L'*'},
};

static wchar_t get_unicode_char_by_name(std::string name)
{
	if(name.length() == 1) {
		return (wchar_t)name[0];
	}
	for(unsigned int i = 0; i < sizeof(charnames_table)/sizeof(charnames_table[0]); i++) {
		if(name == charnames_table[i].n)
			return charnames_table[i].c;
	}
	std::cerr << "Unknown char name <<" << name << ">>" << std::endl;
	return (wchar_t)0;
}



struct TypeTabEntry { enum Font::Encoding::Type t; const char * n; } encodings_tab[] = {
#define C(c) { Font::Encoding::c, #c }
	C(UnknownEncoding),
	C(WinAnsiEncoding),
	C(MacRomanEncoding),
	C(MacExpertEncoding),
	{ Font::Encoding::IdentityH, "Identity-H" },
#undef C
};

/** Returns name of the encoding
 * \return name of the encoding
 **/
const char * Font::Encoding::name() const
{
	for(unsigned int i = 0; i < sizeof(encodings_tab)/sizeof(encodings_tab[0]); i++) {
		if(encodings_tab[i].t == enc)
			return encodings_tab[i].n;
	}
	return NULL;
}

/** Sets encoding, specified by name
 * \param n name of encoding to set
 **/
bool Font::Encoding::set_encoding(const std::string & n)
{
	for(unsigned int i = 0; i < sizeof(encodings_tab)/sizeof(encodings_tab[0]); i++) {
		if(n == encodings_tab[i].n) {
			enc = encodings_tab[i].t;
			return true;
		}
	}
	return false;
}

/** Load font encoding information from a PDF file
 * \param encnode Encoding node from a font dictionary
 **/
void Font::Encoding::load(OH encnode)
{
	if(!encnode)
		return;
	Name * s = dynamic_cast<Name *>(encnode.obj());
	Dictionary * d = dynamic_cast<Dictionary *>(encnode.obj());
	if(!s && !d)
		throw std::string("Font encoding is not a name nor dictionary but is a ") + encnode.obj()->type();

	if(s) { // just encoding name
		if(! set_encoding(s->value()))
			throw std::string("Unknown or unhandled font encoding: ") + s->value();
	} else { // dictionary with differences
		/* Todo: set BaseEncoding, load Differences array */
		Name * be;
		encnode.find("BaseEncoding").put(be);
		OH diffs = encnode.find("Differences");
		if(be) {
			if(! set_encoding(be->value()))
				std::cerr << "Unknown BaseEncoding encoding: " << s->value() << std::endl;
		}
		if(diffs) {
			unsigned int i;
			int j = 0;
			std::clog << "Diffs:";
			for(i = 0; i < diffs.size(); i++) {
				OH d = diffs[i];
				Integer * iii; Name * nnn;
				if(d.put(iii)) {
					j = iii->value();
				} else if(d.put(nnn)) {
					add_diff(j, get_unicode_char_by_name(nnn->value()));
					j++;
				}
			}
			std::clog << std::endl;
		} // diffs
	} // name/dic
}

wchar_t Font::Encoding::map(unsigned long c) const
{
	// first, check diffs
	std::map<long,wchar_t>::const_iterator it=charmap.find(c);
	if(! (it==charmap.end())) return (wchar_t)it->second;

	// then, chech encoding
	switch(enc) {
		case IdentityH:
			return (wchar_t)c; /* unmapped */
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


} // namespace PDF

