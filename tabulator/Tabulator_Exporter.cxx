#include "Tabulator_Exporter.hpp"

static std::string quote_csv(std::string s)
{
	unsigned int l=s.find_first_not_of(" ");
	if(l) s.erase(0, l);
	l=s.find_last_not_of(" ");
	if(l!=s.npos) s.resize(l+1);
	l=0;
	while((l=s.find("\"", l))!=s.npos) {
		s.insert(l, "\\");
		l+=2;
	}
	s.insert(0, "\"");
	s.append("\"");
	return s;
}

static void replace_in_place(std::string & s, std::string what, std::string with)
{
	size_t pos = 0;
	while((pos = s.find(what, pos)) != std::string::npos) {
		s.replace(pos, what.length(), with);
		pos += with.length();
	}
}

static std::string quote_html(std::string s)
{
	replace_in_place(s, "<", "&lt;");
	replace_in_place(s, ">", "&gt;");
	replace_in_place(s, "&", "&amp;");
	replace_in_place(s, "\"", "&quot;");
	return s;
}

static std::string ws2utf8(const std::wstring & ws)
{
	std::string s;
	for(unsigned int i=0; i<ws.length(); i++)
	{
		unsigned long n=ws[i];

		if(n & ~0x7F)
		{
			int x=0;
			unsigned char cbuf[6];
			unsigned char header=0x80;

			while(n & ~0x3F)
			{
				cbuf[x++]=n&0x3F; n>>=6;
				header>>=1; header|=0x80;
			}

			s+=char(n | header); x--;
			while(x>=0) { s+=char(cbuf[x--]|0x80); }
		}
		else { s+=char(n); }
	}
	return s;
}

void ExporterCSV::cell(std::wstring text, unsigned int c, unsigned int r)
{
	if(need_delimiter)
		s << delimiter;
	s << quote_csv( ws2utf8( text ) );
	need_delimiter = true;
}

void ExporterCSV::row_end()
{
	s << std::endl;
	need_delimiter = false;
}

/*= ExporterHTML =======================================================*/

void ExporterHTML::table_begin(unsigned int ncols, unsigned int nrows)
{
	s << "<table>" << std::endl;
}

void ExporterHTML::row_begin(unsigned int r)
{
	s << "<tr>";
}

void ExporterHTML::cell(const Tabulator::Table::Cell * cptr, unsigned int c, unsigned int r)
{
	if(!cptr) {
		s << "<td></td>";
		return;
	}
	s << (cptr->is_header?"<th>":"<td>");
	s << quote_html( ws2utf8( cptr->celltext() ) );
	s << (cptr->is_header?"</th>":"</td>");
}

void ExporterHTML::row_end()
{
	s << "</tr>" << std::endl;
}

void ExporterHTML::table_end()
{
	s << "</table>" << std::endl;
}

#ifdef _WIN32
/*===============================================================*\
 *======= Excel =================================================*
\*===============================================================*/
#include <ole2.h> 
#include <stdio.h>

HRESULT AutoWrap(int autoType, VARIANT *pvResult, IDispatch *pDisp, LPOLESTR ptName, int cArgs...) {
    // Begin variable-argument list...
    va_list marker;
    va_start(marker, cArgs);

    if(!pDisp) {
        MessageBox(NULL, "NULL IDispatch passed to AutoWrap()", "Error", 0x10010);
        _exit(0);
    }

    // Variables used...
    DISPPARAMS dp = { NULL, NULL, 0, 0 };
    DISPID dispidNamed = DISPID_PROPERTYPUT;
    DISPID dispID;
    HRESULT hr;
    char buf[200];
    char szName[200];
    
    // Convert down to ANSI
    WideCharToMultiByte(CP_ACP, 0, ptName, -1, szName, 256, NULL, NULL);
    
    // Get DISPID for name passed...
    hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &dispID);
    if(FAILED(hr)) {
        sprintf(buf, "IDispatch::GetIDsOfNames(\"%s\") failed w/err 0x%08lx", szName, hr);
        MessageBox(NULL, buf, "AutoWrap()", 0x10010);
        _exit(0);
        return hr;
    }
    
    // Allocate memory for arguments...
    VARIANT *pArgs = new VARIANT[cArgs+1];
    // Extract arguments...
    for(int i=0; i<cArgs; i++) {
        pArgs[i] = va_arg(marker, VARIANT);
    }
    
    // Build DISPPARAMS
    dp.cArgs = cArgs;
    dp.rgvarg = pArgs;
    
    // Handle special-case for property-puts!
    if(autoType & DISPATCH_PROPERTYPUT) {
        dp.cNamedArgs = 1;
        dp.rgdispidNamedArgs = &dispidNamed;
    }
    
    // Make the call!
    hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, autoType, &dp, pvResult, NULL, NULL);
    if(FAILED(hr)) {
        sprintf(buf, "IDispatch::Invoke(\"%s\"=%08lx) failed w/err 0x%08lx", szName, dispID, hr);
        MessageBox(NULL, buf, "AutoWrap()", 0x10010);
        _exit(0);
        return hr;
    }
    // End variable-argument section...
    va_end(marker);
    
    delete [] pArgs;
    
    return hr;
}

static int _tmain(/*int argc, _TCHAR* argv[]*/)
{
    VARIANT root[64] = {0}; // Generic IDispatchs
    VARIANT parm[64] = {0}; // Generic Parameters
    VARIANT rVal = {0}; // Temporary result holder
    int level=0; // Current index into root[]

    // Initialize the OLE Library...
    OleInitialize(NULL);

    // Line 1: dim app as object 
    VARIANT app = {0};

    // Line 2: set app = createobject Excel.Application 
    {
        CLSID clsid;
        CLSIDFromProgID(L"Excel.Application", &clsid);
        HRESULT hr = CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER|CLSCTX_INPROC_SERVER, IID_IDispatch, (void **)&rVal.pdispVal);
        if(FAILED(hr)) {
            char buf[256];
            sprintf(buf, "CoCreateInstance() for \"Excel.Application\" failed. Err=%08lx", hr);
            ::MessageBox(NULL, buf, "Error", 0x10010);
            _exit(0);
        }
        rVal.vt = VT_DISPATCH;
    }
    VariantCopy(&app, &rVal);
    VariantClear(&rVal);

    // Line 3: app . visible = 0 
    rVal.vt = VT_I4;
    rVal.lVal = 1; // 0;
    VariantCopy(&root[++level], &app);
    AutoWrap(DISPATCH_PROPERTYPUT, NULL, root[level].pdispVal, L"visible", 1, rVal);
    VariantClear(&root[level--]);
    VariantClear(&rVal);

    // Line 4: app . workbooks . add 
    VariantCopy(&root[++level], &app);
    AutoWrap(DISPATCH_PROPERTYGET|DISPATCH_METHOD, &root[level+1], root[level++].pdispVal, L"workbooks", 0);
    AutoWrap(DISPATCH_METHOD, NULL, root[level].pdispVal, L"add", 0);
    VariantClear(&root[level--]);
    VariantClear(&root[level--]);

    // Line 5: dim arr 1 to 15 , 1 to 15 as long 
    VARIANT arr;
    arr.vt = VT_ARRAY | VT_VARIANT;
    {
        SAFEARRAYBOUND sab[2];
        sab[0].lLbound = 1; sab[0].cElements = 15-1+1;
        sab[1].lLbound = 1; sab[1].cElements = 15-1+1;
        arr.parray = SafeArrayCreate(VT_VARIANT, 2, sab);
    }

	// Line 6: for i = 1 to 15 
	for(int i=1; i<=15; i++) {
		// Line 7: for j = 1 to 15 
		for(int j=1; j<=15; j++) {
			// Line 8: arr i , j = i 
			VARIANT tmp = {0};
			tmp.vt = VT_I4;
			tmp.lVal = i * j;
			long indices[] = {i, j};
			SafeArrayPutElement(arr.parray, indices, (void*)&tmp);
		}
	}

    // Line 11: app . activesheet . range A1:O15 . value = arr 
    VariantCopy(&root[++level], &app);
    AutoWrap(DISPATCH_PROPERTYGET|DISPATCH_METHOD, &root[level+1], root[level++].pdispVal, L"activesheet", 0);
    parm[0].vt = VT_BSTR; parm[0].bstrVal = ::SysAllocString(L"A1:O15");
    AutoWrap(DISPATCH_PROPERTYGET|DISPATCH_METHOD, &root[level+1], root[level++].pdispVal, L"range", 1, parm[0]);
    VariantClear(&parm[0]);
    AutoWrap(DISPATCH_PROPERTYPUT, NULL, root[level].pdispVal, L"value", 1, arr);
    VariantClear(&root[level--]);
    VariantClear(&root[level--]);
    VariantClear(&root[level--]);

#if 0
    // Line 12: app . activeworkbook . SaveAs C:\\test.xls 
    VariantCopy(&root[++level], &app);
    AutoWrap(DISPATCH_PROPERTYGET|DISPATCH_METHOD, &root[level+1], root[level++].pdispVal, L"activeworkbook", 0);
    parm[0].vt = VT_BSTR; parm[0].bstrVal = ::SysAllocString(L"C:\\test.xls");
    AutoWrap(DISPATCH_METHOD, NULL, root[level].pdispVal, L"SaveAs", 1, parm[0]);
    VariantClear(&parm[0]);
    VariantClear(&root[level--]);
    VariantClear(&root[level--]);
#endif

#if 0
    // Line 13: app . quit 
    VariantCopy(&root[++level], &app);
    AutoWrap(DISPATCH_METHOD, NULL, root[level].pdispVal, L"quit", 0);
    VariantClear(&root[level--]);
#endif

    // Line 14: set app = nothing 
    VariantClear(&app);

    // Clearing variables
    VariantClear(&app);
    VariantClear(&arr);

    // Close the OLE Library...
    OleUninitialize();

    return 0;
}
#endif /* _WIN32 */