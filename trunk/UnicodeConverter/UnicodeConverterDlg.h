// UnicodeConverterDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CUnicodeConverterDlg dialog
class CUnicodeConverterDlg : public CDialog
{
// Construction
public:
	CUnicodeConverterDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_UNICODECONVERTER_DIALOG };

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
    CEdit m_raw;
public:
    CEdit m_unicode;
public:
    afx_msg void OnBnClickedButton1();
public:
    afx_msg void OnBnClickedButton2();
};
