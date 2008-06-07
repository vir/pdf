#include "Excel.hpp"

#include <ole2.h>
#include <stdio.h>

static HRESULT AutoWrap(int autoType, VARIANT *pvResult, IDispatch *pDisp, LPOLESTR ptName, int cArgs...) {
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
		return hr;
	}
	// End variable-argument section...
	va_end(marker);

	delete [] pArgs;
	return hr;
}

Excel::Excel()
{
    // Initialize the OLE Library...
    OleInitialize(NULL);
}

Excel::~Excel()
{
    // Close the OLE Library...
    OleUninitialize();
}

/** Opens already running excel */
bool Excel::open()
{
	return false;
}

/** Start new Excel application */
bool Excel::create()
{
	CLSID clsid;
	CLSIDFromProgID(L"Excel.Application", &clsid);
	HRESULT hr = CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER|CLSCTX_INPROC_SERVER, IID_IDispatch, (void **)&app.pdispVal);
	if(FAILED(hr))
		return false;
	app.vt = VT_DISPATCH;
	return true;
}

/** Set visibility of Excel application */
void Excel::set_visible(bool vis)
{
	VARIANT v;
	v.vt = VT_I4;
	v.lVal = vis?1:0;
	AutoWrap(DISPATCH_PROPERTYPUT, NULL, app.pdispVal, L"visible", 1, v);
	VariantClear(&v);
}

void Excel::add_workbook()
{
	// app . workbooks . add 
	VARIANT workbooks;
	AutoWrap(DISPATCH_PROPERTYGET|DISPATCH_METHOD, &workbooks, app.pdispVal, L"workbooks", 0);
	AutoWrap(DISPATCH_METHOD, NULL, workbooks.pdispVal, L"add", 0);
}

void Excel::set_cell_value(std::wstring s, int offset_c, int offset_r)
{
	VARIANT col, row;
	col.vt = row.vt = VT_I4;
	col.lVal = offset_c;
	row.lVal = offset_r;
	
	VARIANT val;
	val.vt = VT_BSTR;
	val.bstrVal = ::SysAllocString(s.c_str());

	// app . ActiveCell . Offset(r, c) . Value = s
	VARIANT activecell, offset;
	AutoWrap(DISPATCH_PROPERTYGET|DISPATCH_METHOD, &activecell, app.pdispVal, L"ActiveCell", 0);
	AutoWrap(DISPATCH_PROPERTYGET|DISPATCH_METHOD, &offset, activecell.pdispVal, L"Offset", 2, col, row);
	AutoWrap(DISPATCH_PROPERTYPUT, NULL, offset.pdispVal, L"value", 1, val);
	VariantClear(&offset);
	VariantClear(&activecell);

	VariantClear(&val);
	VariantClear(&col);
	VariantClear(&row);
}

bool Excel::save_as(std::wstring fname)
{
	VARIANT v;
	v.vt = VT_BSTR;
	v.bstrVal = ::SysAllocString(fname.c_str());

	// app . activeworkbook . SaveAs C:\\test.xls 
	VARIANT activeworkbook;
	AutoWrap(DISPATCH_PROPERTYGET|DISPATCH_METHOD, &activeworkbook, app.pdispVal, L"activeworkbook", 0);
	AutoWrap(DISPATCH_METHOD, NULL, activeworkbook.pdispVal, L"SaveAs", 1, v);
	VariantClear(&activeworkbook);

	VariantClear(&v);
	return true;
}

bool Excel::quit()
{
	HRESULT hr = AutoWrap(DISPATCH_METHOD, NULL, app.pdispVal, L"quit", 0);
	return !FAILED(hr);
}


#if 0
		// Assigning whole array

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
    AutoWrap(DISPATCH_PROPERTYPUT, NULL, root[yylevel].pdispVal, L"value", 1, arr);
    VariantClear(&root[level--]);
    VariantClear(&root[level--]);
    VariantClear(&root[level--]);

    // Clearing variables
    VariantClear(&arr);
#endif

