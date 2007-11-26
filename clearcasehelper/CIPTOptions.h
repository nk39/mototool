#pragma once


// CCIPTOptions dialog

class CCIPTOptions : public CDialog
{
	DECLARE_DYNAMIC(CCIPTOptions)


    static CCIPTOptions dlgOptions;
protected:
	CCIPTOptions(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCIPTOptions();
public:
// Dialog Data
	enum { IDD = IDD_OPTIONS };

    static CCIPTOptions & GetOptionsDlg() {
        return dlgOptions;
    }

    void Initialize();

    UINT GetBeforeLine() const {
        return m_nBefore;
    }

    UINT GetAfterLine() const {
        return m_nAfter;
    }

    BOOL IsSideBySide() const {
        return m_bSideBySide;
    }

    BOOL ShowNCSL() const {
        return m_bShowNCSL;
    }

    BOOL ShowLegend() const {
        return m_bShowLegend;
    }

    BOOL ShowInPortait() const {
        return m_bPortait;
    }

    BOOL IgnoreWhitespace() const {
        return m_bIgnoreWS;
    }
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
public:
    virtual BOOL OnInitDialog();
public:
    BOOL m_bSideBySide;
public:
    BOOL m_bShowNCSL;
public:
    BOOL m_bShowLegend;
public:
    UINT m_nBefore;
public:
    UINT m_nAfter;
public:
    BOOL m_bPortait;
public:
    BOOL m_bIgnoreWS;
};
