#include "Crypto.hpp"

/*===== MD5 implementation ====*/

#include <string.h> // XXX for memcpy an so

/* F, G, H and I are basic MD5 functions. */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits. */ // XXX only for 32bit archs
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4. Rotation is separate from addition to prevent recomputation. */
#define FF(a, b, c, d, x, s, ac) { \
	(a) += F ((b), (c), (d)) + (x) + (uint32_t)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define GG(a, b, c, d, x, s, ac) { \
	(a) += G ((b), (c), (d)) + (x) + (uint32_t)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) { \
	(a) += H ((b), (c), (d)) + (x) + (uint32_t)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define II(a, b, c, d, x, s, ac) { \
	(a) += I ((b), (c), (d)) + (x) + (uint32_t)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

const unsigned char MD5::padding[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static void Encode(unsigned char *, const uint32_t *, unsigned int);
static void Decode(uint32_t *, const unsigned char *, unsigned int);


/** MD5 initialization.
 *
 * Begins an MD5 operation, writing a new context.
 */
void MD5::init()
{
	/* Zeroize sensitive information. */
	memset (this, 0, sizeof (*this)); // XXX

	state[0] = 0x67452301;
	state[1] = 0xefcdab89;
	state[2] = 0x98badcfe;
	state[3] = 0x10325476;
}

/** MD5 block update operation.
 *
 * Continues an MD5 message-digest
 * operation, processing another message block, and updating the
 * context.
 */
void MD5::update(const unsigned char *input, unsigned int inputLen)
{
	unsigned int i, index, partLen;
	/* Compute number of bytes mod 64 */
	index = (unsigned int)((count[0] >> 3) & 0x3F);
	/* Update number of bits */
	if ((count[0] += ((uint32_t)inputLen << 3)) < ((uint32_t)inputLen << 3))
		count[1]++;
	count[1] += ((uint32_t)inputLen >> 29);

	partLen = 64 - index;

	/* Transform as many times as possible. */
	if (inputLen >= partLen) {
		memcpy((POINTER)&buffer[index], (POINTER)input, partLen);
		transform(buffer);

		for (i = partLen; i + 63 < inputLen; i += 64)
			transform(&input[i]);
		index = 0;
	}
	else
		i = 0;

	/* Buffer remaining input */
	memcpy((POINTER)&buffer[index], (POINTER)&input[i], inputLen-i);
}

/** MD5 finalization.
 *
 * Ends an MD5 message-digest operation, writing the
 * the message digest and zeroizing the context.
 */
void MD5::final(unsigned char digest[16])
{
	unsigned char bits[8];
	unsigned int index, padLen;

	/* Save number of bits */
	Encode(bits, count, 8);

	/* Pad out to 56 mod 64. */
	index = (unsigned int)((count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	update(padding, padLen);

	/* Append length (before padding) */
	update(bits, 8);
	/* Store state in digest */
	Encode(digest, state, 16);
}

/** MD5 basic transformation. Transforms state based on block. */
void MD5::transform(const unsigned char block[64])
{
	uint32_t a = state[0], b = state[1], c = state[2], d = state[3], x[16];

	Decode (x, block, 64);

	/* Round 1 */
	FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
	FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
	FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
	FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
	FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
	FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
	FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
	FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
	FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
	FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
	FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

	/* Round 2 */
	GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
	GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
	GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
	GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
	GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
	GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
	GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
	GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */

	GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
	GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
	GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
	GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

	/* Round 3 */
	HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
	HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
	HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
	HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
	HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
	HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
	HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
	HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
	HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
	HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

	/* Round 4 */
	II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
	II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
	II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
	II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
	II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
	II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
	II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
	II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
	II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
	II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;

	/* Zeroize sensitive information. */
	memset((POINTER)x, 0, sizeof (x));
}

/* Encodes input (UINT4) into output (unsigned char). Assumes len is a multiple of 4. */
static void Encode(unsigned char *output, const uint32_t *input, unsigned int len)
{
	unsigned int i, j;

	for (i = 0, j = 0; j < len; i++, j += 4) {
		output[j] = (unsigned char)(input[i] & 0xff);
		output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
		output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
		output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
	}
}

/* Decodes input (unsigned char) into output (UINT4). Assumes len is a multiple of 4. */
static void Decode(uint32_t *output, const unsigned char *input, unsigned int len)
{
	unsigned int i, j;

	for (i = 0, j = 0; j < len; i++, j += 4)
		output[i] = ((uint32_t)input[j]) | (((uint32_t)input[j+1]) << 8) |
			(((uint32_t)input[j+2]) << 16) | (((uint32_t)input[j+3]) << 24);
}



/* ============= AES ============= */
#include "AES.h"
#include <stdexcept>

void AES::init(const std::string & key)
{
	rk = new unsigned long[RKLENGTH(keybits)];
	if(key.length() != KEYLENGTH(keybits))
		throw std::logic_error("Bad key length");
	nrounds = encrypt
		? rijndaelSetupEncrypt(rk, reinterpret_cast<const unsigned char*>(key.c_str()), keybits)
		: rijndaelSetupDecrypt(rk, reinterpret_cast<const unsigned char*>(key.c_str()), keybits);
}

AES::~AES()
{
	delete[] rk;
}

void AES::transform(const unsigned char * source_16_bytes, unsigned char * result_16_bytes)
{
	if(encrypt)
		rijndaelEncrypt(rk, nrounds, source_16_bytes, result_16_bytes);
	else
		rijndaelDecrypt(rk, nrounds, source_16_bytes, result_16_bytes);
}

#ifdef TEST_MAIN
#include <iostream>
#include <sstream>
//#include <iomanip>

using namespace std;

string hexstr(const unsigned char * b, unsigned int l)
{
#define HEXB(c) (((c)<10)?('0'+(c)):('a'+((c)-10)))
	unsigned int i;
	string r;
	r.reserve(l*2);
	for(i = 0; i < l; i++) {
		r += HEXB((b[i]>>4)&0x0F);
		r += HEXB(b[i]&0x0F);
	}
#if 0
	for(p = 0; p < result.length(); p++)
		cout << hex << setw(2) << setfill('0') << (unsigned int)(unsigned char)result[p];
#endif
	return r;
#undef HEXB
}
inline string hexstr(const string & s)
{
	return hexstr((const unsigned char *)s.c_str(), s.length());
}

string unhexstr(const char* hex)
{
	ostringstream ss;
	while(*hex) {
#define UNHEXB(c) ( \
		((c) >= '0' && (c) <= '9') ? ((c) - '0') : \
		((c) >= 'A' && (c) <= 'F') ? ((c) - 'A' + 10) : \
		((c) >= 'a' && (c) <= 'f') ? ((c) - 'a' + 10) : \
	0)
		char c = UNHEXB(*hex) << 4;
		++hex;
		if(! *hex)
			break;
		c |= UNHEXB(*hex);
		++hex;
		ss << c;
	}
	return ss.str();
}

struct testdata
{
	const char * key;
	const char * data;
	const char * correct;
};

/*== test MD5 ==*/

static struct testdata td_md5[] = {
	{ NULL, "", "d41d8cd98f00b204e9800998ecf8427e" },
	{ NULL, "a", "0cc175b9c0f1b6a831c399e269772661" },
	{ NULL, "The quick brown fox jumps over the lazy dog", "9e107d9d372bb6826bd81d3542a419d6" },
	{ NULL, "The quick brown fox jumps over the lazy dog.", "e4d909c290d0fb1ca068ffaddf22cbd0" },
};

int test_md5()
{
	unsigned int t, p;
	int r = 0;
	MD5 md5;
	unsigned char digest[16];

	cout << endl << "Testing MD5 hash function..." << endl;
	for(t = 0; t < sizeof(td_md5)/sizeof(td_md5[0]); t++) {
		struct testdata * td = &td_md5[t];
		md5.init();
		md5.update(td->data, strlen(td->data));
		md5.final(digest);
		string result = hexstr(digest, sizeof(digest));
		cout << "Test " << t << ". data: " << td->data << endl;
		cout << " Result:  " << result << endl;
		cout << " Shuld be " << td->correct << endl;
		if(result == td->correct)
			cout << " `---> test " << t << " OK" << endl;
		else {
			cout << "!!!!!! test " << t << " FAILED !!!!!" << endl;
			r++;
		}
	}
	return r;
}

/*== test RC4 ==*/

static struct testdata td_rc4[] = {
	{ "Key", "Plaintext", "bbf316e8d940af0ad3" },
	{ "Wiki", "pedia", "1021bf0420" },
	{ "Secret", "Attack at dawn", "45a01f645fc35b383552544b9bf5" },
};

int test_rc4()
{
	unsigned int t, p;
	int r = 0;
	RC4 rc4;
	cout << endl << "Testing RC4 crypto function..." << endl;
	for(t = 0; t < sizeof(td_rc4)/sizeof(td_rc4[0]); t++) {
		struct testdata * td = &td_rc4[t];
		rc4.init(td->key);
		string result = hexstr( rc4.transform(td->data) );
		cout << "Test " << t << ". Key: " << td->key << ", data: " << td->data << endl;
		cout << " Result:  " << result << endl;
		cout << " Shuld be " << td->correct << endl;
		if(result == td->correct)
			cout << " `---> test " << t << " OK" << endl;
		else {
			cout << "!!!!!! test " << t << " FAILED !!!!!" << endl;
			r++;
		}
	}
	return r;
}

/*== test AES ==*/

struct testdata_aes
{
	const bool encrypt;
	const char * key;
	const char * data;
	const char * correct;
};

/* Test vectors from http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf */
static struct testdata_aes td_aes[] = {
	// 128 bit key
	{ true,  "000102030405060708090a0b0c0d0e0f", "00112233445566778899aabbccddeeff", "69c4e0d86a7b0430d8cdb78070b4c55a" },
	{ false, "000102030405060708090a0b0c0d0e0f", "69c4e0d86a7b0430d8cdb78070b4c55a", "00112233445566778899aabbccddeeff" },
	// 192 bit key
	{ true,  "000102030405060708090a0b0c0d0e0f1011121314151617", "00112233445566778899aabbccddeeff", "dda97ca4864cdfe06eaf70a0ec0d7191" },
	{ false, "000102030405060708090a0b0c0d0e0f1011121314151617", "dda97ca4864cdfe06eaf70a0ec0d7191", "00112233445566778899aabbccddeeff" },
	// 256 bits
	{ true,  "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f", "00112233445566778899aabbccddeeff", "8ea2b7ca516745bfeafc49904b496089" },
	{ false, "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f", "8ea2b7ca516745bfeafc49904b496089", "00112233445566778899aabbccddeeff" },
};

int test_aes()
{
	unsigned int t, p;
	int r = 0;
	cout << endl << "Testing AES crypto function..." << endl;
	for(t = 0; t < sizeof(td_aes)/sizeof(td_aes[0]); t++) {
		struct testdata_aes * td = &td_aes[t];
		string key = unhexstr(td->key);
		AES aes(td->encrypt, key.length()*8);
		aes.init(key);
		string result = hexstr( aes.transform(unhexstr(td->data)) );
		cout << "Test " << t << ". Encr: " << td->encrypt << ", Bits: " << key.length()*8 << ", Key: " << td->key << ", data: " << td->data << endl;
		cout << " Result:   " << result << endl;
		cout << " Should be " << td->correct << endl;
		if(result == td->correct)
			cout << " `---> test " << t << " OK" << endl;
		else {
			cout << "!!!!!! test " << t << " FAILED !!!!!" << endl;
			r++;
		}
	}
	return r;
}

int main()
{
	int r = 0;
	r += test_md5();
	r += test_rc4();
	r += test_aes();
	if(r)
		cout << endl << "!!! " << r << "tests failed! Check output for details." << endl;
	else
		cout << endl << "=== All tests completed successfully." << endl;
	return r;
}

#endif

