#ifndef SECHANDLER_HPP_INCLUDED
#define SECHANDLER_HPP_INCLUDED

#include <string>
#include <vector>

namespace PDF {

class Dictionary;
class File;
class Filter;

class SecHandler
{
	public:
		enum ObjType { OBJ_UNKNOWN = 0, OBJ_STREAM, OBJ_STRING };
		virtual ~SecHandler() { }
		static SecHandler * create(const Dictionary * cryptodict, const File * file = NULL);
		virtual bool set_password(const std::string & pw, int which = 0) = 0;
		virtual void decrypt_object(long num, long gen, std::vector<char>& buf, ObjType ot = OBJ_UNKNOWN) = 0;
		virtual Filter * create_stream_filter(Dictionary * dic) = 0;
		virtual Filter * create_string_filter() = 0;
		virtual std::string dump() const = 0;
};

} // namespace PDF

#endif /* SECHANDLER_HPP_INCLUDED */

