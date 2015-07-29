
#ifndef PDF_EXCEPTIONS_HPP
#define PDF_EXCEPTIONS_HPP

// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

//#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>

#define NOT_IMPLEMENTED(a) \
	throw UnimplementedException((std::string(__FILE__) + a).c_str());

namespace PDF {

class Exception:public std::exception
{
protected:
	std::string prefix;
	std::string msg;
public:
	Exception(const char * prefix = "", const char * msg = "") throw() :prefix(prefix), msg(msg) { }
	virtual ~Exception() throw() {}
	virtual const char * what() throw()
	{
		std::string r = prefix;
		if(r.length())
			r += ": ";
		r += msg;
		return r.c_str();
	}
	Exception& operator << (const std::string& s) { msg += s; return *this; }
	Exception& operator << (const char * s) { msg += s; return *this; }
	Exception& operator << (long num) { std::stringstream ss; ss << num; msg += ss.str(); return *this; }
};

class LogicException:public Exception
{
public:
	LogicException(const char * msg): Exception("Logic error", msg) { }
};

/// PDF File format exception class
class FormatException:public std::exception
{
  private:
    std::string s, r;
    long off;
  public:
    /**
     * Constructs exception object
     * \param descr problem description.
     * \param offset offset from the beginning of file at (or near) which
     * error encountered.
     */
    FormatException(std::string descr="", long offset=-1) throw()
      :s(descr),off(offset)
    {
    }
    virtual ~FormatException() throw() {}
    virtual const char * what() throw()
    {
      std::stringstream ss;
      ss << "PDF format error: " << s;
      if(off>=0) ss << " at offset " << off;
      r = ss.str();
      return r.c_str();
    }
};

/// Error in document structure
class DocumentStructureException:public std::exception
{
  protected:
    std::string msg;
  public:
    DocumentStructureException(std::string s="") throw() :msg(s) { }
    virtual ~DocumentStructureException() throw() {}
    virtual const char * what() throw() { return (std::string("DocStrucErr: ")+msg).c_str(); }
    DocumentStructureException& operator << (const std::string& s) { msg += s; return *this; }
    DocumentStructureException& operator << (const char * s) { msg += s; return *this; }
};

class WrongPageException:public Exception
{
public:
	WrongPageException(const char * s):Exception("Page drawing error", s) { }
};

class InvalidNodeTypeException: public DocumentStructureException
{
public:
	InvalidNodeTypeException(std::string s="") throw() :DocumentStructureException(s) { }
	virtual const char * what() throw() { return (std::string("NodeTypeErr: ")+msg).c_str(); }
};

class UnimplementedException:public Exception
{
public:
	UnimplementedException(const char * s):Exception("Unimplemented", s) { }
};

class FreeObjectUsageException:public Exception
{
public:
	FreeObjectUsageException(const char * s): Exception("Usage of free object", s) { }
};

class WrongPasswordException:public std::exception
{
	virtual const char * what() throw()
	{
		return "Wrong password";
	}
};

}; // namespace PDF


#endif /* PDF_EXCEPTIONS_HPP */

