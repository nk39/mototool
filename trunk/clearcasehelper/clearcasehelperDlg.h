// clearcasehelperDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "Redir.h"
#include "ResizeLib/ResizableDialog.h"
#include "web.h"
#include <Afxmt.h>
#include "CustomListCtrl/XListCtrl.h"
#include <list>

#define WM_MY_MESSAGE (WM_USER + 100)

//WPARAM
#define MSG_OK 0
#define MSG_FAILURE 1
#define MSG_DONE 0xFFFF

enum FileInfo {
    FileInfo_Name = 0,
    FileInfo_Base = 1,
    FileInfo_Curr = 2,
    FileInfo_Prev = 3,
    FileInfo_Status = 4
};

// CclearcasehelperDlg dialog
class CclearcasehelperDlg : public CResizableDialog
{
    DECLARE_EASYSIZE

    class CClearRedirector :
	    public CRedirector
    {
        CString m_cache;
        CString m_prefix;
    public:
        CClearRedirector(CXListCtrl &list) : m_list(list){}
        ~CClearRedirector(void){}

        void SetPrefix(CString pref) { m_prefix = pref; }
    public:
	    CXListCtrl& m_list;

    protected:
	    // overrides:
	    virtual void WriteStdOut(LPCTSTR pszOutput);
	    virtual void WriteStdError(LPCTSTR pszError);
    } m_cleartool;

    void PrepareCIPT();
    void GenerateCIPT(CString sOld, CString sNew);

    enum {
        StateLoginPrepare,      // prepare login
        StateLoginCheck,        // wait Completion to check login
        StateLoginPerform,      // perform login and wait to check result ...
        StateLoginFailed,       // login failed
        StateLoginSuccess,      // login success
        StateCIPTPrepare,       // prepare CIPT stuff
        StateCIPTTwoFile,       // two file comparison
        StateFileRequest,       // request send to server, wait response 
        StateFileDownload,      // file downloaded
        StateFileFailed,        // error received from server
        StateFileFinish         // file finished
    }m_state;
// Construction
public:
	CclearcasehelperDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CLEARCASEHELPER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
    CMutex m_mutex;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	afx_msg LRESULT OnDocumentComplete(LPDISPATCH pDisp, VARIANT FAR* URL);
public:
    afx_msg void OnBnClickedBrowse();
public:
    afx_msg void OnBnClickedBrowseout();
public:
    afx_msg void OnBnClickedCipt();
public:
    afx_msg void OnBnClickedDiff();
public:
    afx_msg void OnBnClickedRefresh();
public:
    afx_msg void OnBnClickedBase();
public:
    afx_msg void OnBnClickedPrev();
public:
    CXListCtrl m_files;
public:
    afx_msg void OnBnClickedSelall();
public:
    afx_msg void OnBnClickedClrall();
public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
public:
    CEdit m_source;
public:
    CEdit m_output;
public:
    CEdit m_branch;
public:
    CWeb m_loginWeb;

public:
    void GetTwoFileHomeThread();
    void RetrieveCIPTThreadProc();

protected:
    struct CIPTItem{
        int item;
        std::string path;
        std::string name;
        std::string vold;
        std::string vnew;
    };

    std::list<CIPTItem> m_items;
    bool m_bCompareBase;
    CString m_sURL;
    CString m_sFile;

    CString m_sOld;
    CString m_sNew;
    CString m_sOldPath;
    CString m_sNewPath;

    CIPTItem m_item;
    void EnableSubControls(BOOL bEnable);
public:
    afx_msg void OnBnClickedOk();
public:
    CStatic m_status;
public:
    afx_msg void OnBnClickedCiptoption();
};
