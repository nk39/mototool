// DDTSCheckerDlg.h : header file
//

#pragma once

#include <vector>
#include "CustomListCtrl/XListCtrl.h"
#include "ResizeLib/ResizableDialog.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "HTTPClient.h"

#define WM_THREAD_DONE  (WM_USER + 100)
#define     WPARAM_NEEDLOGIN 1
#define     WPARAM_GETHTML   2
#define     WPARAM_LOGINDONE 3
#define     WPARAM_EXCEPTION   0xFFFFFFFF

struct Rule{
    enum Operation{
        OPEqual,        // this field must be equal to value
        OPLarger,       // this field must be larger than value (integer)
        OPMust,         // this field cannot be empty
        OPLink,         // Link shall be contained. value omitted
        OPContain       // value shall contain substring of value
    };

    Operation op;
    CString item;
    CString value;
};

typedef std::vector<Rule> RuleVector;
// CDDTSCheckerDlg dialog
class CDDTSCheckerDlg : public CResizableDialog
{
  DECLARE_EASYSIZE

  enum State {
      StateGetHTML,
      StateLogin,
      StateIdle
  } m_state;

  void     ResetColumnWidth();

  CHTTPClient m_HTTPClient; 

  RuleVector m_rules;
  static UINT LoginThread(LPVOID pParam);      // param: address of the dialog
  static UINT GetCRThread(LPVOID pParam);      // param: address of the dialog

  void HandleLogin();
  void HandleHTML();
  void HandleException();
  void ApplyRule(CString html, Rule &rule);
  bool LoadRule();
// Construction
public:
	CDDTSCheckerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DDTSCHECKER_DIALOG };

    std::string m_user;
    std::string m_password;
    std::string m_bug;
    std::string m_host;
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
public:
    CXListCtrl m_items;
public:
    CEdit m_cr;
public:
    CEdit m_rule;
public:
    afx_msg void OnEnChangeCr();
public:
    CButton m_btnQuery;
public:
    afx_msg void OnBnClickedAbort();
public:
    afx_msg void OnBnClickedQuery();
public:
    afx_msg void OnBnClickedBrowse();
    LRESULT OnThreadDone(WPARAM wParam, LPARAM lParam);
public:
    CButton m_btnBrowse;
public:
    CButton m_btnAbort;
public:
    CButton m_btnOpen;
public:
    afx_msg void OnBnClickedOpen();
public:
    afx_msg void OnBnClickedValidate();
public:
    CButton m_btnValidate;
};
