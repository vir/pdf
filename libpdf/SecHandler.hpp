#ifndef SECHANDLER_HPP_INCLUDED
#define SECHANDLER_HPP_INCLUDED

#include <string>

namespace PDF {

class Dictionary;
class File;

class SecHandler
{
	public:
		virtual ~SecHandler() { }
		static SecHandler * create(const Dictionary * cryptodict, const File * file = NULL);
		virtual bool set_password(const std::string & pw, int which = 0) = 0;
		virtual void decrypt_object(long num, long gen, char * buf, long len) = 0;
};

} // namespace PDF

#endif /* SECHANDLER_HPP_INCLUDED */

