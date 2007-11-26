// XListCtrl.h  Version 1.4
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// License:
//     This software is released into the public domain.  You are free to use
//     it in any way you like, except that you may not sell this source code.
//
//     This software is provided "as is" with no expressed or implied warranty.
//     I accept no liability for any damage or loss of business that this 
//     software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef XLISTCTRL_H
#define XLISTCTRL_H

#include "XHeaderCtrl.h"
#include "AdvComboBox.h"
#include "InplaceEdit.h"
#include "InplaceCombo.h"
///////////////////////////////////////////////////////////////////////////////
// CXListCtrl data

struct XLISTCTRLDATA
{
  // ctor
  XLISTCTRLDATA(){
    bEnabled             = TRUE;
    bBold                = FALSE;
    bModified            = FALSE;
    nImage               = -1;
    strToolTip           = _T("");
    bCombo               = FALSE;
    bSort                = FALSE;
    psa                  = NULL;
    nComboListHeight     = 10;
    nInitialComboSel     = -1;	// use default
    bEdit                = FALSE;
    crText               = ::GetSysColor(COLOR_WINDOWTEXT);
    crBackground         = ::GetSysColor(COLOR_WINDOW);

    bShowProgress        = FALSE;
    nProgressPercent     = 0;
    strProgressMessage   = _T("");
    bShowProgressMessage = TRUE;
    nCheckedState        = -1;
    dwItemData           = 0;
    
    bShowTime           = FALSE;
    nTimeSeconds        = 0;
    bEnableTime         = FALSE;
  }

  BOOL			bEnabled;				// TRUE = enabled, FALSE = disabled (gray text)
  BOOL			bBold;					// TRUE = display bold text
  BOOL			bModified;				// TRUE = subitem has been modified
  int				nImage;					// index in image list, else -1
  CString			strToolTip;				// tool tip text for cell

  // for combo
  BOOL			bCombo;					// TRUE = display combobox
  BOOL			bSort;					// TRUE = add CBS_SORT style;  this means
  // that the list will be re-sorted on
  // each AddString()
  CStringArray *	psa;					// pointer to string array for combo listbox
  int				nComboListHeight;		// combo listbox height (in rows)
  int				nInitialComboSel;		// initial combo listbox selection (0 = first)

  //for edit
  BOOL		bEdit;						// true = enable editing

  // for color
  COLORREF	crText;
  COLORREF	crBackground;

  // for time
  BOOL bShowTime;
  time_t nTimeSeconds;
  BOOL bEnableTime;
  
  // for progress
  BOOL		bShowProgress;				// true = show progress control
  int			nProgressPercent;			// 0 - 100
  CString		strProgressMessage;			// custom message for progress indicator -
  // MUST INCLUDE %d
  BOOL		bShowProgressMessage;		// TRUE = display % message, or custom message
  // if one is supplied
  // for checkbox
  int			nCheckedState;				// -1 = don't show, 0 = unchecked, 1 = checked

  DWORD		dwItemData;					// pointer to app's data
};


///////////////////////////////////////////////////////////////////////////////
// CXListCtrl class

class CXListCtrl : public CListCtrl
{
    class CMyTimeCtrl : public CDateTimeCtrl{
        int m_iRowIndex;
        int m_iColumnIndex;
        time_t m_t;
        void FinishEdit();
    public:
        void SetItem(int nItem, int nSubItem, time_t t){
            m_iRowIndex = nItem;
            m_iColumnIndex = nSubItem;
            m_t = t;
            CTime tt(t);
            SetTime(&tt);
        }
        
        time_t GetTime(){
            return m_t;
        }
               
        virtual BOOL PreTranslateMessage(MSG *pMsg);
        virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    };
    
    CImageList m_reminder;
    CString m_sTipEmptyView;
    int m_nSortCol; // The sorted column, -1 if none
    BOOL m_bSortAscending; // Is sort ascending?
    LPTSTR m_pszSeparator; // Sort separator

  bool HitTestEx(CPoint &obPoint, int* pRowIndex, int* pColumnIndex) const;
  void CalculateCellRect(int iColumnIndex, int iRowIndex, CRect& robCellRect);
  void ScrollToView(int iColumnIndex, CRect& robCellRect);

    BOOL _PartialSort(int nStart, int nEnd);
    int _FindSeparator(int nStartAfter, int nColumn) const;
    void _QuickSortRecursive(int* pArr, int nLow, int nHigh);
    int _CompareItems(int nItem1, int nItem2);
    void SortItems(int nColumn, BOOL bAscending); // Sort a specified column.

// Construction
public:
  CXListCtrl();
  virtual ~CXListCtrl();

// Attributes
public:

// Operations
public:
    void SetEmptyViewTip(const CString &sTip){
        m_sTipEmptyView = sTip;
    }

  void SetReminderImages(UINT nImage);
  
  void    StartEdit(int iItem, int iSubItem);
  
  void SetTimeCtrl(int iItem, int iSubItem);
  void UpdateTime(int iItem, int iSubItem, UINT t);
  time_t RetrieveTime(int iItem, int iSubItem);
  void EnableTime(int iItem, int iSubItem, bool bEnable);
  bool IsTimeEnabled(int iItem, int iSubItem);
  
  int		CountCheckedItems(int nSubItem);
  BOOL	DeleteAllItems();
  BOOL	DeleteItem(int nItem);
  void	DeleteProgress(int nItem, int nSubItem);
  int		FindDataItem(DWORD dwData);	//+++
  BOOL	GetBold(int nItem, int nSubItem);
  int		GetCellPadding() { return m_nPadding; }					//+++
  int		GetCheckbox(int nItem, int nSubItem);
  int		GetColumns();
  CString	GetComboText(int iItem, int iSubItem);
  int		GetCurSel();
  BOOL	GetEllipsis() { return m_bUseEllipsis; }				//+++
  BOOL	GetEnabled(int nItem);
  DWORD	GetExtendedStyleX() { return m_dwExtendedStyleX; }
  int		GetHeaderCheckedState(int nSubItem);
  int		GetItemCheckedState(int nItem, int nSubItem);			//+++
  BOOL	GetItemColors(int nItem,								//+++
                      int nSubItem, 
                      COLORREF& crText, 
                      COLORREF& crBackground);
  DWORD	GetItemData(int nItem);
  BOOL	GetListModified() { return m_bListModified; }			//+++
  BOOL	GetModified(int nItem, int nSubItem);					//+++
  BOOL	GetSubItemRect(int iItem, int iSubItem, int nArea, CRect& rect);
  int		InsertItem(int nItem, LPCTSTR lpszItem);
  int		InsertItem(int nItem, 
                           LPCTSTR lpszItem, 
                           COLORREF crText, 
                           COLORREF crBackground);
  int		InsertItem(const LVITEM* pItem);
  BOOL	SetBold(int nItem, int nSubItem, BOOL bBold);
  void	SetCellPadding(int nPadding) { m_nPadding = nPadding; }	//+++
  BOOL	SetComboBox(int nItem, 
                    int nSubItem, 
                    BOOL bEnableCombo, 
                    CStringArray *psa,
                    int nComboListHeight,
                    int nInitialComboSel,
                    BOOL bSort = FALSE);	//+++
  BOOL	SetCheckbox(int nItem, int nSubItem, int nCheckedState);
  virtual int SetColumnWidth(int nCol, int cx)
    {
      CListCtrl::SetColumnWidth(nCol, cx);
      int w = CListCtrl::GetColumnWidth(nCol);
      return CListCtrl::SetColumnWidth(nCol, w + (m_HeaderCtrl.GetSpacing()*2));
    }
  BOOL	SetCurSel(int nItem, BOOL bEnsureVisible = FALSE);	//+++
  BOOL	SetEdit(int nItem, int nSubItem);
  BOOL	SetEllipsis(BOOL bEllipsis)							//+++
    {
      BOOL bOldEllipsis = m_bUseEllipsis;
      m_bUseEllipsis = bEllipsis;
      return bOldEllipsis;
    }
  BOOL	SetEnabled(int nItem, BOOL bEnable);
  DWORD	SetExtendedStyleX(DWORD dwNewStyle) 
    {
      DWORD dwOldStyle = m_dwExtendedStyleX;
      m_dwExtendedStyleX = dwNewStyle;
      return dwOldStyle;
    }

  void	SetHeaderAlignment(UINT nFormat) { m_HeaderCtrl.SetAlignment(nFormat); } //+++
  void	SetHeaderTextColor(COLORREF rgbText) { m_HeaderCtrl.SetTextColor(rgbText); } //+++
  BOOL	SetHeaderCheckedState(int nSubItem, int nCheckedState);
  void	SetItemCheckedState(int nItem, int nSubItem, int nCheckedState);		//+++
  int		SetItem(const LVITEM* pItem);
  void	SetItemColors(int nItem,								//+++
                      int nSubItem, 
                      COLORREF crText, 
                      COLORREF crBackground);
  BOOL	SetItemData(int nItem, DWORD dwData);
  BOOL	SetItemImage(int nItem, int nSubItem, int nImage);
  BOOL	SetItemText(int nItem, int nSubItem, LPCTSTR lpszText); 
  BOOL	SetItemText(int nItem, 
                    int nSubItem, 
                    LPCTSTR lpszText,
                    COLORREF crText, 
                    COLORREF crBackground);
  void	SetListModified(BOOL bListModified) { m_bListModified = bListModified; }	//+++
  void	SetModified(int nItem, int nSubItem, BOOL bModified);			//+++
  BOOL	SetProgress(int nItem, 
                    int nSubItem, 
                    BOOL bShowProgressText = TRUE, 
                    LPCTSTR lpszProgressText = NULL);
  void	UpdateProgress(int nItem, int nSubItem, int nPercent);
  void	UpdateSubItem(int nItem, int nSubItem);
  int   RetrieveProgress(int nItem, int nSubItem);

  void DeleteAllToolTips();
  BOOL SetItemToolTipText(int nItem, int nSubItem, LPCTSTR lpszToolTipText);
  CString GetItemToolTipText(int nItem, int nSubItem);
  virtual int OnToolHitTest(CPoint point, TOOLINFO * pTI) const;

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CXListCtrl)
public:
  virtual void PreSubclassWindow();
  //}}AFX_VIRTUAL

// Implementation
public:
  CXHeaderCtrl	m_HeaderCtrl;
  CImageList		m_cImageList;	// Image list for the header control
  CMyTimeCtrl     m_timeCtrl;
protected:
  void DrawItem(CDC *pDC, int nItem, int nSubItem);
  
  void DrawTimeCtrl(int nItem, int nSubItem);
  void	DrawCheckbox(int nItem, 
                     int nSubItem, 
                     CDC *pDC, 
                     COLORREF crText,
                     COLORREF crBkgnd,
                     CRect& rect, 
                     XLISTCTRLDATA *pCLD);
  void	DrawComboBox(int nItem, 
                     int nSubItem);

  void	DrawProgressComboBox(int nItem, 
                     int nSubItem);
                     
  void	DrawEdit(int nItem, int nSubItem);
  int		DrawImage(int nItem, 
                          int nSubItem, 
                          CDC* pDC, 
                          COLORREF crText,
                          COLORREF crBkgnd,
                          CRect rect,
                          XLISTCTRLDATA *pXLCD);
  void	DrawProgress(int nItem, 
                     int nSubItem, 
                     CDC *pDC, 
                     COLORREF crText,
                     COLORREF crBkgnd,
                     CRect& rect, 
                     XLISTCTRLDATA *pCLD);
  void	DrawText(int nItem, 
                 int nSubItem, 
                 CDC *pDC, 
                 COLORREF crText,
                 COLORREF crBkgnd,
                 CRect& rect, 
                 XLISTCTRLDATA *pCLD);
  void	GetColors();
  void	GetDrawColors(int nItem,
                      int nSubItem,
                      COLORREF& colorText,
                      COLORREF& colorBkgnd);
  void	SubclassHeaderControl();

  BOOL			m_bHeaderIsSubclassed;
  BOOL			m_bUseEllipsis;			//+++
  BOOL			m_bListModified;		//+++
  DWORD			m_dwExtendedStyleX;
  int				m_nPadding;				//+++
  CString			m_strInitialString;		// used to save the initial string
  // for edit and combo fields
  BOOL			m_bInitialCheck;		// initial check state of a checkbox field

  // for edit box 
  CInPlaceEdit *	m_pEdit;	
  CInPlaceCombo *   m_pCombo;
  CInPlaceCombo *   m_pComboProgress;
  
  COLORREF		m_cr3DFace;
  COLORREF		m_cr3DHighLight;
  COLORREF		m_cr3DShadow;
  COLORREF		m_crActiveCaption;
  COLORREF		m_crBtnFace;
  COLORREF		m_crBtnShadow;
  COLORREF		m_crBtnText;
  COLORREF		m_crGrayText;
  COLORREF		m_crHighLight;
  COLORREF		m_crHighLightText;
  COLORREF		m_crInactiveCaption;
  COLORREF		m_crInactiveCaptionText;
  COLORREF		m_crWindow;
  COLORREF		m_crWindowText;

  // Generated message map functions
protected:
  //{{AFX_MSG(CXListCtrl)
  afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg BOOL OnClick(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg BOOL OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnDestroy();
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnPaint();
  afx_msg void OnSysColorChange();
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
  //}}AFX_MSG

  virtual afx_msg BOOL OnToolTipText(UINT id, NMHDR * pNMHDR, LRESULT * pResult);
  afx_msg void MeasureItem ( LPMEASUREITEMSTRUCT lpMeasureItemStruct );
  afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM);
  afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);

  DECLARE_MESSAGE_MAP()
public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
    virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);

    LPCTSTR GetSortSeparator() const;
    void SetSortSeparator(LPCTSTR lpSortSeparator);
    BOOL IsSortAscending() const;
    int GetSortedColumn() const;
    BOOL IsSortable() const; // Is sort allowed?
    BOOL SetSortable(BOOL bSet); // Allow/disallow sorting
protected:
    BOOL HasColumnHeader() const;
    int GetColumnCount();
    BOOL _IsValidIndex(int nIndex);
public:
    afx_msg void OnHdnItemclick(NMHDR *pNMHDR, LRESULT *pResult);
};


inline BOOL CXListCtrl::_IsValidIndex(int nIndex){
	return nIndex >= 0 && nIndex < CListCtrl::GetItemCount();
}

inline int CXListCtrl::GetColumnCount()
{
    return GetHeaderCtrl()->GetItemCount();
}

inline BOOL CXListCtrl::HasColumnHeader() const{
    return (GetStyle() & LVS_NOCOLUMNHEADER) == 0;
}


inline BOOL CXListCtrl::IsSortable() const{
    if (!HasColumnHeader())
        return FALSE;

    LONG lStyle = ::GetWindowLong(ListView_GetHeader(GetSafeHwnd()), GWL_STYLE);
    return (lStyle & HDS_BUTTONS) != 0;
}

inline BOOL CXListCtrl::IsSortAscending() const{
    return m_bSortAscending; // Is sort ascending?
}

inline int CXListCtrl::GetSortedColumn() const{
    return  m_nSortCol; // The sorted column, -1 if none
}

inline LPCTSTR CXListCtrl::GetSortSeparator() const
{
	return m_pszSeparator;
}

inline void CXListCtrl::SetSortSeparator(LPCTSTR lpSortSeparator)
{
	if (m_pszSeparator != NULL)
	{
		delete [] m_pszSeparator;
		m_pszSeparator = NULL;
	}

	if (lpSortSeparator != NULL)
		m_pszSeparator = _tcsdup(lpSortSeparator);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif //XLISTCTRL_H
