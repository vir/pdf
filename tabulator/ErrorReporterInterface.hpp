#ifndef ERRORREPORTERINTERFACE_H_INCLUDED
#define ERRORREPORTERINTERFACE_H_INCLUDED

class ErrorReporter
{
public:
	virtual ~ErrorReporter() { }
	virtual void ReportError(const char * prefix, const char * msg) = 0;
};

#endif /* ERRORREPORTERINTERFACE_H_INCLUDED */
