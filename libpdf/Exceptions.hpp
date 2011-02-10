
#ifndef PDF_EXCEPTIONS_HPP
#define PDF_EXCEPTIONS_HPP

// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

//#include <fstream>
#include <string>
#include <sstream>

#define NOT_IMPLEMENTED(a) \
 throw std::string(__FILE__ ": Not implemented: "a);

namespace PDF {

/// PDF File format exception class
class FormatException:public std::exception
{
  private:
    std::string s;
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
      std::stringstream r;
      r << "PDF format error: " << s;
      if(off>=0) r << " at offset " << off;
      return r.str().c_str();
    }
};

/// Error in document structure
class DocumentStructureException:public std::exception
{
  private:
    std::string msg;
  public:
    DocumentStructureException(std::string s="") throw() :msg(s) { }
    virtual ~DocumentStructureException() throw() {}
    virtual const char * what() throw() { return (std::string("DocStrucErr: ")+msg).c_str(); }
};

class UnimplementedException:public std::exception
{
public:
	UnimplementedException(const char * s):std::exception(s) { }
    virtual const char * what() throw()
    {
		return (std::string("Unimplemented: ") + std::exception::what()).c_str();
    }
};

class FreeObjectUsageException:public std::exception
{
public:
	FreeObjectUsageException(const char * s):std::exception(s) { }
	virtual const char * what() throw()
	{
		return (std::string("Usage of free object: ") + std::exception::what()).c_str();
	}
};

}; // namespace PDF


#endif /* PDF_EXCEPTIONS_HPP */

