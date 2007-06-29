
#include <vector>
#include <string>

#include "Filter.hpp"

namespace PDF {

const static char * Whitespace="\x09\x0A\x0C\x0D\x20"; // plus '0x00' char!

/// Converts binary data to a base85-encoded text
/**
 * \todo Implement it!
 * \param src should contain data to be converted
 * \param dst receives base85 text
 */
bool Base85Filter::Encode(const std::vector<char> & src, std::vector<char> & dst)
{
  return false;
}

class b85dh
{
	private:
		unsigned long q;
		int ql;
	public:
		b85dh():q(0),ql(0) {}
		bool add(char c)
		{
			if(c == 'z') {
				if(ql) throw std::string("Base85 decode failed: found 'z' inside group");
				q = 0;
				ql = 5;
				return true;
			}
			q*=85;
			q+=(unsigned char)(c - '!');
			ql++;
			if(ql == 5) {
				return true;
			}
			return false;
		}
		void flush(std::vector<char> & v)
		{
			int i;
			if(ql < 5) { // append 'v' and pad to 5 chars
				q*=85;
				q+='v' - '!';
				for(i = ql+1; i < 5; i++, q*=85) ;
			}
			for(ql-=2, i = 24; ql >= 0; ql--, i-=8) {
				v.push_back(q>>i);
			}
			q = 0; ql = 0;
		}
};

/// Decodes base85-encoded text into binary data block
/**
 * \param src should contain base85 encoded text
 * \param dst receives decoded data
 */
bool Base85Filter::Decode(const std::vector<char> & src, std::vector<char> & dst)
{
	unsigned int pos = 0;
	b85dh h;
	dst.reserve(src.size()*4/5);
	while(1) {
		while(std::strchr(Whitespace,src[pos]) && pos<src.size()) pos++;
		if(src[pos] == '~' && src[pos + 1] == '>') {
			h.flush(dst);
			break;
		}
		if(h.add(src[pos])) {
			h.flush(dst);
		}
		pos++;
	}
  return true;
}

}; // namespace PDF

