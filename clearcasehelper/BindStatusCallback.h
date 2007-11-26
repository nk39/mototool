//////////////////////////////////////////////////////////////////////
//
// Written by Michael Dunn (mdunn at inreach dot com).  Copyright and all
// that stuff.  Use however you like but give me credit where it's due.  I'll
// know if you don't. bwa ha ha ha ha!
// 
// Release history:
//   December 24, 1999: Version 1.0.  First release.
//
//////////////////////////////////////////////////////////////////////

// BindStatusCallback.h: interface for the CBindStatusCallback class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BINDSTATUSCALLBACK_H__6B469ECE_B785_11D3_8D3B_D5CFB868D41C__INCLUDED_)
#define AFX_BINDSTATUSCALLBACK_H__6B469ECE_B785_11D3_8D3B_D5CFB868D41C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "clearcasehelper.h"

#pragma warning(disable:4100)   // disable warnings about unreferenced params

class CCallback : public IBindStatusCallback  
{
public:
	CCallback();
	~CCallback();

    // Pointer to the download progress dialog.
    CURLDownloadDlg* m_pDlg;

    // The time when the download should timeout.
    BOOL  m_bUseTimeout;
    CTime m_timeToStop;

    // IBindStatusCallback methods.  Note that the only method called by IE
    // is OnProgress(), so the others just return E_NOTIMPL.

    STDMETHOD(OnStartBinding)(
        /* [in] */ DWORD dwReserved,
        /* [in] */ IBinding __RPC_FAR *pib)
        { return E_NOTIMPL; }

    STDMETHOD(GetPriority)(
        /* [out] */ LONG __RPC_FAR *pnPriority)
        { return E_NOTIMPL; }

    STDMETHOD(OnLowResource)(
        /* [in] */ DWORD reserved)
        { return E_NOTIMPL; }

    STDMETHOD(OnProgress)(
        /* [in] */ ULONG ulProgress,
        /* [in] */ ULONG ulProgressMax,
        /* [in] */ ULONG ulStatusCode,
        /* [in] */ LPCWSTR wszStatusText);

    STDMETHOD(OnStopBinding)(
        /* [in] */ HRESULT hresult,
        /* [unique][in] */ LPCWSTR szError)
        { return E_NOTIMPL; }

    STDMETHOD(GetBindInfo)(
        /* [out] */ DWORD __RPC_FAR *grfBINDF,
        /* [unique][out][in] */ BINDINFO __RPC_FAR *pbindinfo)
        { return E_NOTIMPL; }

    STDMETHOD(OnDataAvailable)(
        /* [in] */ DWORD grfBSCF,
        /* [in] */ DWORD dwSize,
        /* [in] */ FORMATETC __RPC_FAR *pformatetc,
        /* [in] */ STGMEDIUM __RPC_FAR *pstgmed)
        { return E_NOTIMPL; }

    STDMETHOD(OnObjectAvailable)(
        /* [in] */ REFIID riid,
        /* [iid_is][in] */ IUnknown __RPC_FAR *punk)
        { return E_NOTIMPL; }

    // IUnknown methods.  Note that IE never calls any of these methods, since
    // the caller owns the IBindStatusCallback interface, so the methods all
    // return zero/E_NOTIMPL.

    STDMETHOD_(ULONG,AddRef)()
        { return 0; }

    STDMETHOD_(ULONG,Release)()
        { return 0; }

    STDMETHOD(QueryInterface)(
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
        { return E_NOTIMPL; }
};

#pragma warning(default:4100)

#endif // !defined(AFX_BINDSTATUSCALLBACK_H__6B469ECE_B785_11D3_8D3B_D5CFB868D41C__INCLUDED_)
