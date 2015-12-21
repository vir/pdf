/** \file Crypto.hpp
 * Header file with cryptography-related classes
 **/
#ifndef CRYPTO_HPP_INCLUDED
#define CRYPTO_HPP_INCLUDED

#ifdef _MSC_VER
# include <wtypes.h>
# define uint32_t DWORD
# define int32_t LONG
#else
# include <stdint.h>
#endif
#include <string>
#include <vector>
#include "Exceptions.hpp"
#include <assert.h>

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
		MD5() { init(); }
		void init();
		void update(const unsigned char *input, unsigned int inputLen);
		void update(const char *input, unsigned int inputLen) { update((const unsigned char *)input, inputLen); }
		void update(const std::string & s) { update((const unsigned char *)s.c_str(), s.length()); }
		void final(unsigned char digest[16]);
		std::string final() { unsigned char digest[16]; final(digest); return std::string((char*)digest, sizeof(digest)); }
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
		void init(const std::string & key)
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
		void transform(char * buf, int len)
		{
			for(int i = 0; i < len; i++) {
				buf[i] = transform(buf[i]);
			}
		}
		std::string transform(const std::string & data)
		{
			std::string r;
			r.reserve(data.length());
			unsigned int k;
			for(k = 0; k < data.length(); k++)
				r += transform(data[k]);
			return r;
		}
		static std::string transform(const std::string & key, const std::string & data)
		{
			RC4 rc4;
			rc4.init(key);
			return rc4.transform(data);
		}
};

class AES
{
private:
	bool encrypt;
	int keybits;
	unsigned long* rk;
	int nrounds;
public:
	AES(bool encrypt, int keybits = 128):encrypt(encrypt), keybits(keybits), rk(NULL), nrounds(0) {}
	~AES();
	void init(const std::string & key);
	void transform(const char * source_16_bytes, char * result_16_bytes);
	std::string transform(const std::string & data)
	{
		if(data.size() != 16)
			return "";
		std::string r;
		char buf[16];
		transform(data.c_str(), buf);
		return std::string(buf, sizeof(buf));
	}
	static inline std::string transform(bool encr, const std::string & key, const std::string & data, int bits = 128)
	{
		AES aes(encr, bits);
		aes.init(key);
		return aes.transform(data);
	}
};

class AES_CBC
{
	const static unsigned int blocksize = 16;
public:
	AES_CBC(const char* iv, bool encr, std::string key, unsigned int blocksize = 16)
		: aes(encr, 8 * key.size())
		, encr(encr)
	{
		::memcpy(buf, iv, blocksize);
		aes.init(key);
	}
	void transform_one_block(const char * source, char * result)
	{
		if(encr) {
			xor(buf, source);
			aes.transform(buf, buf);
			::memcpy(result, buf, sizeof(buf));
		} else {
			aes.transform(source, result);
			xor(result, buf);
			memcpy(buf, source, sizeof(buf));
		}
	}
	static void xor(char* dst, const char* src)
	{
		for(size_t i = 0; i < blocksize; ++i)
			dst[i] ^= src[i];
	}
	static void encrypt(std::string key, std::vector<char>& buf)
	{
		NOT_IMPLEMENTED("AES_CBC::encrypt");
	}
	static void decrypt(std::string key, std::vector<char>& buf)
	{
		assert(buf.size() > blocksize);
		AES_CBC a(&buf[0], false, key);
		const char * read = &buf[blocksize];
		char * write = &buf[0];
		size_t remaining = buf.size() - blocksize;
		while(remaining >= blocksize) {
			a.transform_one_block(read, write);
			read += blocksize;
			write += blocksize;
			remaining -= blocksize;
		}
		assert(0 == remaining);
		// remove padding
		--write;
		size_t padding = (size_t)*(unsigned char*)write; // get last byte
		assert(padding > 0 && padding <= blocksize);
		read = write - padding + 1; // points at padding beginning
		size_t new_length = read - &buf[0];
#ifdef DEBUG
		for(; read < write; ++read) {
			assert(*read == *write);
		}
#endif
		buf.resize(new_length);
	}
private:
	AES aes;
	char buf[blocksize];
	bool encr;
};

#endif /* CRYPTO_HPP_INCLUDED */

