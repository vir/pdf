#include "Filter.hpp"
#include "Crypto.hpp"
#include <assert.h>
#include "Exceptions.hpp"
#include <string.h> // for memcpy
#include <cmath>

#ifndef min
# include <algorithm>
using std::min;
#endif

PDF::AESV2Filter::AESV2Filter(const Dictionary * params /*= NULL*/)
	: aes(NULL)
	, bufused(0)
{
	NOT_IMPLEMENTED("AES Crypto filter with params");
}

PDF::AESV2Filter::~AESV2Filter()
{
	delete aes;
}

bool PDF::AESV2Filter::Encode(const std::vector<char> & src, std::vector<char> & dst)
{
	process(src, true, dst);
	return true;
}

bool PDF::AESV2Filter::Decode(const std::vector<char> & src, std::vector<char> & dst)
{
	process(src, false, dst);
	return true;
}


void PDF::AESV2Filter::process(const std::vector<char> &src, bool encr, std::vector<char> &dst)
{
	assert(blocksize == keysize);
	size_t skip = 0;
	const char * data = &src[0];
	size_t remaining = src.size();
	if(bufused) {
		size_t append = min((size_t)(sizeof(buf) - bufused), src.size());
		memcpy(&buf[bufused], data, append);
		bufused += append;
		skip += append;
		remaining -= append;
		data = buf;
	}
	remaining += bufused;
	if(remaining < blocksize)
		return;
	if(bufused) {
		process(buf, encr, dst);
		bufused = 0;
		remaining -= blocksize;
	}
	while(remaining >= blocksize) {
		process(&src[skip], encr, dst);
		skip += blocksize;
		remaining -= blocksize;
	}
	if(remaining) {
		memcpy(buf, &src[skip], remaining);
		bufused = remaining;
	}
}

void PDF::AESV2Filter::process(const char * data, bool encr, std::vector<char> &dst)
{
	if(aes) {
		size_t pos = dst.size();
		dst.resize(pos + blocksize);
		aes->transform_one_block(data, &dst[pos]);
	} else {
		aes = new AES_CBC(data, encr, key, blocksize);
	}
}


