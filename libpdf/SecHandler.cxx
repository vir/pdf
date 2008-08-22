#include "SecHandler.hpp"
#include "Object.hpp"
#include "Crypto.hpp"
#include "File.hpp"

/** \file SecHandler.cxx Implementation of algorithms, specified in section 3.5
 * of the pdf reference.
 **/

static std::string hexstr(const std::string & s)
{
#define HEXB(c) (((c)<10)?('0'+(c)):('a'+((c)-10)))
	unsigned int i;
	std::string r;
	r.reserve(s.length() * 2);
	for(i = 0; i < s.length(); i++) {
		r += HEXB((s[i]>>4)&0x0F);
		r += HEXB(s[i]&0x0F);
	}
	return r;
#undef HEXB
}

namespace PDF {

/* Typical security dictionary:
 	Filter => /Standard
 	Length => 128
 	O =>(6\x83\x17\x94\xFC\x91\xCFP\xD0\x8Cd\x18\x1E\xF8E[\x03\xFCma\x89\xDB{\x10\x95\x19\xB8\xA0\x0B\x9EQ\xA6)
	P => -1340
 	R => 3
 	U => (\xAE\x82O\x05\x9B\xCC\xA1\xB8]2j\x02\xC0\x1A\x8C\x82\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00)
	V => 2
 */

class StdSecHandler:public SecHandler
{
	private:
		const File * f;
		static const char * pwpadding;
		int R;
		std::string O; // 32 chars
		std::string U; // 32 chars
		int32_t P;
		int Length;
		std::string encrkey;
	protected:
		void compute_encryption_key(std::string pw);
		std::string compute_O(const std::string & pw);
		std::string compute_U_r2(const std::string & pw);
		std::string compute_U_r3(const std::string & pw);
	public:
		StdSecHandler(const File * file):f(file),R(0),P(0),Length(0) { }
		virtual bool set_password(const std::string & pw, int which = 0);
		void init(const Dictionary * cryptodict);
		bool set_user_password(const std::string & pw);
		bool set_owner_password(const std::string & pw);
		virtual void decrypt_object(long num, long gen, char * buf, long len);
};

SecHandler * SecHandler::create(const Dictionary * cryptodict, const File * file)
{
	Name * n;

	// first, check for standard security handler
	if(cryptodict->find("Filter", n)) {
		if(n->value() != "Standard")
			throw std::string("Unknown security handler, only 'Standard' is supported");
	} else
		throw std::string("Invalid or missing 'Filter' entry in crypto dictionary");
	// TODO: support SubFilter one day?

	StdSecHandler * h = new StdSecHandler(file);
	h->init(cryptodict);
	return h;
}

const char * StdSecHandler::pwpadding =
	"\x28\xBF\x4E\x5E\x4E\x75\x8A\x41\x64\x00\x4E\x56\xFF\xFA\x01\x08"
	"\x2E\x2E\x00\xB6\xD0\x68\x3E\x80\x2F\x0C\xA9\xFE\x64\x53\x69\x7A";

void StdSecHandler::init(const Dictionary * cryptodict)
{
	Integer * ii;
	String * ss;
	if(! cryptodict->find("R", ii))
		throw std::string("Bad crypto dict: no R integer");
	R = ii->value();
	if(! cryptodict->find("O", ss))
		throw std::string("Bad crypto dict: no O string");
	O = ss->value();
	if(! cryptodict->find("U", ss))
		throw std::string("Bad crypto dict: no U string");
	U = ss->value();
	if(! cryptodict->find("P", ii))
		throw std::string("Bad crypto dict: no P integer");
	P = ii->value();
	if(cryptodict->find("Length", ii))
		Length = ii->value()/8; // in bytes
	else
		Length = 5; // 40 bits
	// TODO: support EncryptMetadata ?
}

void StdSecHandler::compute_encryption_key(std::string pw) // Algorithm 3.2 Computing an encryption key
{
	// 1. pad or truncate
	if(pw.length() < 32) {
		pw.append(pwpadding, 32 - pw.length());
	}
	pw.resize(32);
	// 2. init md5 hash and pass pw
	MD5 md5;
	unsigned char digest[16];
	md5.init();
	md5.update(pw.c_str(), pw.length());
	// 3. pass O to md5
	md5.update(O.c_str(), O.length());
	// 4. pass P to md5
	md5.update((const char*)&P, sizeof(P));
	// 5. pass ID to md5
	std::string first_id = f->id(0);
std::cerr << "First file id: " << hexstr(first_id) << std::endl;
	md5.update(first_id.c_str(), first_id.length());
	// 6. if R>=4 and no metadata encr. pass  0xFFFFFFFF
	if(R >= 4) { // XXX EncryptMetadata
		uint32_t t = 0xFFFFFFFF;
		md5.update((const char*)&t, sizeof(t));
	}
	// 7. Finish the hash.
	md5.final(digest);
	// 8. if R>=3, rehash (Length)bits 50 times
	if(R >= 3) {
		int i;
		for(i = 0; i < 50; i++) {
			md5.init();
			md5.update(digest, Length);
			md5.final(digest);
		}
	}
	// 9. Set encr. key
	encrkey = std::string((char*)digest, Length);
std::cerr << "Computed encr. key: " << hexstr(encrkey) << std::endl;
}

std::string StdSecHandler::compute_O(const std::string & pw)
{
	std::cerr << "Unimplemented StdSecHandler::compute_O() called" << std::endl;
	return false;
}

std::string StdSecHandler::compute_U_r2(const std::string & pw) // Algorithm 3.4
{
	compute_encryption_key(pw);
	return RC4::transform(encrkey, std::string(pwpadding, 32));
}

std::string StdSecHandler::compute_U_r3(const std::string & pw) // Algorithm 3.5
{
	unsigned int i, j;
	std::string s;
	/* 1. Create an encryption key based on the user password string, as described in Algorithm 3.2. */
	compute_encryption_key(pw);
	/* 2. Initialize the MD5 hash function and pass the 32-byte padding string as input to this function. */
	MD5 md5;
	unsigned char digest[16];
	md5.init();
	md5.update(pwpadding, 32);
	/* 3. Pass the first element of the file's file identifier array to the hash function and finish the hash. */
	std::string first_id = f->id(0);
	md5.update(first_id.c_str(), first_id.length());
	md5.final(digest);
	/* 4. Encrypt the 16-byte result of the hash, using an RC4 encryption function with the encryption key from step 1. */
	s = RC4::transform(encrkey, std::string((char*)digest, sizeof(digest)));
	for(i = 1; i <= 19; i++) {
		std::string key = encrkey;
		for(j = 0; j < key.length(); j++)
			key[j] ^= i;
		s = RC4::transform(key, s);
	}
	return s + std::string(pwpadding, 16);
}

bool StdSecHandler::set_password(const std::string & pw, int which)
{
	return which?set_owner_password(pw):set_user_password(pw);
}

bool StdSecHandler::set_user_password(const std::string & pw)
{
	std::string myU;
	if(R == 2) {
		myU = compute_U_r2(pw);
	} else if(R >= 3) {
		myU = compute_U_r3(pw);
	}
std::cerr << "Testing password match (first 16 bytes:" << std::endl << hexstr(myU) << std::endl << hexstr(U) << std::endl;
	return myU.substr(0, 16) == U.substr(0, 16); // Algorithm 3.6
}
bool StdSecHandler::set_owner_password(const std::string & pw)
{
	std::cerr << "Unimplemented StdSecHandler::set_owner_password() called" << std::endl;
	return false;
}

void StdSecHandler::decrypt_object(long num, long gen, char * buf, long len) // Algorithm 3.1 Encryption of data using the RC4 or AES algorithms
{
	/* 1. Obtain the object number and generation number from the object ... */
	/* 2. Treating the object number and generation number as binary integers,
	 * extend the original n-byte encryption key to n + 5 bytes by appending the
	 * low-order 3 bytes of the object number and the low-order 2 bytes of the
	 * generation number in that order, low-order byte first. */
	std::string ek = encrkey;
	ek.append((char*)&num, 3);
	ek.append((char*)&gen, 2);
	/* 3. Initialize the MD5 hash function and pass the result of step 2 as input to this function. */
	MD5 md5;
	md5.update(ek);
	ek = md5.final();
	if(Length + 5 < 16)
		ek.resize(Length + 5);

	RC4 rc4;
	rc4.init(ek);
	rc4.transform(buf, len);
}

} // namespace PDF
