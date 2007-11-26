/////////////////////////////////////////////////////////////////
//     By Eugene Khodakovsky         //
//      April,2002           //
//     Eugene@cpplab.com           //
//    Last Update: April, 2002         //
/////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "WebPage.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define CHECK_POINTER(p)\
    ATLASSERT(p != NULL);\
    if(p == NULL)\
{\
    ShowError(_T("NULL pointer"));\
    return false;\
}

const CString GetSystemErrorMessage(DWORD dwError)
{
    CString strError;
    LPTSTR lpBuffer;

    if(!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,  dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
        (LPTSTR) &lpBuffer, 0, NULL))

    {
        strError = "FormatMessage Netive Error" ;
    }
    else
    {
        strError = lpBuffer;
        LocalFree(lpBuffer);
    }
    return strError;
}

CString GetNextToken(CString& strSrc, const CString strDelim,bool bTrim, bool bFindOneOf)
{
    CString strToken;
    int idx = bFindOneOf? strSrc.FindOneOf(strDelim) : strSrc.Find(strDelim);
    if(idx != -1)
    {
        strToken  = strSrc.Left(idx);
        strSrc = strSrc.Right(strSrc.GetLength() - (idx + 1));
    }
    else
    {
        strToken = strSrc;
        strSrc.Empty();
    }
    if(bTrim)
    {
        strToken.TrimLeft();
        strToken.TrimRight();
    }
    return strToken;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWebPage::CWebPage()
{
}

CWebPage::~CWebPage()
{

}

bool CWebPage::SetDocument(IDispatch* pDisp)
{
    CHECK_POINTER(pDisp);

    m_spDoc = NULL;

    CComPtr<IDispatch> spDisp = pDisp;

    HRESULT hr = spDisp->QueryInterface(IID_IHTMLDocument2,(void**)&m_spDoc);
    if(FAILED(hr))
    {
        ShowError(_T("Failed to get HTML document COM object"));
        return false;
    }
    return true;
}

bool CWebPage::GetJScript(CComPtr<IDispatch>& spDisp)
{
    CHECK_POINTER(m_spDoc);
    HRESULT hr = m_spDoc->get_Script(&spDisp);
    ATLASSERT(SUCCEEDED(hr));
    return SUCCEEDED(hr);
}

bool CWebPage::GetJScripts(CComPtr<IHTMLElementCollection>& spColl)
{
    CHECK_POINTER(m_spDoc);
    HRESULT hr = m_spDoc->get_scripts(&spColl);
    ATLASSERT(SUCCEEDED(hr));
    return SUCCEEDED(hr);
}

bool CWebPage::InsertJScript(CString src)
{
    CHECK_POINTER(m_spDoc);

    /*
    // The value of bstrScript should from HTC file,which represents the script function you want to invoke. 
    //Here for the simplicity's sake, I set this value directly. 
    BSTR bstrScript = SysAllocString(OLESTR("function fun1(){alert(\"My Script\");}")); 
    BSTR bstrInsertWhere = SysAllocString(OLESTR("afterBegin")); 

    CComPtr<IHTMLElement> spElement; 
    CComPtr<IHTMLElement2> spElement2; 
    CComPtr<IHTMLElement> spScriptElement; 
    CComPtr<IHTMLScriptElement > spScript; 

    //Create the script element, which will be inserted into HTMLDocument 
    m_spDoc->get_body(&spElement); 
    m_spDoc->createElement(CComBSTR(_T("script")), &spScriptElement); 

    //Set text and attribute of this script element 
    spScriptElement->QueryInterface(IID_IHTMLScriptElement,(void**)&spScript ); 
    spScript->put_defer(VARIANT_TRUE); 
    spScript->put_text(bstrScript); 

    //Insert this element into HTMLDocument 
    spElement->QueryInterface(IID_IHTMLElement2, (void**)&spElement2); 
    spElement2->insertAdjacentElement(bstrInsertWhere, spScriptElement, NULL); 

    //Free resource 
    SysFreeString(bstrScript); 
    SysFreeString(bstrInsertWhere); 
    */

    ///////////////////////////////////////////
    CComPtr<IHTMLElementCollection> pColl;
    HRESULT hr = m_spDoc->get_all(&pColl);
    if(FAILED(hr)){
        return false;
    }

    long l; 
    if(S_OK == pColl->get_length(&l)) 
    { 
        for(long i=0; i<l; i++) 
        { 
            VARIANT vt; 
            vt.vt = VT_I4; 
            vt.lVal = i; 
            CComPtr<IDispatch> pDis; 
            if(S_OK == pColl->item(vt, vt, &pDis)) 
            { 
                CComBSTR str; 
                CComPtr<IHTMLDOMNode> pNode = NULL; 
                hr = pDis->QueryInterface(IID_IHTMLDOMNode, (void **)&pNode);
                if(SUCCEEDED(hr) && pNode) 
                { 
                    if(S_OK == pNode->get_nodeName(&str)) 
                    { 
                        CComPtr<IHTMLElement> pEle; 
                        //获取body element 
                        if(str == CComBSTR("HEAD") || str == CComBSTR("head")) 
                        { 
                            //CComPtr<IHTMLElement> spElement; 
                            CComPtr<IHTMLElement> spScriptElement; 
                            CComPtr<IHTMLScriptElement > spScript; 

                            //创建一个script关键字的element 
                            if(S_OK == m_spDoc->createElement(CComBSTR(_T("SCRIPT")), &spScriptElement)) 
                            { 
                                if(S_OK == spScriptElement->QueryInterface(IID_IHTMLScriptElement,(void**)&spScript )){
                                    spScript->put_defer(VARIANT_TRUE); 
                                    spScript->put_src(CComBSTR(src));                                     
                                    spScript->put_type(CComBSTR(_T("text/javascript")));
                                }

                                CComPtr<IHTMLDOMNode> pNodeScript = NULL; 
                                hr = spScriptElement->QueryInterface(IID_IHTMLDOMNode, (void **)&pNodeScript);
                                if(SUCCEEDED(hr) && pNodeScript) {
                                    CComPtr<IHTMLDOMNode> pReturn; 
                                    //将新的element插入到body最后 
                                    if(S_OK == pNode->appendChild(pNodeScript, &pReturn) && pReturn) 
                                    { 
                                        return true;
                                    } 
                                }
                            } 

                            return false;
                        }
                    } 
                } 
            } 
        } 
    } 

    return false;
}

CString CWebPage::GetHTMLContent()
{
    CHECK_POINTER(m_spDoc);

    CComPtr<IHTMLElement> pBody;
    VERIFY(SUCCEEDED(m_spDoc->get_body(&pBody)));

    CComBSTR  content;
    VERIFY(SUCCEEDED(pBody->get_innerHTML(&content)));

    return CString(content);
}


/*
bool CWebPage::InsertJScript(CString src)
{
CHECK_POINTER(m_spDoc);

CComPtr<IHTMLElementCollection> spColl;
if(!GetJScripts(spColl)){
return false;
}

// Make sure there are items in the collection 
long lCount; 
if(SUCCEEDED(spColl->get_length(&lCount)) && lCount  > 0)
{ 
CComPtr<IDispatch> pDisp;
CComVariant varName,varIndex(lCount);

spColl->item(varName, varIndex, &pDisp);

// Get the element returned above and insert the text
// before the end of it
CComQIPtr<IHTMLElement> pElement(pDisp);
if(pElement){
CString s;
s.Format(_T("<script DEFER src=\"%s\" type=\"text/javascript\"></script>"), src);
HRESULT hr = pElement->insertAdjacentHTML(_T("beforeEnd"), CComBSTR(src));
if(SUCCEEDED(hr)){
return true;
}

GetSystemErrorMessage(hr);
}
}

return false;
}
*/

bool CWebPage::CallJScript(const CString strFunc,CComVariant* pVarResult)
{
    CStringArray paramArray;
    return CallJScript(strFunc,paramArray,pVarResult);
}

bool CWebPage::CallJScript(const CString strFunc,const CString strArg1,CComVariant* pVarResult)
{
    CStringArray paramArray;
    paramArray.Add(strArg1);
    return CallJScript(strFunc,paramArray,pVarResult);
}

bool CWebPage::CallJScript(const CString strFunc,const CString strArg1,const CString strArg2,CComVariant* pVarResult)
{
    CStringArray paramArray;
    paramArray.Add(strArg1);
    paramArray.Add(strArg2);
    return CallJScript(strFunc,paramArray,pVarResult);
}

bool CWebPage::CallJScript(const CString strFunc,const CString strArg1,const CString strArg2,const CString strArg3,CComVariant* pVarResult)
{
    CStringArray paramArray;
    paramArray.Add(strArg1);
    paramArray.Add(strArg2);
    paramArray.Add(strArg3);
    return CallJScript(strFunc,paramArray,pVarResult);
}

bool CWebPage::CallJScript(const CString strFunc, const CStringArray& paramArray,CComVariant* pVarResult)
{
    CComPtr<IDispatch> spScript;
    if(!GetJScript(spScript))
    {
        ShowError(_T("Cannot GetScript"));
        return false;
    }
    CComBSTR bstrMember(strFunc);
    DISPID dispid = NULL;
    HRESULT hr = spScript->GetIDsOfNames(IID_NULL,&bstrMember,1,
        LOCALE_SYSTEM_DEFAULT,&dispid);
    if(FAILED(hr))
    {
        ShowError(GetSystemErrorMessage(hr));
        return false;
    }

    const int arraySize = paramArray.GetSize();

    DISPPARAMS dispparams;
    memset(&dispparams, 0, sizeof dispparams);
    dispparams.cArgs = arraySize;
    dispparams.rgvarg = new VARIANT[dispparams.cArgs];

    for(int i = 0; i < arraySize; i++)
    {
        CComBSTR bstr = paramArray.GetAt(arraySize - 1 - i); // back reading
        bstr.CopyTo(&dispparams.rgvarg[i].bstrVal);
        dispparams.rgvarg[i].vt = VT_BSTR;
    }
    dispparams.cNamedArgs = 0;

    EXCEPINFO excepInfo;
    memset(&excepInfo, 0, sizeof excepInfo);
    CComVariant vaResult;
    UINT nArgErr = (UINT)-1;  // initialize to invalid arg

    hr = spScript->Invoke(dispid,IID_NULL,0,
        DISPATCH_METHOD,&dispparams,&vaResult,&excepInfo,&nArgErr);

    delete [] dispparams.rgvarg;
    if(FAILED(hr))
    {
        ShowError(GetSystemErrorMessage(hr));
        return false;
    }

    if(pVarResult)
    {
        *pVarResult = vaResult;
    }
    return true;
}

// returned java script function name, input string is truncating
CString CWebPage::ScanJScript(CString& strAText, CStringArray& args)
{
    args.RemoveAll();
    CString strDelim(" \n\r\t"),strSrc(strAText);
    bool bFound = false;
    while(!strSrc.IsEmpty())
    {
        CString strStart = GetNextToken(strSrc,strDelim);
        if(strStart == "function")
        {
            bFound = true;
            break;
        }
        if(strStart == "/*")
        {
            // Skip comments
            while(!strSrc.IsEmpty())
            {
                CString strStop = GetNextToken(strSrc,strDelim);
                if(strStop == "*/")
                {
                    break;
                }
            }
        }
    }

    if(!bFound){
        return _T("");
    }

    CString strFunc = GetNextToken(strSrc, _T("("),true);
    CString strArgs = GetNextToken(strSrc, _T(")"),true);

    // Parse arguments
    CString strArg;
    while(!(strArg = GetNextToken(strArgs, _T(","))).IsEmpty())
        args.Add(strArg);

    strAText= strSrc;
    return strFunc;
}
