/** \file Crypto.hpp
 * Header file with cryptography-related classes
 **/
#ifndef CRYPTO_HPP_INCLUDED
#define CRYPTO_HPP_INCLUDED

#include <stdint.h>
#include <string>

typedef unsigned char * POINTER;

/** Message-Digest algorithm 5
 *
 * (from rfc1321): The algorithm takes as input a message of arbitrary length
 * and produces as output a 128-bit "fingerprint" or "message digest" of the
 * input. It is conjectured that it is computationally infeasible to produce
 * two messages having the same message digest, or to produce any message
 * having a given prespecified target message digest.
 *
 * Full description: http://tools.ietf.org/html/rfc1321
 **/
class MD5
{
	private:
		const static unsigned char padding[64];
		uint32_t state[4]; /* state (ABCD) */
		uint32_t count[2]; /* number of bits, modulo 2^64 (lsb first) */
		unsigned char buffer[64]; /* input buffer */
	protected:
		void transform(const unsigned char block[64]);
	public:
		void init();
		void update(const unsigned char *input, unsigned int inputLen);
		void update(const char *input, unsigned int inputLen) { update((const unsigned char *)input, inputLen); }
		void final(unsigned char digest[16]);
};

class RC4 // Description: http://en.wikipedia.org/wiki/RC4
{
	private:
		unsigned char s[256];
		unsigned int i, j;
	protected:
		void do_swap()
		{
			unsigned char t = s[i];
			s[i] = s[j];
			s[j] = t;
		}
		unsigned char prga()
		{
			i++; i &= 0xFF; // i := (i + 1) mod 256
			j = (j + s[i]) & 0xFF; // j := (j + S[i]) mod 256
			do_swap();
			return s[(s[i] + s[j])&0xFF]; // output S[(S[i] + S[j]) mod 256]
		}
	public:
		RC4():i(0),j(0) {}
		void init(std::string key)
		{
			for(i = 0; i < 256; i++)
				s[i] = i;
			if(!key.length())
				return;
			for(i = 0, j = 0; i < 256; i++) {
				j = (j + (unsigned int)s[i] + (unsigned int)(unsigned char)key[i % key.length()]) & 0xFF;
				do_swap(); // swap s[i], j[j]
			}
			i = 0; j = 0; // prepare for extraction
		}
		unsigned char transform(unsigned char b)
		{
			return b ^ prga();
		}
		std::string transform(std::string data)
		{
			std::string r;
			r.reserve(data.length());
			unsigned int k;
			for(k = 0; k < data.length(); k++)
				r += transform(data[k]);
			return r;
		}
};

#endif /* CRYPTO_HPP_INCLUDED */

