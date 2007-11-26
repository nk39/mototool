#include "stdafx.h"
#include "XListCtrl.h"
#include "SortCStringArray.h"
#include "time.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UINT WM_XLISTCTRL_COMBO_SELECTION  = ::RegisterWindowMessage(_T("WM_XLISTCTRL_COMBO_SELECTION"));
UINT WM_XLISTCTRL_EDIT_END         = ::RegisterWindowMessage(_T("WM_XLISTCTRL_EDIT_END"));
UINT WM_XLISTCTRL_CHECKBOX_CLICKED = ::RegisterWindowMessage(_T("WM_XLISTCTRL_CHECKBOX_CLICKED"));

namespace _ITEM_COMPARE_FUNCS
{
	BOOL _IsDecNumber(const CString& str, double& f);
	int _DecNumberCompare(double f1, double f2);
	BOOL _IsHexNumber(const CString& str, DWORD& dw);
	int _HexNumberCompare(DWORD dw1, DWORD dw2);
	BOOL _IsDate(const CString& str, COleDateTime& date);
	int _DateCompare(const COleDateTime& date1, const COleDateTime& date2);
};

BOOL _ITEM_COMPARE_FUNCS::_IsDecNumber(const CString& str, double& f)
{
	if (str.IsEmpty())
		return FALSE;

	LPTSTR p;
	f = _tcstod(str, &p);
	return (*p == _T('\0') || (*p == _T('%') && p[1] == _T('\0')));
}

int _ITEM_COMPARE_FUNCS::_DecNumberCompare(double f1, double f2)
{
	if(f1 < f2)
		return -1;
	
	if(f1 > f2)
		return 1;

	return 0;
}

BOOL _ITEM_COMPARE_FUNCS::_IsHexNumber(const CString& str, DWORD& dw)
{
	if (str.IsEmpty())
		return FALSE;

	LPTSTR p;
	dw = _tcstoul(str, &p, 16);
	return *p == _T('\0');
}

int _ITEM_COMPARE_FUNCS::_HexNumberCompare(DWORD dw1, DWORD dw2)
{
	if (dw1 > dw2)
		return 1;

	if (dw1 < dw2)
		return -1;

	return 0;
}

BOOL _ITEM_COMPARE_FUNCS::_IsDate(const CString& str, COleDateTime& date)
{
	return date.ParseDateTime(str);
}

int _ITEM_COMPARE_FUNCS::_DateCompare(const COleDateTime& date1, const COleDateTime& date2)
{
	if (date1 < date2)
		return -1;

	if (date1 > date2)
		return 1;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CXListCtrl
BEGIN_MESSAGE_MAP(CXListCtrl, CListCtrl)
  ON_WM_KEYDOWN()
ON_WM_HSCROLL()
ON_WM_VSCROLL()
ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndLabelEdit)
ON_NOTIFY_REFLECT(LVN_BEGINLABELEDIT, OnBeginLabelEdit)
//{{AFX_MSG_MAP(CXListCtrl)
ON_NOTIFY_REFLECT_EX(NM_CLICK, OnClick)
ON_NOTIFY_REFLECT_EX(LVN_COLUMNCLICK, OnColumnClick)
ON_WM_CREATE()
ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
ON_WM_DESTROY()
ON_WM_LBUTTONDOWN()
ON_WM_PAINT()
ON_WM_SYSCOLORCHANGE()
ON_WM_ERASEBKGND()
ON_WM_KEYDOWN()
ON_WM_RBUTTONDOWN()
//}}AFX_MSG_MAP
ON_WM_MEASUREITEM_REFLECT()
ON_MESSAGE(WM_SETFONT, OnSetFont)
ON_WM_MEASUREITEM()
ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
ON_NOTIFY(HDN_ITEMCLICKA, 0, &CXListCtrl::OnHdnItemclick)
ON_NOTIFY(HDN_ITEMCLICKW, 0, &CXListCtrl::OnHdnItemclick)
END_MESSAGE_MAP()

void CXListCtrl::CMyTimeCtrl::FinishEdit()
{
  LV_DISPINFO dispinfo;
  dispinfo.hdr.hwndFrom = GetParent()->m_hWnd;
  dispinfo.hdr.idFrom = GetDlgCtrlID();
  dispinfo.hdr.code = LVN_ENDLABELEDIT;
  
  dispinfo.item.mask = LVIF_TEXT;
  dispinfo.item.iItem = m_iRowIndex;
  dispinfo.item.iSubItem = m_iColumnIndex;
  dispinfo.item.pszText = "";
  dispinfo.item.cchTextMax = 0;
	
	CTime t;
	CDateTimeCtrl::GetTime(t);
	m_t = t.GetTime();
	
  GetParent()->SendMessage(WM_NOTIFY, GetParent()->GetDlgCtrlID(), (LPARAM)&dispinfo);
}

LRESULT CXListCtrl::CMyTimeCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if(message == WM_KILLFOCUS){
        LRESULT ret = CDateTimeCtrl::WindowProc(message, wParam, lParam);
        FinishEdit();
        ShowWindow(SW_HIDE);
        return ret;
    }
    
    return CDateTimeCtrl::WindowProc(message, wParam, lParam);
}

BOOL CXListCtrl::CMyTimeCtrl::PreTranslateMessage(MSG *pMsg)
{
  if (WM_KEYDOWN == pMsg->message){
    switch(pMsg->wParam){
    case VK_ESCAPE:
      SetTime(m_t);
      GetParent()->SetFocus();
      return TRUE;
    case VK_RETURN:
    case VK_TAB:
      FinishEdit();
      GetParent()->PostMessage(WM_KEYDOWN, pMsg->wParam);
      return TRUE;
    default:
      break;    //do nothing      
    }
  }
  
  return CDateTimeCtrl::PreTranslateMessage(pMsg);
}

///////////////////////////////////////////////////////////////////////////////
// ctor
CXListCtrl::CXListCtrl()
{
    m_nSortCol = -1;
    m_bSortAscending = TRUE;
    m_pszSeparator = NULL;

  m_pEdit                 = NULL;
  m_pCombo             = NULL;
  m_pComboProgress      = NULL;
  
  m_dwExtendedStyleX      = 0;
  m_bHeaderIsSubclassed   = FALSE;
  m_bUseEllipsis          = TRUE;
  m_bListModified         = FALSE;
  m_bInitialCheck         = FALSE;
  m_strInitialString      = _T("");
  m_nPadding              = 5;	

  GetColors();
}

///////////////////////////////////////////////////////////////////////////////
// dtor
CXListCtrl::~CXListCtrl()
{
    if (m_pszSeparator != NULL){
        delete [] m_pszSeparator;
    }

  if(m_pCombo){
    delete m_pCombo;
  }

  if(m_pComboProgress){
    delete m_pComboProgress;
  }
  
  if (m_pEdit){
    delete m_pEdit;
  }
}

///////////////////////////////////////////////////////////////////////////////
// PreSubclassWindow
void CXListCtrl::PreSubclassWindow()
{
  CListCtrl::PreSubclassWindow();

  // for Dialog based applications, this is a good place
  // to subclass the header control because the OnCreate()
  // function does not get called.

  SubclassHeaderControl();

    CRect rc;
    GetWindowRect( &rc );

    WINDOWPOS wp;
    wp.hwnd  = m_hWnd;
    wp.cx    = rc.Width();
    wp.cy    = rc.Height();
    wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;
    SendMessage( WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp );
}

///////////////////////////////////////////////////////////////////////////////
// OnCreate
int CXListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CListCtrl::OnCreate(lpCreateStruct) == -1)
  {
    ASSERT(FALSE);
    return -1;
  }

  // When the CXListCtrl object is created via a call to Create(), instead
  // of via a dialog box template, we must subclass the header control
  // window here because it does not exist when the PreSubclassWindow()
  // function is called.

  SubclassHeaderControl();

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// SubclassHeaderControl
void CXListCtrl::SubclassHeaderControl()
{
  if (m_bHeaderIsSubclassed)
    return;

  // if the list control has a header control window, then
  // subclass it

  // Thanks to Alberto Gattegno and Alon Peleg  and their article
  // "A Multiline Header Control Inside a CListCtrl" for easy way
  // to determine if the header control exists.

  CHeaderCtrl* pHeader = GetHeaderCtrl();
  if (pHeader)
  {
    VERIFY(m_HeaderCtrl.SubclassWindow(pHeader->m_hWnd));
    m_bHeaderIsSubclassed = TRUE;
    m_HeaderCtrl.SetListCtrl(this);
  }
}

void CXListCtrl::ScrollToView(int iColumnIndex, CRect& robCellRect)
{
  // Now scroll if we need to expose the column
  CRect rcClient;
  GetClientRect(&rcClient);

  int iColumnWidth = GetColumnWidth(iColumnIndex);

  // Get the column iOffset
  int iOffSet = 0;
  for (int iIndex_ = 0; iIndex_ < iColumnIndex; iIndex_++)
  {
    iOffSet += GetColumnWidth(iIndex_);
  }

  // If x1 of cell rect is < x1 of ctrl rect or
  // If x1 of cell rect is > x1 of ctrl rect or **Should not ideally happen**
  // If the width of the cell extends beyond x2 of ctrl rect then
  // Scroll

  CSize obScrollSize(0, 0);

  if (((iOffSet + robCellRect.left) < rcClient.left) || 
      ((iOffSet + robCellRect.left) > rcClient.right))
  {
    obScrollSize.cx = iOffSet + robCellRect.left;
  }
  else if ((iOffSet + robCellRect.left + iColumnWidth) > rcClient.right)
  {
    obScrollSize.cx = iOffSet + robCellRect.left + iColumnWidth - rcClient.right;
  }

  Scroll(obScrollSize);
  robCellRect.left -= obScrollSize.cx;
	
  // Set the width to the column width
  robCellRect.left += iOffSet;
  robCellRect.right = robCellRect.left + iColumnWidth;
}

void CXListCtrl::CalculateCellRect(int iRowIndex, int iColumnIndex, CRect& robCellRect)
{
  //GetItemRect(iRowIndex, &robCellRect, LVIR_LABEL);
  GetItemRect(iRowIndex, &robCellRect, LVIR_BOUNDS);

  CRect rcClient;
  GetClientRect(&rcClient);

  if (robCellRect.right > rcClient.right) {
    robCellRect.right = rcClient.right;
  }

  ScrollToView(iColumnIndex, robCellRect); 
}


///////////////////////////////////////////////////////////////////////////////
// DrawComboBox
void CXListCtrl::DrawComboBox(int nItem, int nSubItem)
{
  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD){
    ASSERT(FALSE);
    return;
  }

  if (!pXLCD[0].bEnabled){
    return;                     // not enabled
  }

  CRect rect;
  CalculateCellRect(nItem, nSubItem, rect);
  m_strInitialString = GetItemText(nItem, nSubItem);

  if(!m_pCombo){
    m_pCombo = new CInPlaceCombo();
    DWORD dwStyle = WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST;
    VERIFY(m_pCombo->Create(dwStyle, rect, this, 0));
  }else{
    ASSERT(m_pCombo && m_pCombo->GetSafeHwnd());
    m_pCombo->MoveWindow(rect);
  }

  m_pCombo->SetDefaultVisibleItems(pXLCD[nSubItem].nComboListHeight);
  m_pCombo->InitializeList(nItem, nSubItem, m_strInitialString, pXLCD[nSubItem].psa);
  m_pCombo->ShowWindow(SW_SHOW);
  m_pCombo->SetFocus();
}

///////////////////////////////////////////////////////////
// special time control
void CXListCtrl::DrawTimeCtrl(int nItem, int nSubItem)
{
  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD){
    ASSERT(FALSE);
    return;
  }

  if (!pXLCD[0].bEnabled){
    return;                     // not enabled
  }

  CRect rect;
  CalculateCellRect(nItem, nSubItem, rect);

  IMAGEINFO info;
  m_reminder.GetImageInfo(0, &info);

  rect.left = rect.left + info.rcImage.right - info.rcImage.left + m_HeaderCtrl.GetSpacing();

  CString sPercent;

  if(m_timeCtrl.GetSafeHwnd()){
    m_timeCtrl.MoveWindow(rect);
  }else{
    DWORD dwStyle = WS_VISIBLE | WS_CHILD | WS_TABSTOP | DTS_UPDOWN | DTS_TIMEFORMAT;
    m_timeCtrl.Create(dwStyle, rect, this, 0);
  }

  m_timeCtrl.SetItem(nItem, nSubItem, pXLCD[nSubItem].nTimeSeconds);
  m_timeCtrl.ShowWindow(SW_SHOW);
  m_timeCtrl.SetFocus();
}

///////////////////////////////////////////////////////////
// special progress combo box
void CXListCtrl::DrawProgressComboBox(int nItem, int nSubItem)
{
  static CStringList gPercentList;
  if(gPercentList.GetCount() == 0){
    for(int i = 0; i <= 10; i++){
        CString s;
        s.Format("%d%%", i * 10);
        gPercentList.AddTail(s);
    }
  }
  
  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD){
    ASSERT(FALSE);
    return;
  }

  if (!pXLCD[0].bEnabled){
    return;                     // not enabled
  }

  CRect rect;
  CalculateCellRect(nItem, nSubItem, rect);
  
  CString sPercent;
  sPercent.Format("%d%%", pXLCD[nSubItem].nProgressPercent);

  if(!m_pComboProgress){
    m_pComboProgress = new CInPlaceCombo();
    DWORD dwStyle = WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST;
    VERIFY(m_pComboProgress->Create(dwStyle, rect, this, 0));
  }else{
    ASSERT(m_pComboProgress && m_pComboProgress->GetSafeHwnd());
    m_pComboProgress->MoveWindow(rect);
  }

  m_pComboProgress->SetDefaultVisibleItems(gPercentList.GetCount());
  m_pComboProgress->InitializeList(nItem, nSubItem, sPercent, &gPercentList);
  m_pComboProgress->ShowWindow(SW_SHOW);
  m_pComboProgress->SetFocus();
  m_pComboProgress->ShowDropDown();
}
///////////////////////////////////////////////////////////////////////////////
// DrawEdit - Start edit of a sub item label
// nItem        - The row index of the item to edit
// nSubItem     - The column of the sub item.
void CXListCtrl::DrawEdit(int nItem, int nSubItem)
{
  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    ASSERT(FALSE);
    return;
  }

  if (!pXLCD[0].bEnabled){
    ASSERT(0);
    return;
  }


  CRect rect;
  CalculateCellRect(nItem, nSubItem, rect);

  m_strInitialString = GetItemText(nItem, nSubItem);

  if(!m_pEdit){
    DWORD dwStyle = WS_CHILDWINDOW | WS_VISIBLE | ES_AUTOHSCROLL | ES_LEFT | WS_BORDER;
    m_pEdit = new CInPlaceEdit();
    VERIFY(m_pEdit->Create(dwStyle, rect, this, 0));
  }else{
    ASSERT(m_pEdit->GetSafeHwnd());
    m_pEdit->MoveWindow(rect);
  }

  m_pEdit->SetWindowText(nItem, nSubItem, m_strInitialString);
  m_pEdit->ShowWindow(SW_SHOW);
  m_pEdit->SetFocus();
}

void CXListCtrl::OnHScroll(UINT iSBCode, UINT iPos, CScrollBar* pScrollBar) 
{
  // TODO: Add your message handler code here and/or call default

  if (GetFocus() != this)
  {
    SetFocus();
  }

  CListCtrl::OnHScroll(iSBCode, iPos, pScrollBar);
}

void CXListCtrl::OnVScroll(UINT iSBCode, UINT iPos, CScrollBar* pScrollBar) 
{
  // TODO: Add your message handler code here and/or call default

  if (GetFocus() != this)
  {
    SetFocus();
  }

  CListCtrl::OnVScroll(iSBCode, iPos, pScrollBar);
}

void CXListCtrl::OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult) 
{
  LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

  *pResult = 1;
}

void CXListCtrl::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult) 
{
  *pResult = 0;

  LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

  //see if this a progress?
  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(pDispInfo->item.iItem);
  if (!pXLCD){
    ASSERT(FALSE);
    return;
  }

  if(pXLCD[pDispInfo->item.iSubItem].bShowProgress){
    int percent = atoi(pDispInfo->item.pszText);    
    if(percent >= 0 && percent <= 100){
        pXLCD[pDispInfo->item.iSubItem].nProgressPercent = percent;
        
        //notify parent that this value has changed
        pDispInfo->hdr.code = LVN_LAST - 1;
        GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)pNMHDR);
    }
    return;
  }else if(pXLCD[pDispInfo->item.iSubItem].bShowTime){
    if(m_timeCtrl.GetTime() !=  pXLCD[pDispInfo->item.iSubItem].nTimeSeconds){
        pXLCD[pDispInfo->item.iSubItem].nTimeSeconds = m_timeCtrl.GetTime();
        UpdateSubItem(pDispInfo->item.iItem, pDispInfo->item.iSubItem);
        
        pDispInfo->hdr.code = LVN_LAST - 1;
        GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)pNMHDR);
    }
    
    return;
  }
  
  // ask parent to check
  CString sText = GetItemText(pDispInfo->item.iItem, pDispInfo->item.iSubItem);
  
  if(sText == pDispInfo->item.pszText){
      return;
  }

  pDispInfo->hdr.code = LVN_LAST;
  if(GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)pNMHDR)){
    // Update the item text with the new text
    SetItemText(pDispInfo->item.iItem, pDispInfo->item.iSubItem, pDispInfo->item.pszText);

    pDispInfo->hdr.code = LVN_LAST - 1;
    GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)pNMHDR);
  }
}


///////////////////////////////////////////////////////////////////////////////
// OnClick
BOOL CXListCtrl::OnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
  TRACE(_T("in CXListCtrl::OnClick\n"));

  pNMHDR   = pNMHDR;
  *pResult = 0;
  return FALSE;		// return FALSE to send message to parent also -
  // NOTE:  MSDN documentation is incorrect
}


void CXListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC;
    pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

    int nItem = lpDrawItemStruct->itemID;
    for(int nSubItem = 0; nSubItem < this->GetColumns(); nSubItem ++){
        DrawItem(pDC, nItem, nSubItem);
    }
}

///////////////////////////////////////////////////////////////////////////////
// OnCustomDraw
void CXListCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
  NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

  // Take the default processing unless we set this to something else below.
  *pResult = CDRF_DODEFAULT;

  // First thing - check the draw stage. If it's the control's prepaint
  // stage, then tell Windows we want messages for every item.

  if (pLVCD->nmcd.dwDrawStage == CDDS_PREPAINT)
  {
    *pResult = CDRF_NOTIFYITEMDRAW;
  }
  else if (pLVCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
  {
    // This is the notification message for an item.  We'll request
    // notifications before each subitem's prepaint stage.

    *pResult = CDRF_NOTIFYSUBITEMDRAW;
  }
  else if (pLVCD->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM))
  {
    // This is the prepaint stage for a subitem. Here's where we set the
    // item's text and background colors. Our return value will tell
    // Windows to draw the subitem itself, but it will use the new colors
    // we set here.

    int nItem = static_cast<int> (pLVCD->nmcd.dwItemSpec);
    int nSubItem = pLVCD->iSubItem;
    CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);

    DrawItem(pDC, nItem, nSubItem);
    
    *pResult = CDRF_SKIPDEFAULT;	// We've painted everything.
  }
}


void CXListCtrl::DrawItem(CDC *pDC, int nItem, int nSubItem)
{
    XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
    //XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) pLVCD->nmcd.lItemlParam;
    ASSERT(pXLCD);

    COLORREF crText  = m_crWindowText;
    COLORREF crBkgnd = m_crWindow;

    if (pXLCD)
    {
      crText  = pXLCD[nSubItem].crText;
      crBkgnd = pXLCD[nSubItem].crBackground;

      if (!pXLCD[0].bEnabled)
        crText = m_crGrayText;
    }

    // store the colors back in the NMLVCUSTOMDRAW struct
    //pLVCD->clrText = crText;
    //pLVCD->clrTextBk = crBkgnd;

    CRect rect;
    GetSubItemRect(nItem, nSubItem, LVIR_BOUNDS, rect);

    if (pXLCD && (pXLCD[nSubItem].bShowProgress))
    {
      DrawProgress(nItem, nSubItem, pDC, crText, crBkgnd, rect, pXLCD);
    }
    else if (pXLCD && (pXLCD[nSubItem].nCheckedState != -1))
    {
      DrawCheckbox(nItem, nSubItem, pDC, crText, crBkgnd, rect, pXLCD);
    }
    else if(pXLCD && (pXLCD[nSubItem].bShowTime))
    {
        //draw a timer here ...
      if (!rect.IsRectEmpty())
      {
        GetDrawColors(nItem, nSubItem, crText, crBkgnd);

        pDC->FillSolidRect(&rect, crBkgnd);

        int nWidth = 0;
        rect.left += m_HeaderCtrl.GetSpacing();
        
        if(m_reminder.GetSafeHandle()){
            CImageList* pImageList = &m_reminder;
            SIZE sizeImage;
            sizeImage.cx = sizeImage.cy = 0;
            IMAGEINFO info;

            int nImage = 0;
            if(pXLCD[nSubItem].bEnableTime != TRUE){
                nImage = 1;
            }

            if (pImageList->GetImageInfo(nImage, &info))
            {
              sizeImage.cx = info.rcImage.right - info.rcImage.left;
              sizeImage.cy = info.rcImage.bottom - info.rcImage.top;
            }

            POINT point;

            point.y = rect.CenterPoint().y - (sizeImage.cy >> 1);
            point.x = rect.left;

            SIZE size;
            size.cx = rect.Width() < sizeImage.cx ? rect.Width() : sizeImage.cx;
            size.cy = rect.Height() < sizeImage.cy ? rect.Height() : sizeImage.cy;

            // save image list background color
            COLORREF rgb = pImageList->GetBkColor();

            // set image list background color
            pImageList->SetBkColor(crBkgnd);
            pImageList->DrawIndirect(pDC, nImage, point, size, CPoint(0, 0));
            pImageList->SetBkColor(rgb);

            nWidth = sizeImage.cx + m_HeaderCtrl.GetSpacing();
            
            rect.left += nWidth;
         }
        
        pDC->FillSolidRect(&rect, crBkgnd);

        CTime t(pXLCD[nSubItem].nTimeSeconds);
        CString str;
        str.Format("%02d:%02d:%02d", t.GetHour(), t.GetMinute(), t .GetSecond());

        // get text justification
        HDITEM hditem;
        hditem.mask = HDI_FORMAT;
        m_HeaderCtrl.GetItem(nSubItem, &hditem);
        int nFmt = hditem.fmt & HDF_JUSTIFYMASK;
        UINT nFormat = DT_VCENTER | DT_SINGLELINE;
        if (m_bUseEllipsis)								//+++
          nFormat |= DT_END_ELLIPSIS;
        if (nFmt == HDF_CENTER)
          nFormat |= DT_CENTER;
        else if (nFmt == HDF_LEFT)
          nFormat |= DT_LEFT;
        else
          nFormat |= DT_RIGHT;

        CFont *pOldFont = NULL;
        CFont boldfont;

        // check if bold specified for subitem
        if (pXLCD && pXLCD[nSubItem].bBold)
        {
          CFont *font = pDC->GetCurrentFont();
          if (font)
          {
            LOGFONT lf;
            font->GetLogFont(&lf);
            lf.lfWeight = FW_BOLD;
            boldfont.CreateFontIndirect(&lf);
            pOldFont = pDC->SelectObject(&boldfont);
          }
        }
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(crText);
        pDC->SetBkColor(crBkgnd);
        rect.DeflateRect(m_nPadding, 0);			//+++
        pDC->DrawText(str, &rect, nFormat);
        rect.InflateRect(m_nPadding, 0);			//+++
        if (pOldFont)
          pDC->SelectObject(pOldFont);
        }
    }
    else
    {
      rect.left += DrawImage(nItem, nSubItem, pDC, crText, crBkgnd, rect, pXLCD);

      DrawText(nItem, nSubItem, pDC, crText, crBkgnd, rect, pXLCD);
    }
}

///////////////////////////////////////////////////////////////////////////////
// DrawProgress
void CXListCtrl::DrawProgress(int nItem,
                              int nSubItem,
                              CDC *pDC,
                              COLORREF crText,
                              COLORREF /*crBkgnd*/,
                              CRect& rect,
                              XLISTCTRLDATA *pXLCD)
{
  UNUSED_ALWAYS(nItem);

  ASSERT(pDC);
  ASSERT(pXLCD);

  if (rect.IsRectEmpty())
  {
    return;
  }

  rect.bottom -= 1;
  rect.left += 1;		// leave margin in case row is highlighted

  // fill interior with light gray
  pDC->FillSolidRect(rect, RGB(224,224,224));

  // draw border
  pDC->Draw3dRect(&rect, RGB(0,0,0), m_crBtnShadow);

  if (pXLCD[nSubItem].nProgressPercent >= 0)
  {
    // draw progress bar and text

    CRect LeftRect, RightRect;
    LeftRect = rect;
    LeftRect.left += 1;
    LeftRect.top += 1;
    LeftRect.bottom -= 1;
    RightRect = LeftRect;
    int w = (LeftRect.Width() * pXLCD[nSubItem].nProgressPercent) / 100;
    LeftRect.right = LeftRect.left + w - 1;
    RightRect.left = LeftRect.right;
    pDC->FillSolidRect(LeftRect, m_crHighLight);

    if (pXLCD[nSubItem].bShowProgressMessage)
    {
      CString str, format;
      format = pXLCD[nSubItem].strProgressMessage;
      if (format.IsEmpty())
        str.Format(_T("%d%%"), pXLCD[nSubItem].nProgressPercent);
      else
        str.Format(format, pXLCD[nSubItem].nProgressPercent);

      pDC->SetBkMode(TRANSPARENT);

      CRect TextRect;
      TextRect = rect;
      TextRect.DeflateRect(1, 1);

      CRgn rgn;
      rgn.CreateRectRgn(LeftRect.left, LeftRect.top, LeftRect.right, 
                        LeftRect.bottom);
      pDC->SelectClipRgn(&rgn);
      pDC->SetTextColor(m_crHighLightText);//crBkgnd);
      pDC->DrawText(str, &TextRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

      rgn.DeleteObject();
      rgn.CreateRectRgn(RightRect.left, RightRect.top, RightRect.right, 
                        RightRect.bottom);
      pDC->SelectClipRgn(&rgn);
      pDC->SetTextColor(crText);
      pDC->DrawText(str, &TextRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
      rgn.DeleteObject();
      pDC->SelectClipRgn(NULL);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// DrawCheckbox
void CXListCtrl::DrawCheckbox(int nItem,
                              int nSubItem,
                              CDC *pDC,
                              COLORREF crText,
                              COLORREF crBkgnd,
                              CRect& rect,
                              XLISTCTRLDATA *pXLCD)
{
  ASSERT(pDC);
  ASSERT(pXLCD);

  if (rect.IsRectEmpty())
  {
    return;
  }

  GetDrawColors(nItem, nSubItem, crText, crBkgnd);

  pDC->FillSolidRect(&rect, crBkgnd);

  CRect chkboxrect;
  chkboxrect = rect;
  chkboxrect.bottom -= 1;
  chkboxrect.left += 9;		// line up checkbox with header checkbox
  chkboxrect.right = chkboxrect.left + chkboxrect.Height();	// width = height

  CString str;
  str = GetItemText(nItem, nSubItem);

  if (str.IsEmpty())
  {
    // center the checkbox

    chkboxrect.left = rect.left + rect.Width()/2 - chkboxrect.Height()/2 - 1;
    chkboxrect.right = chkboxrect.left + chkboxrect.Height();
  }

  // fill rect around checkbox with white
  pDC->FillSolidRect(&chkboxrect, m_crWindow);

  // draw border
  CBrush brush(RGB(51,102,153));
  pDC->FrameRect(&chkboxrect, &brush);

  if (pXLCD[nSubItem].nCheckedState == 1)
  {
    CPen *pOldPen = NULL;

    CPen graypen(PS_SOLID, 1, m_crGrayText);
    CPen blackpen(PS_SOLID, 1, RGB(51,153,51));

    if (pXLCD[0].bEnabled)
      pOldPen = pDC->SelectObject(&blackpen);
    else
      pOldPen = pDC->SelectObject(&graypen);

    // draw the checkmark
    int x = chkboxrect.left + 9;
    ASSERT(x < chkboxrect.right);
    int y = chkboxrect.top + 3;
    int i;
    for (i = 0; i < 4; i++)
    {
      pDC->MoveTo(x, y);
      pDC->LineTo(x, y+3);
      x--;
      y++;
    }
    for (i = 0; i < 3; i++)
    {
      pDC->MoveTo(x, y);
      pDC->LineTo(x, y+3);
      x--;
      y--;
    }

    if (pOldPen)
      pDC->SelectObject(pOldPen);
  }

  if (!str.IsEmpty())
  {
    pDC->SetBkMode(TRANSPARENT);
    pDC->SetTextColor(crText);
    pDC->SetBkColor(crBkgnd);
    CRect textrect;
    textrect = rect;
    textrect.left = chkboxrect.right + 4;

    UINT nFormat = DT_LEFT | DT_VCENTER | DT_SINGLELINE;	//+++
    if (m_bUseEllipsis)
      nFormat |= DT_END_ELLIPSIS;

    pDC->DrawText(str, &textrect, nFormat);
  }
}

///////////////////////////////////////////////////////////////////////////////
// GetDrawColors
void CXListCtrl::GetDrawColors(int nItem,
                               int nSubItem,
                               COLORREF& colorText,
                               COLORREF& colorBkgnd)
{/*
  if(nItem % 2){
    colorBkgnd = RGB(240,247,249);
  }else{
    colorBkgnd = RGB(229,232,239);
  }

  return;
*/
  DWORD dwStyle    = GetStyle();
  DWORD dwExStyle  = GetExtendedStyle();

  COLORREF crText  = colorText;
  COLORREF crBkgnd = colorBkgnd;

  if (GetItemState(nItem, LVIS_SELECTED))
  {
    if (dwExStyle & LVS_EX_FULLROWSELECT)
    {
      // selected?  if so, draw highlight background
      crText  = m_crHighLightText;
      crBkgnd = m_crHighLight;

      // has focus?  if not, draw gray background
      if (m_hWnd != ::GetFocus())
      {
        if (dwStyle & LVS_SHOWSELALWAYS)
        {
          crText  = m_crWindowText;
          crBkgnd = m_crBtnFace;
        }
        else
        {
          crText  = colorText;
          crBkgnd = colorBkgnd;
        }
      }
    }
    else	// not full row select
    {
      if (nSubItem == 0)
      {
        // selected?  if so, draw highlight background
        crText  = m_crHighLightText;
        crBkgnd = m_crHighLight;

        // has focus?  if not, draw gray background
        if (m_hWnd != ::GetFocus())
        {
          if (dwStyle & LVS_SHOWSELALWAYS)
          {
            crText  = m_crWindowText;
            crBkgnd = m_crBtnFace;
          }
          else
          {
            crText  = colorText;
            crBkgnd = colorBkgnd;
          }
        }
      }
    }
  }

  colorText = crText;
  colorBkgnd = crBkgnd;
}

///////////////////////////////////////////////////////////////////////////////
// DrawImage
int CXListCtrl::DrawImage(int nItem,
                          int nSubItem,
                          CDC* pDC,
                          COLORREF crText,
                          COLORREF crBkgnd,
                          CRect rect,
                          XLISTCTRLDATA *pXLCD)
{
  if (rect.IsRectEmpty())
  {
    return 0;
  }

  GetDrawColors(nItem, nSubItem, crText, crBkgnd);

  pDC->FillSolidRect(&rect, crBkgnd);

  int nWidth = 0;
  rect.left += m_HeaderCtrl.GetSpacing();

  CImageList* pImageList = GetImageList(LVSIL_SMALL);
  if (pImageList)
  {
    SIZE sizeImage;
    sizeImage.cx = sizeImage.cy = 0;
    IMAGEINFO info;

    int nImage = -1;
    if (pXLCD)
      nImage = pXLCD[nSubItem].nImage;

    if (nImage == -1)
      return 0;

    if (pImageList->GetImageInfo(nImage, &info))
    {
      sizeImage.cx = info.rcImage.right - info.rcImage.left;
      sizeImage.cy = info.rcImage.bottom - info.rcImage.top;
    }

    if (nImage >= 0)
    {
      if (rect.Width() > 0)
      {
        POINT point;

        point.y = rect.CenterPoint().y - (sizeImage.cy >> 1);
        point.x = rect.left;

        SIZE size;
        size.cx = rect.Width() < sizeImage.cx ? rect.Width() : sizeImage.cx;
        size.cy = rect.Height() < sizeImage.cy ? rect.Height() : sizeImage.cy;

        // save image list background color
        COLORREF rgb = pImageList->GetBkColor();

        // set image list background color
        pImageList->SetBkColor(crBkgnd);
        pImageList->DrawIndirect(pDC, nImage, point, size, CPoint(0, 0));
        pImageList->SetBkColor(rgb);

        nWidth = sizeImage.cx + m_HeaderCtrl.GetSpacing();
      }
    }
  }

  return nWidth;
}

///////////////////////////////////////////////////////////////////////////////
// DrawText
void CXListCtrl::DrawText(int nItem,
                          int nSubItem,
                          CDC *pDC,
                          COLORREF crText,
                          COLORREF crBkgnd,
                          CRect& rect,
                          XLISTCTRLDATA *pXLCD)
{
  ASSERT(pDC);
  ASSERT(pXLCD);

  if(m_bHeaderIsSubclassed == false){
    SubclassHeaderControl();
  }

  if (rect.IsRectEmpty())
  {
    return;
  }

  GetDrawColors(nItem, nSubItem, crText, crBkgnd);

  pDC->FillSolidRect(&rect, crBkgnd);

  CString str;
  str = GetItemText(nItem, nSubItem);

  if (!str.IsEmpty())
  {
    // get text justification
    HDITEM hditem;
    hditem.mask = HDI_FORMAT;
    m_HeaderCtrl.GetItem(nSubItem, &hditem);
    int nFmt = hditem.fmt & HDF_JUSTIFYMASK;
    UINT nFormat = DT_VCENTER | DT_SINGLELINE;
    if (m_bUseEllipsis)								//+++
      nFormat |= DT_END_ELLIPSIS;
    if (nFmt == HDF_CENTER)
      nFormat |= DT_CENTER;
    else if (nFmt == HDF_LEFT)
      nFormat |= DT_LEFT;
    else
      nFormat |= DT_RIGHT;

    CFont *pOldFont = NULL;
    CFont boldfont;

    // check if bold specified for subitem
    if (pXLCD && pXLCD[nSubItem].bBold)
    {
      CFont *font = pDC->GetCurrentFont();
      if (font)
      {
        LOGFONT lf;
        font->GetLogFont(&lf);
        lf.lfWeight = FW_BOLD;
        boldfont.CreateFontIndirect(&lf);
        pOldFont = pDC->SelectObject(&boldfont);
      }
    }
    pDC->SetBkMode(TRANSPARENT);
    pDC->SetTextColor(crText);
    pDC->SetBkColor(crBkgnd);
    rect.DeflateRect(m_nPadding, 0);			//+++
    pDC->DrawText(str, &rect, nFormat);
    rect.InflateRect(m_nPadding, 0);			//+++
    if (pOldFont)
      pDC->SelectObject(pOldFont);
  }
}

///////////////////////////////////////////////////////////////////////////////
// GetSubItemRect
BOOL CXListCtrl::GetSubItemRect(int nItem,
                                int nSubItem,
                                int nArea,
                                CRect& rect)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return FALSE;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return FALSE;

  BOOL bRC = CListCtrl::GetSubItemRect(nItem, nSubItem, nArea, rect);

  // if nSubItem == 0, the rect returned by CListCtrl::GetSubItemRect
  // is the entire row, so use left edge of second subitem

  if (nSubItem == 0)
  {
    if (GetColumns() > 1)
    {
      CRect rect1;
      bRC = GetSubItemRect(nItem, 1, LVIR_BOUNDS, rect1);
      rect.right = rect1.left;
    }
  }

  //+++
  if (nSubItem == 0)
  {
    if (GetColumns() > 1)
    {
      CRect rect1;
      // in case 2nd col width = 0
      for (int i = 1; i < GetColumns(); i++)
      {
        bRC = GetSubItemRect(nItem, i, LVIR_BOUNDS, rect1);
        if (rect1.Width() > 0)
        {
          rect.right = rect1.left;
          break;
        }
      }
    }
  }

  return bRC;
}


bool CXListCtrl::HitTestEx(CPoint &obPoint, int* pRowIndex, int* pColumnIndex) const
{
  if (!pRowIndex || !pColumnIndex)
  {
    return false;
  }

  // Get the row index
  *pRowIndex = HitTest(obPoint, NULL);

  if (pColumnIndex)
  {
    *pColumnIndex = 0;
  }

  // Make sure that the ListView is in LVS_REPORT
  if ((GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT)
  {
    return false;
  }

  // Get the number of columns
  CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);

  int iColumnCount = pHeader->GetItemCount();

  // Get bounding rect of item and check whether obPoint falls in it.
  CRect obCellRect;
  //GetItemRect(*pRowIndex, &obCellRect, LVIR_LABEL);
  GetItemRect(*pRowIndex, &obCellRect, LVIR_BOUNDS);
	
  if (obCellRect.PtInRect(obPoint))
  {
    // Now find the column
    for (*pColumnIndex = 0; *pColumnIndex < iColumnCount; (*pColumnIndex)++)
    {
      int iColWidth = GetColumnWidth(*pColumnIndex);
			
      if (obPoint.x >= obCellRect.left && obPoint.x <= (obCellRect.left + iColWidth))
      {
        return true;
      }
      obCellRect.left += iColWidth;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
// OnLButtonDown
void CXListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
  CListCtrl::OnLButtonDown(nFlags, point);

  int nItem, nSubItem;
  if (!HitTestEx(point, &nItem, &nSubItem)){
    return;
  }

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD){
    return;
  }

  if (!pXLCD[0].bEnabled){
    return;
  }

  TRACE("Starting edit item: %d, sub item: %d.\n", nItem, nSubItem);

  //special handling for timer control
  if(pXLCD[nSubItem].bShowTime){
      if (!EnsureVisible(nItem, TRUE))
      {
        return;
      }
      
      IMAGEINFO info;
      m_reminder.GetImageInfo(0, &info);
      
      CRect rect;
      GetSubItemRect(nItem, nSubItem, LVIR_BOUNDS, rect);

      rect.right = rect.left + info.rcImage.right - info.rcImage.left + m_HeaderCtrl.GetSpacing();
      
      if(rect.PtInRect(point)){
        pXLCD[nSubItem].bEnableTime = (pXLCD[nSubItem].bEnableTime == TRUE) ? FALSE : TRUE;
        
        UpdateSubItem(nItem, nSubItem);
        
        // notify parent this has changed ...
          LV_DISPINFO dispinfo;
          dispinfo.hdr.hwndFrom = GetSafeHwnd();
          dispinfo.hdr.idFrom = GetDlgCtrlID();
          dispinfo.hdr.code = LVN_LAST - 1;
          
          dispinfo.item.mask = LVIF_TEXT;
          dispinfo.item.iItem = nItem;
          dispinfo.item.iSubItem = nSubItem;
          dispinfo.item.pszText = "";
          dispinfo.item.cchTextMax = 0;
      
        GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&dispinfo);
        return;
    }
  }
  
  StartEdit(nItem, nSubItem);
}

///////////////////////////////////////////////////////////////////////////////
// OnRButtonDown - added so we can ignore disabled items
void CXListCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
  TRACE(_T("in CXListCtrl::OnRButtonDown\n"));

  int nItem = -1;

  //+++
  LVHITTESTINFO lvhit;
  lvhit.pt = point;
  SubItemHitTest(&lvhit);
  if (lvhit.flags & LVHT_ONITEMLABEL)
  {
    TRACE(_T("lvhit.iItem=%d  lvhit.iSubItem=%d  ~~~~~\n"), lvhit.iItem, lvhit.iSubItem);

    nItem = lvhit.iItem;
  }

  if (nItem != -1)
  {
    XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
    if (!pXLCD)
    {
      return;
    }

    if (!pXLCD[0].bEnabled)
      return;
  }

  CListCtrl::OnRButtonDown(nFlags, point);
}

///////////////////////////////////////////////////////////////////////////////
// OnPaint
void CXListCtrl::OnPaint()
{
  Default();
  if (GetItemCount() <= 0)
  {
    CDC* pDC = GetDC();
    int nSavedDC = pDC->SaveDC();

    CRect rc;
    GetWindowRect(&rc);
    ScreenToClient(&rc);
    CHeaderCtrl* pHC = GetHeaderCtrl();
    if (pHC != NULL)
    {
      CRect rcH;
      pHC->GetItemRect(0, &rcH);
      rc.top += rcH.bottom;
    }
    rc.top += 10;
    CString strText = m_sTipEmptyView;

    COLORREF crText = m_crWindowText;
    COLORREF crBkgnd = m_crWindow;

    CBrush brush(crBkgnd);
    pDC->FillRect(rc, &brush);

    pDC->SetTextColor(crText);
    pDC->SetBkColor(crBkgnd);
    pDC->SelectStockObject(ANSI_VAR_FONT);
    pDC->DrawText(strText, -1, rc, DT_CENTER | DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP);
    pDC->RestoreDC(nSavedDC);
    ReleaseDC(pDC);
  }
}

///////////////////////////////////////////////////////////////////////////////
// InsertItem
int CXListCtrl::InsertItem(const LVITEM* pItem)
{
  ASSERT(pItem->iItem >= 0);
  if (pItem->iItem < 0)
    return -1;

  int index = CListCtrl::InsertItem(pItem);

  if (index < 0)
    return index;

  XLISTCTRLDATA *pXLCD = new XLISTCTRLDATA [GetColumns()];
  ASSERT(pXLCD);
  if (!pXLCD)
    return -1;

  pXLCD[0].crText       = m_crWindowText;
  pXLCD[0].crBackground = m_crWindow;
  pXLCD[0].nImage       = pItem->iImage;
  pXLCD[0].dwItemData   = pItem->lParam;	//+++

  CListCtrl::SetItemData(index, (DWORD) pXLCD);

  return index;
}

///////////////////////////////////////////////////////////////////////////////
// InsertItem
int CXListCtrl::InsertItem(int nItem, LPCTSTR lpszItem)
{
  ASSERT(nItem >= 0);
  if (nItem < 0)
    return -1;

  return InsertItem(nItem,
                    lpszItem,
                    m_crWindowText,
                    m_crWindow);
}

///////////////////////////////////////////////////////////////////////////////
// InsertItem
int CXListCtrl::InsertItem(int nItem,
                           LPCTSTR lpszItem,
                           COLORREF crText,
                           COLORREF crBackground)
{
  ASSERT(nItem >= 0);
  if (nItem < 0)
    return -1;

  int index = CListCtrl::InsertItem(nItem, lpszItem);

  if (index < 0)
    return index;

  XLISTCTRLDATA *pXLCD = new XLISTCTRLDATA [GetColumns()];
  ASSERT(pXLCD);
  if (!pXLCD)
    return -1;

  pXLCD[0].crText       = crText;
  pXLCD[0].crBackground = crBackground;
  pXLCD[0].nImage       = -1;

  CListCtrl::SetItemData(index, (DWORD) pXLCD);

  return index;
}

///////////////////////////////////////////////////////////////////////////////
// SetItem
int CXListCtrl::SetItem(const LVITEM* pItem)
{
  ASSERT(pItem->iItem >= 0);
  if (pItem->iItem < 0)
    return -1;

  BOOL rc = CListCtrl::SetItem(pItem);

  if (!rc)
    return FALSE;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(pItem->iItem);
  if (pXLCD)
  {
    pXLCD[pItem->iSubItem].nImage = pItem->iImage;
    UpdateSubItem(pItem->iItem, pItem->iSubItem);
    rc = TRUE;
  }
  else
  {
    rc = FALSE;
  }

  return rc;
}

///////////////////////////////////////////////////////////////////////////////
// SetItemImage
BOOL CXListCtrl::SetItemImage(int nItem, int nSubItem, int nImage)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return FALSE;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return FALSE;

  BOOL rc = TRUE;

  if (nItem < 0)
    return FALSE;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (pXLCD)
  {
    pXLCD[nSubItem].nImage = nImage;
  }

  UpdateSubItem(nItem, nSubItem);

  return rc;
}

///////////////////////////////////////////////////////////////////////////////
// SetItemText
BOOL CXListCtrl::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return FALSE;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return FALSE;

  BOOL rc = CListCtrl::SetItemText(nItem, nSubItem, lpszText);

  UpdateSubItem(nItem, nSubItem);

  return rc;
}

///////////////////////////////////////////////////////////////////////////////
// SetItemText
//
// This function will set the text and colors for a subitem.  If lpszText
// is NULL, only the colors will be set.  If a color value is -1, the display
// color will be set to the default Windows color.
//
BOOL CXListCtrl::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText,
                             COLORREF crText, COLORREF crBackground)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return FALSE;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return FALSE;

  BOOL rc = TRUE;

  if (nItem < 0)
    return FALSE;

  if (lpszText)
    rc = CListCtrl::SetItemText(nItem, nSubItem, lpszText);

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (pXLCD)
  {
    pXLCD[nSubItem].crText       = (crText == -1) ? m_crWindowText : crText;
    pXLCD[nSubItem].crBackground = (crBackground == -1) ? m_crWindow : crBackground;
  }

  UpdateSubItem(nItem, nSubItem);

  return rc;
}

///////////////////////////////////////////////////////////////////////////////
// DeleteItem
BOOL CXListCtrl::DeleteItem(int nItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return FALSE;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (pXLCD)
    delete [] pXLCD;
  CListCtrl::SetItemData(nItem, 0);
  return CListCtrl::DeleteItem(nItem);
}

///////////////////////////////////////////////////////////////////////////////
// DeleteAllItems
BOOL CXListCtrl::DeleteAllItems()
{
  int n = GetItemCount();
  for (int i = 0; i < n; i++)
  {
    XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(i);
    if (pXLCD)
      delete [] pXLCD;
    CListCtrl::SetItemData(i, 0);
  }

  return CListCtrl::DeleteAllItems();
}

///////////////////////////////////////////////////////////////////////////////
// OnDestroy
void CXListCtrl::OnDestroy()
{
  int n = GetItemCount();
  for (int i = 0; i < n; i++)
  {
    XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(i);
    if (pXLCD)
      delete [] pXLCD;
    CListCtrl::SetItemData(i, 0);
  }

  m_bHeaderIsSubclassed = FALSE;

  CListCtrl::OnDestroy();
}

///////////////////////////////////////////////////////////////////////////////
// SetEdit
BOOL CXListCtrl::SetEdit(int nItem, int nSubItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return FALSE;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return FALSE;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return FALSE;
  }

  pXLCD[nSubItem].bEdit = TRUE;

  UpdateSubItem(nItem, nSubItem);

  return TRUE;
}

void CXListCtrl::EnableTime(int nItem, int nSubItem, bool bEnable)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return;
  }
  
  if(pXLCD[nSubItem].bShowTime != TRUE){
    return;
  }
  
  pXLCD[nSubItem].bEnableTime = bEnable ? TRUE : FALSE;
  
  UpdateSubItem(nItem, nSubItem);
}

void CXListCtrl::UpdateTime(int nItem, int nSubItem, UINT t)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return;
  }
  
  if(pXLCD[nSubItem].bShowTime != TRUE){
    return;
  }
  
  pXLCD[nSubItem].nTimeSeconds = t;
  
  UpdateSubItem(nItem, nSubItem);
}

bool CXListCtrl::IsTimeEnabled(int nItem, int nSubItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return false;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return false;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return false;
  }
  
  if(pXLCD[nSubItem].bShowTime != TRUE){
    return false;
  }
  
  return (pXLCD[nSubItem].bEnableTime == TRUE);
}

time_t CXListCtrl::RetrieveTime(int nItem, int nSubItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return 0;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return 0;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return 0;
  }
  
  if(pXLCD[nSubItem].bShowTime != TRUE){
    return 0;
  }
  
  return pXLCD[nSubItem].nTimeSeconds;
}

void CXListCtrl::SetReminderImages(UINT nImage)
{
    m_reminder.Create(nImage, 16, 2, RGB(255, 0, 255));
    ASSERT(m_reminder.GetImageCount() >= 2);
}

void CXListCtrl::SetTimeCtrl(int nItem, int nSubItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return;
  }

  pXLCD[nSubItem].bShowTime        = TRUE;
  pXLCD[nSubItem].nTimeSeconds     = time(0);

  UpdateSubItem(nItem, nSubItem);
}

///////////////////////////////////////////////////////////////////////////////
// SetProgress
//
// This function creates a progress bar in the specified subitem.  The
// UpdateProgress function may then be called to update the progress
// percent.  If bShowProgressText is TRUE, either the default text
// of "n%" or the custom percent text (lpszProgressText) will be
// displayed.  If bShowProgressText is FALSE, only the progress bar
// will be displayed, with no text.
//
// Note that the lpszProgressText string should include the format
// specifier "%d":  e.g., "Pct %d%%"
//
BOOL CXListCtrl::SetProgress(int nItem,
                             int nSubItem,
                             BOOL bShowProgressText /*= TRUE*/,
                             LPCTSTR lpszProgressText /*= NULL*/)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return FALSE;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return FALSE;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return FALSE;
  }

  pXLCD[nSubItem].bShowProgress        = TRUE;
  pXLCD[nSubItem].nProgressPercent     = 0;
  pXLCD[nSubItem].bShowProgressMessage = bShowProgressText;
  pXLCD[nSubItem].strProgressMessage   = lpszProgressText;

  UpdateSubItem(nItem, nSubItem);

  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// DeleteProgress
void CXListCtrl::DeleteProgress(int nItem, int nSubItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return;
  }

  pXLCD[nSubItem].bShowProgress = FALSE;
  pXLCD[nSubItem].nProgressPercent = 0;

  UpdateSubItem(nItem, nSubItem);
}

///////////////////////////////////////////////////////////////////////////////
// UpdateProgress
void CXListCtrl::UpdateProgress(int nItem, int nSubItem, int nPercent)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return;

  ASSERT(nPercent >= 0 && nPercent <= 100);

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return;
  }

  pXLCD[nSubItem].nProgressPercent = nPercent;

  UpdateSubItem(nItem, nSubItem);
}

int   CXListCtrl::RetrieveProgress(int nItem, int nSubItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return 0;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return 0;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return 0;
  }

  return pXLCD[nSubItem].nProgressPercent;
}

///////////////////////////////////////////////////////////////////////////////
// SetCheckbox
BOOL CXListCtrl::SetCheckbox(int nItem, int nSubItem, int nCheckedState)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return FALSE;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return FALSE;
  ASSERT(nCheckedState == 0 || nCheckedState == 1 || nCheckedState == -1);

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return FALSE;
  }

  // update checkbox in subitem

  pXLCD[nSubItem].nCheckedState = nCheckedState;

  UpdateSubItem(nItem, nSubItem);

  // now update checkbox in column header

  // -1 = no checkbox in column header
  if (GetHeaderCheckedState(nSubItem) != XHEADERCTRL_NO_IMAGE)
  {
    int nCheckedCount = CountCheckedItems(nSubItem);

    if (nCheckedCount == GetItemCount())
      SetHeaderCheckedState(nSubItem, XHEADERCTRL_CHECKED_IMAGE);
    else
      SetHeaderCheckedState(nSubItem, XHEADERCTRL_UNCHECKED_IMAGE);
  }

  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// GetCheckbox
int CXListCtrl::GetCheckbox(int nItem, int nSubItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return -1;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return -1;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return -1;
  }

  return pXLCD[nSubItem].nCheckedState;
}

///////////////////////////////////////////////////////////////////////////////
// GetEnabled
//
// Note that GetEnabled and SetEnabled only Get/Set the enabled flag from
// subitem 0, since this is a per-row flag.
//
BOOL CXListCtrl::GetEnabled(int nItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return FALSE;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return FALSE;
  }

  return pXLCD[0].bEnabled;
}

///////////////////////////////////////////////////////////////////////////////
// SetEnabled
BOOL CXListCtrl::SetEnabled(int nItem, BOOL bEnable)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return FALSE;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return FALSE;
  }

  pXLCD[0].bEnabled = bEnable;

  CRect rect;
  GetItemRect(nItem, &rect, LVIR_BOUNDS);
  InvalidateRect(&rect);
  UpdateWindow();

  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// SetBold
BOOL CXListCtrl::SetBold(int nItem, int nSubItem, BOOL bBold)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return FALSE;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return FALSE;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return FALSE;
  }

  // update bold flag
  pXLCD[nSubItem].bBold = bBold;

  UpdateSubItem(nItem, nSubItem);

  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// GetBold
BOOL CXListCtrl::GetBold(int nItem, int nSubItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return FALSE;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return FALSE;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return FALSE;
  }

  // return bold flag
  return pXLCD[nSubItem].bBold;
}

///////////////////////////////////////////////////////////////////////////////
// GetModified
BOOL CXListCtrl::GetModified(int nItem, int nSubItem)					//+++
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return FALSE;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return FALSE;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return FALSE;
  }

  // return modified flag
  return pXLCD[nSubItem].bModified;
}

///////////////////////////////////////////////////////////////////////////////
// SetModified
void CXListCtrl::SetModified(int nItem, int nSubItem, BOOL bModified)	//+++
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (pXLCD)
  {
    // set modified flag
    pXLCD[nSubItem].bModified = bModified;
  }
}

///////////////////////////////////////////////////////////////////////////////
// GetItemCheckedState
int CXListCtrl::GetItemCheckedState(int nItem, int nSubItem)		//+++
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return -1;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return -1;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return -1;
  }

  // return checked state
  return pXLCD[nSubItem].nCheckedState;
}

///////////////////////////////////////////////////////////////////////////////
// SetItemCheckedState
void CXListCtrl::SetItemCheckedState(int nItem, int nSubItem, int nCheckedState)		//+++
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (pXLCD)
  {
    // update checked state
    pXLCD[nSubItem].nCheckedState = nCheckedState;

    UpdateSubItem(nItem, nSubItem);
  }
}

///////////////////////////////////////////////////////////////////////////////
// GetItemColors
BOOL CXListCtrl::GetItemColors(int nItem,								//+++
                               int nSubItem, 
                               COLORREF& crText, 
                               COLORREF& crBackground)
{
  crText = RGB(0,0,0);
  crBackground = RGB(0,0,0);
	
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return FALSE;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return FALSE;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return FALSE;
  }

  crText = pXLCD[nSubItem].crText;
  crBackground = pXLCD[nSubItem].crBackground;

  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// SetItemColors
void CXListCtrl::SetItemColors(int nItem,								//+++
                               int nSubItem, 
                               COLORREF crText, 
                               COLORREF crBackground)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (pXLCD)
  {
    pXLCD[nSubItem].crText = crText;
    pXLCD[nSubItem].crBackground = (crBackground == -1) ? m_crWindow : crBackground;
  }
}

///////////////////////////////////////////////////////////////////////////////
// SetComboBox
//
// Note:  SetItemText may also be used to set the initial combo selection.
//
BOOL CXListCtrl::SetComboBox(int nItem,
                             int nSubItem,
                             BOOL bEnableCombo,
                             CStringArray *psa,		// should not be allocated on stack
                             int nComboListHeight,
                             int nInitialComboSel,
                             BOOL bSort /*= FALSE*/)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return FALSE;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return FALSE;
  ASSERT(psa);
  if (!psa)
    return FALSE;
  ASSERT(nComboListHeight > 0);
  ASSERT(nInitialComboSel >= 0 && nInitialComboSel < psa->GetSize());
  if ((nInitialComboSel < 0) || (nInitialComboSel >= psa->GetSize()))
    nInitialComboSel = 0;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return FALSE;
  }

  // update flag
  pXLCD[nSubItem].bCombo = bEnableCombo;

  if (bEnableCombo)
  {
    // sort CStringArray before setting initial selection
    if (bSort)
      CSortCStringArray::SortCStringArray(psa);

    pXLCD[nSubItem].psa = psa;
    pXLCD[nSubItem].nComboListHeight = nComboListHeight;
    pXLCD[nSubItem].nInitialComboSel = nInitialComboSel;
    pXLCD[nSubItem].bSort = bSort;

    CString str = _T("");

    if ((pXLCD[nSubItem].nInitialComboSel >= 0) &&
        (pXLCD[nSubItem].psa->GetSize() > pXLCD[nSubItem].nInitialComboSel))
    {
      int index = pXLCD[nSubItem].nInitialComboSel;
      str = pXLCD[nSubItem].psa->GetAt(index);
    }

    SetItemText(nItem, nSubItem, str);
  }

  UpdateSubItem(nItem, nSubItem);

  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// GetComboText
//
// Actually this does nothing more than GetItemText()
//
CString	CXListCtrl::GetComboText(int nItem, int nSubItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return _T("");
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return _T("");

  CString str;
  str = _T("");

  str = GetItemText(nItem, nSubItem);

  return str;
}

///////////////////////////////////////////////////////////////////////////////
// SetCurSel
BOOL CXListCtrl::SetCurSel(int nItem, BOOL bEnsureVisible /*= FALSE*/)
{
  BOOL bRet = SetItemState(nItem, LVIS_FOCUSED | LVIS_SELECTED,
                           LVIS_FOCUSED | LVIS_SELECTED);

  //+++
  if (bEnsureVisible)
    EnsureVisible(nItem, FALSE);

  return bRet;
}

///////////////////////////////////////////////////////////////////////////////
// GetCurSel - returns selected item number, or -1 if no item selected
//
// Note:  for single-selection lists only
//
int CXListCtrl::GetCurSel()
{
  POSITION pos = GetFirstSelectedItemPosition();
  int nSelectedItem = -1;
  if (pos != NULL)
    nSelectedItem = GetNextSelectedItem(pos);
  return nSelectedItem;
}

///////////////////////////////////////////////////////////////////////////////
// UpdateSubItem
void CXListCtrl::UpdateSubItem(int nItem, int nSubItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return;

  CRect rect;
  if (nSubItem == -1)
  {
    GetItemRect(nItem, &rect, LVIR_BOUNDS);
  }
  else
  {
    GetSubItemRect(nItem, nSubItem, LVIR_BOUNDS, rect);
  }

  rect.InflateRect(2, 2);

  InvalidateRect(&rect);
  UpdateWindow();
}

///////////////////////////////////////////////////////////////////////////////
// GetColumns
int CXListCtrl::GetColumns()
{
  return GetHeaderCtrl()->GetItemCount();
}

///////////////////////////////////////////////////////////////////////////////
// GetItemData
//
// The GetItemData and SetItemData functions allow for app-specific data
// to be stored, by using an extra field in the XLISTCTRLDATA struct.
//
DWORD CXListCtrl::GetItemData(int nItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return 0;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return 0;
  }

  return pXLCD->dwItemData;
}

///////////////////////////////////////////////////////////////////////////////
// SetItemData
BOOL CXListCtrl::SetItemData(int nItem, DWORD dwData)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return FALSE;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return FALSE;
  }

  pXLCD->dwItemData = dwData;

  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// GetHeaderCheckedState
//
// The GetHeaderCheckedState and SetHeaderCheckedState may be used to toggle
// the checkbox in a column header.
//     0 = no checkbox
//     1 = unchecked
//     2 = checked
//
int CXListCtrl::GetHeaderCheckedState(int nSubItem)
{
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return -1;

  HDITEM hditem;

  // use the image index (0 or 1) to indicate the checked status
  hditem.mask = HDI_IMAGE;
  m_HeaderCtrl.GetItem(nSubItem, &hditem);
  return hditem.iImage;
}

///////////////////////////////////////////////////////////////////////////////
// SetHeaderCheckedState
BOOL CXListCtrl::SetHeaderCheckedState(int nSubItem, int nCheckedState)
{
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return FALSE;
  ASSERT(nCheckedState == 0 || nCheckedState == 1 || nCheckedState == 2);

  HDITEM hditem;

  hditem.mask = HDI_IMAGE;
  hditem.iImage = nCheckedState;
  m_HeaderCtrl.SetItem(nSubItem, &hditem);

  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// OnColumnClick
BOOL CXListCtrl::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
  TRACE(_T("in CXListCtrl::OnColumnClick\n"));

  NMLISTVIEW* pnmlv = (NMLISTVIEW*)pNMHDR;

  int nSubItem = pnmlv->iSubItem;

  int nCheckedState = GetHeaderCheckedState(nSubItem);

  // 0 = no checkbox
  if (nCheckedState != XHEADERCTRL_NO_IMAGE)
  {
    nCheckedState = (nCheckedState == 1) ? 2 : 1;
    SetHeaderCheckedState(nSubItem, nCheckedState);

    m_HeaderCtrl.UpdateWindow();

    for (int nItem = 0; nItem < GetItemCount(); nItem++)
    {
      XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
      if (!pXLCD)
      {
        continue;
      }

      if (pXLCD[nSubItem].nCheckedState != -1)
      {
        pXLCD[nSubItem].nCheckedState = nCheckedState - 1;
        pXLCD[nSubItem].bModified = TRUE;
        m_bListModified = TRUE;
        UpdateSubItem(nItem, nSubItem);
      }
    }
  }

  *pResult = 0;
  return FALSE;		// return FALSE to send message to parent also -
  // NOTE:  MSDN documentation is incorrect
}

///////////////////////////////////////////////////////////////////////////////
// CountCheckedItems
int CXListCtrl::CountCheckedItems(int nSubItem)
{
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return 0;

  int nCount = 0;

  for (int nItem = 0; nItem < GetItemCount(); nItem++)
  {
    XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
    if (!pXLCD)
    {
      continue;
    }

    if (pXLCD[nSubItem].nCheckedState == 1)
      nCount++;
  }

  return nCount;
}

///////////////////////////////////////////////////////////////////////////////
// GetColors
void CXListCtrl::GetColors()
{
  m_cr3DFace              = ::GetSysColor(COLOR_3DFACE);
  m_cr3DHighLight         = ::GetSysColor(COLOR_3DHIGHLIGHT);
  m_cr3DShadow            = ::GetSysColor(COLOR_3DSHADOW);
  m_crActiveCaption       = ::GetSysColor(COLOR_ACTIVECAPTION);
  m_crBtnFace             = ::GetSysColor(COLOR_BTNFACE);
  m_crBtnShadow           = ::GetSysColor(COLOR_BTNSHADOW);
  m_crBtnText             = ::GetSysColor(COLOR_BTNTEXT);
  m_crGrayText            = ::GetSysColor(COLOR_GRAYTEXT);
  m_crHighLight           = ::GetSysColor(COLOR_HIGHLIGHT);
  m_crHighLightText       = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
  m_crInactiveCaption     = ::GetSysColor(COLOR_INACTIVECAPTION);
  m_crInactiveCaptionText = ::GetSysColor(COLOR_INACTIVECAPTIONTEXT);
  m_crWindow              = ::GetSysColor(COLOR_WINDOW);
  m_crWindowText          = ::GetSysColor(COLOR_WINDOWTEXT);
}

///////////////////////////////////////////////////////////////////////////////
// OnSysColorChange
void CXListCtrl::OnSysColorChange()
{
  TRACE(_T("in CXListCtrl::OnSysColorChange\n"));

  CListCtrl::OnSysColorChange();

  GetColors();
}

///////////////////////////////////////////////////////////////////////////////
// OnToolHitTest
int CXListCtrl::OnToolHitTest(CPoint point, TOOLINFO * pTI) const
{
  LVHITTESTINFO lvhitTestInfo;
	
  lvhitTestInfo.pt = point;
	
  int nItem = ListView_SubItemHitTest(this->m_hWnd, &lvhitTestInfo);
  int nSubItem = lvhitTestInfo.iSubItem;
  //TRACE(_T("in CToolTipListCtrl::OnToolHitTest: %d,%d\n"), nItem, nSubItem);

  UINT nFlags = lvhitTestInfo.flags;

  // nFlags is 0 if the SubItemHitTest fails
  // Therefore, 0 & <anything> will equal false
  if (nFlags & LVHT_ONITEMLABEL)
  {
    // If it did fall on a list item,
    // and it was also hit one of the
    // item specific subitems we wish to show tool tips for
		
    // get the client (area occupied by this control
    RECT rcClient;
    GetClientRect(&rcClient);
		
    // fill in the TOOLINFO structure
    pTI->hwnd = m_hWnd;
    pTI->uId = (UINT) (nItem * 1000 + nSubItem + 1);
    pTI->lpszText = LPSTR_TEXTCALLBACK;
    pTI->rect = rcClient;
		
    return pTI->uId;	// By returning a unique value per listItem,
    // we ensure that when the mouse moves over another
    // list item, the tooltip will change
  }
  else
  {
    //Otherwise, we aren't interested, so let the message propagate
    return -1;
  }
}

///////////////////////////////////////////////////////////////////////////////
// OnToolTipText
BOOL CXListCtrl::OnToolTipText(UINT /*id*/, NMHDR * pNMHDR, LRESULT * pResult)
{
  UINT nID = pNMHDR->idFrom;
  //TRACE(_T("in CXListCtrl::OnToolTipText: id=%d\n"), nID);
	
  // check if this is the automatic tooltip of the control
  if (nID == 0) 
    return TRUE;	// do not allow display of automatic tooltip,
  // or our tooltip will disappear
	
  // handle both ANSI and UNICODE versions of the message
  TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
  TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	
  *pResult = 0;
	
  // get the mouse position
  const MSG* pMessage;
  pMessage = GetCurrentMessage();
  ASSERT(pMessage);
  CPoint pt;
  pt = pMessage->pt;		// get the point from the message
  ScreenToClient(&pt);	// convert the point's coords to be relative to this control
	
  // see if the point falls onto a list item
	
  LVHITTESTINFO lvhitTestInfo;
	
  lvhitTestInfo.pt = pt;
	
  int nItem = SubItemHitTest(&lvhitTestInfo);
  int nSubItem = lvhitTestInfo.iSubItem;
	
  UINT nFlags = lvhitTestInfo.flags;
	
  // nFlags is 0 if the SubItemHitTest fails
  // Therefore, 0 & <anything> will equal false
  if (nFlags & LVHT_ONITEMLABEL)
  {
    // If it did fall on a list item,
    // and it was also hit one of the
    // item specific subitems we wish to show tooltips for
		
    CString strToolTip;
    strToolTip = _T("");

    XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
    if (pXLCD)
    {
      strToolTip = pXLCD[nSubItem].strToolTip;
    }

    if (!strToolTip.IsEmpty())
    {
      // If there was a CString associated with the list item,
      // copy it's text (up to 80 characters worth, limitation 
      // of the TOOLTIPTEXT structure) into the TOOLTIPTEXT 
      // structure's szText member
			
#ifndef _UNICODE
        if (pNMHDR->code == TTN_NEEDTEXTA){
            static char tip[1025];
            strncpy(tip, strToolTip, 1024);
            //lstrcpyn(pTTTA->szText, strToolTip, 80);
            pTTTA->lpszText = tip;
        }else{
            static wchar_t tip[1025];
            //_mbstowcsz(pTTTW->szText, strToolTip, 80);
            _mbstowcsz(tip, strToolTip, 1024);
            pTTTW->lpszText = tip;
        }
#else
      if (pNMHDR->code == TTN_NEEDTEXTA)
        _wcstombsz(pTTTA->szText, strToolTip, 80);
      else
        lstrcpyn(pTTTW->szText, strToolTip, 80);
#endif
      return FALSE;	 // we found a tool tip,
    }
  }
	
  return FALSE;	// we didn't handle the message, let the 
  // framework continue propagating the message
}

///////////////////////////////////////////////////////////////////////////////
// SetItemToolTipText
BOOL CXListCtrl::SetItemToolTipText(int nItem, int nSubItem, LPCTSTR lpszToolTipText)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return FALSE;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return FALSE;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD)
  {
    return FALSE;
  }

  pXLCD[nSubItem].strToolTip = lpszToolTipText;

  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// GetItemToolTipText
CString CXListCtrl::GetItemToolTipText(int nItem, int nSubItem)
{
  CString strToolTip;
  strToolTip = _T("");

  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount())
    return strToolTip;
  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns())
    return strToolTip;

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (pXLCD)
  {
    strToolTip = pXLCD[nSubItem].strToolTip;
  }

  return strToolTip;
}

///////////////////////////////////////////////////////////////////////////////
// DeleteAllToolTips
void CXListCtrl::DeleteAllToolTips()
{
  int nRow = GetItemCount();
  int nCol = GetColumns();

  for (int nItem = 0; nItem < nRow; nItem++)
  {
    XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
    if (pXLCD)
      for (int nSubItem = 0; nSubItem < nCol; nSubItem++)
        pXLCD[nSubItem].strToolTip = _T("");
  }
}


///////////////////////////////////////////////////////////////////////////////
// OnEraseBkgnd
BOOL CXListCtrl::OnEraseBkgnd(CDC* pDC) 
{
/*
  CRect rect;
  GetClientRect(rect);


  POINT mypoint;  
  
  CBrush brush0(RGB(240,247,249));
  CBrush brush1(RGB(229,232,239));
 
  int chunk_height=GetCountPerPage();
  pDC->FillRect(&rect,&brush1);

  for (int i=0;i<=chunk_height;i++)
  {
    GetItemPosition(i,&mypoint);
    rect.top=mypoint.y ;
    GetItemPosition(i+1,&mypoint);
    rect.bottom =mypoint.y;
    pDC->FillRect(&rect,i %2 ? &brush1 : &brush0);
  }

  brush0.DeleteObject();
  brush1.DeleteObject();
*/
  CRect rectClip, rectTop, rectBottom, rectRight;
  int nItemCount = GetItemCount();

  if (!nItemCount) // Empty XListCtrl, nothing to do, CListCtrl will
    return CListCtrl::OnEraseBkgnd(pDC); // erase the Background

  if (pDC->GetClipBox(&rectClip) == ERROR)
  {
    ASSERT(false);
    return CListCtrl::OnEraseBkgnd(pDC);
  }

  int nFirstRow = GetTopIndex();
  int nLastRow = nFirstRow + GetCountPerPage();
  nLastRow = min (nLastRow, nItemCount - 1); // Last Item displayed in Ctrl

  CListCtrl::GetSubItemRect(nFirstRow, 0, LVIR_BOUNDS, rectTop);
  CListCtrl::GetSubItemRect(nLastRow, 0, LVIR_BOUNDS, rectBottom);

  CRect rectEraseTop = rectClip;
  rectEraseTop.bottom = rectTop.top;
  pDC->FillSolidRect(rectEraseTop, m_crWindow);

  CRect rectEraseBottom = rectClip;
  rectEraseBottom.top = rectBottom.bottom;
  pDC->FillSolidRect(rectEraseBottom, m_crWindow);

  CRect rectEraseRight = rectClip;
  rectEraseRight.top = rectTop.top;
  rectEraseRight.bottom = rectBottom.bottom;
  rectEraseRight.left = rectTop.right;
  pDC->FillSolidRect(rectEraseRight, m_crWindow);

  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// FindDataItem
//+++
int CXListCtrl::FindDataItem(DWORD dwData)
{
  for (int nItem = 0; nItem < GetItemCount(); nItem++) 
  {
    if (GetItemData(nItem) == dwData)
      return nItem;
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////
// OnKeyDown - check for disabled items
void CXListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  TRACE(_T("in CXListCtrl::OnKeyDown\n"));

  int nOldItem = GetCurSel();

  CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);

  if(nChar == VK_SHIFT || nChar == VK_CONTROL) {
      return;
  }

  int nNewItem = GetCurSel();

  if (nNewItem >= 0)
  {
    int nTrial = nNewItem;

    BOOL bEnabled = GetEnabled(nTrial);

    if (!bEnabled)
    {
      // item is disabled, try another

      int nCount = GetItemCount();

      if (nChar == VK_DOWN || nChar == VK_NEXT)
      {
        int nDirection = 1;

        while (!bEnabled)
        {
          nTrial += 1 * nDirection;

          if (nTrial >= nCount)
          {
            // at the end, back up
            nTrial = nCount;
            nDirection = -1;
            continue;
          }
          else if (nTrial < 0)
          {
            // at beginning - must have been backing up
            nTrial = nOldItem;
            break;
          }

          bEnabled = GetEnabled(nTrial);
        }
      }
      else if (nChar == VK_UP || nChar == VK_PRIOR)
      {
        int nDirection = -1;

        while (!bEnabled)
        {
          nTrial += 1 * nDirection;

          if (nTrial < 0)
          {
            // at the beginning, go forward
            nTrial = 0;
            nDirection = 1;
            continue;
          }
          else if (nTrial >= nCount)
          {
            // at end - must have been going forward
            nTrial = nOldItem;
            break;
          }

          bEnabled = GetEnabled(nTrial);
        }
      }
      else
      {
        // don't know how user got here, just go back to previous
        nTrial = nOldItem;
      }
    }

    SetCurSel(nTrial, TRUE);	// set new selection, scroll into view
  }
}


void CXListCtrl::StartEdit(int nItem, int nSubItem)
{
#ifdef _DEBUG
  DWORD dwExStyle = GetExtendedStyle();
  if ((dwExStyle & LVS_EX_FULLROWSELECT) == 0)
  {
    TRACE(_T("XListCtrl: edit boxes require LVS_EX_FULLROWSELECT style\n"));
    ASSERT(FALSE);
  }
#endif

  ASSERT(nItem >= 0);
  ASSERT(nItem < GetItemCount());
  if ((nItem < 0) || nItem >= GetItemCount()){
    return;
  }

  ASSERT(nSubItem >= 0);
  ASSERT(nSubItem < GetColumns());
  if ((nSubItem < 0) || nSubItem >= GetColumns()){
    return;
  }

  // Make sure that the item is visible
  if (!EnsureVisible(nItem, TRUE))
  {
    return;
  }

  // Make sure that nSubItem is valid
  ASSERT(GetColumnWidth(nSubItem) >= 5);
  if (GetColumnWidth(nSubItem) < 5){
    return;
  }

  XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
  if (!pXLCD){
    ASSERT(FALSE);
    return;
  }

  if (!pXLCD[0].bEnabled){
    return;                     // not enabled
  }

  CRect rect;
  GetSubItemRect(nItem, nSubItem, LVIR_BOUNDS, rect);

  if (pXLCD[nSubItem].nCheckedState >= 0)
  {
    int nChecked = pXLCD[nSubItem].nCheckedState;

    nChecked = (nChecked == 0) ? 1 : 0;

    pXLCD[nSubItem].nCheckedState = nChecked;
    pXLCD[nSubItem].bModified = TRUE;
    m_bListModified = TRUE;

    UpdateSubItem(nItem, nSubItem);

    CWnd *pWnd = GetParent();
    if (!pWnd)
      pWnd = GetOwner();
    if (pWnd && ::IsWindow(pWnd->m_hWnd))
      pWnd->SendMessage(WM_XLISTCTRL_CHECKBOX_CLICKED, 
                        nItem, nSubItem);

    // now update checkbox in header

    // -1 = no checkbox in column header
    if (GetHeaderCheckedState(nSubItem) != XHEADERCTRL_NO_IMAGE)
    {
      int nCheckedCount = CountCheckedItems(nSubItem);

      if (nCheckedCount == GetItemCount())
        SetHeaderCheckedState(nSubItem, XHEADERCTRL_CHECKED_IMAGE);
      else
        SetHeaderCheckedState(nSubItem, XHEADERCTRL_UNCHECKED_IMAGE);
    }

    //notify parent about state change ...
      LV_DISPINFO dispinfo;
      dispinfo.hdr.hwndFrom = m_hWnd;
      dispinfo.hdr.idFrom = GetDlgCtrlID();
      dispinfo.hdr.code = LVN_LAST - 1;
      
      dispinfo.item.mask = LVIF_STATE;
      dispinfo.item.iItem = nItem;
      dispinfo.item.iSubItem = nSubItem;
      dispinfo.item.state = LVIS_SELECTED;;
    	
      GetParent()->SendMessage(WM_NOTIFY, GetParent()->GetDlgCtrlID(), (LPARAM)&dispinfo);
  }
  else if (pXLCD[nSubItem].bCombo)
  {
    DrawComboBox(nItem, nSubItem);
  }
  else if (pXLCD[nSubItem].bEdit)
  {
    DrawEdit(nItem, nSubItem);
  }else if(pXLCD[nSubItem].bShowProgress){
    DrawProgressComboBox(nItem, nSubItem);
  }else if(pXLCD[nSubItem].bShowTime){
    DrawTimeCtrl(nItem, nSubItem);
  }
}

BOOL CXListCtrl::PreTranslateMessage(MSG* pMsg)
{
    // TODO: Add your specialized code here and/or call the base class
    if(pMsg->message == WM_KEYDOWN){
        TRACE("Key: %d down.\n", pMsg->wParam);
        if(pMsg->wParam == VK_RETURN){
            return TRUE;
        }
    }
    
    return CListCtrl::PreTranslateMessage(pMsg);
}

void CXListCtrl::MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct )
{
    LOGFONT lf;
    GetFont()->GetLogFont( &lf );
    
    if ( lf.lfHeight < 0 )
        lpMeasureItemStruct->itemHeight = -lf.lfHeight; 
    else
        lpMeasureItemStruct->itemHeight = lf.lfHeight; 
        
    lpMeasureItemStruct->itemHeight += 8; 
}

LRESULT CXListCtrl::OnSetFont(WPARAM wParam, LPARAM)
{
    LRESULT res = Default();

    CRect rc;
    GetWindowRect( &rc );

    WINDOWPOS wp;
    wp.hwnd  = m_hWnd;
    wp.cx    = rc.Width();
    wp.cy    = rc.Height();
    wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;
    SendMessage( WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp );

    return res;
}

void CXListCtrl::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	CListCtrl::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}


void CXListCtrl::OnHdnItemclick(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
    // TODO: Add your control notification handler code here
    *pResult = 0;
    
    const int COL = phdr->iItem;
    SortItems(COL, COL == m_nSortCol ? !m_bSortAscending : TRUE);
	
    TRACE("Sort item: %d\n", COL);
}

void CXListCtrl::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    // TODO: Add your control notification handler code here
    //const int COL = pNMListView->iSubItem;
    //SortItems(COL, COL == m_nSortCol ? !m_bSortAscending : TRUE);
    *pResult = 0;
}

void CXListCtrl::SortItems(int nColumn, BOOL bAscending)
{
    if (nColumn < 0 || nColumn >= GetColumnCount() || !IsSortable()){
        return;	
    }

    // do the sorting	
    m_nSortCol = nColumn;
    m_bSortAscending = bAscending;
	
    BOOL bEnd = FALSE;
    int nSep1 = -1;
    int nSep2 = _FindSeparator(-1, nColumn);
    do
    {
        if (nSep2 < 0)
        {
            //nSep2 = GetItemCount();
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // special!!! the last low is used for tip ....
            nSep2 = GetItemCount() - 1;
            bEnd = TRUE;
        }
        _PartialSort(nSep1 + 1, nSep2 - 1);
        nSep1 = _FindSeparator(nSep2 - 1, nColumn);
        nSep2 = _FindSeparator(nSep1, nColumn);
    } while (!bEnd && nSep1 >= 0);
}

int CXListCtrl::_FindSeparator(int nStartAfter, int nColumn) const
{
    if (m_pszSeparator == NULL)
        return -1;

    const int ITEMS = GetItemCount();
    for (int i = nStartAfter + 1; i < ITEMS; i++)
    {
        if (GetItemText(i, nColumn) == m_pszSeparator)
            return i;
    }

    return -1;
}

void CXListCtrl::_QuickSortRecursive(int* pArr, int nLow, int nHigh)
{
    int i = nHigh, j = nLow;
    int n = pArr[(nLow + nHigh) / 2];
    do
    {
        if (m_bSortAscending)
        {
            while (_CompareItems(pArr[j], n) < 0)
                j++;

            while (_CompareItems(pArr[i], n) > 0)
                i--;
        }
        else
        {
            while (_CompareItems(pArr[j], n) > 0)
                j++;

            while (_CompareItems(pArr[i], n) < 0)
                i--;
        }

        if ( i >= j )
        {
            if ( i != j )
            {
                int nTemp = pArr[i];
                pArr[i] = pArr[j];
                pArr[j] = nTemp;
            }

            i--;
            j++;
        }
	
    } while (j <= i);

    if (nLow < i)
        _QuickSortRecursive(pArr,nLow,i);

    if (j < nHigh)
        _QuickSortRecursive(pArr,j,nHigh);
}

BOOL CXListCtrl::_PartialSort(int nStart, int nEnd)
{
    struct ROWINFO
    {
	DWORD dwData;
	DWORD dwStates;
	CArray<int, int> aImages;
	CStringArray aTexts;
    };

    if (nStart >= nEnd || !_IsValidIndex(nStart) || !_IsValidIndex(nEnd))
        return FALSE;

    const int COUNT = nEnd - nStart + 1;
    int i = 0;
    int* aIndices = new int[COUNT];
    for (i = 0; i < COUNT; i++){
        aIndices[i] = nStart + i;
    }
	
    _QuickSortRecursive(aIndices, 0, COUNT - 1); 

    // rearrange items
    const int COLS = GetColumnCount();
    ROWINFO* aRows = new ROWINFO[COUNT];
    for (i = 0; i < COUNT; i++)
    {
        int n = aIndices[i];
        aRows[i].dwStates = GetItemState(aIndices[i], LVIS_SELECTED | LVIS_FOCUSED);
        aRows[i].dwData = CListCtrl::GetItemData(aIndices[i]);
        aRows[i].aImages.SetSize(COLS);
        aRows[i].aTexts.SetSize(COLS);
        for (int j = 0; j < COLS; j++)
        {
            aRows[i].aTexts[j] = GetItemText(aIndices[i], j);
        }
    }

    for (i = 0; i < COUNT; i++)
    {
        SetItemState(nStart + i, aRows[i].dwStates, aRows[i].dwStates);
        CListCtrl::SetItemData(nStart + i, aRows[i].dwData);
        for (int j = 0; j < COLS; j++)
        {
            SetItemImage(nStart + i, j, aRows[i].aImages[j]);			
            CListCtrl::SetItemText(nStart + i, j, aRows[i].aTexts[j]);
        }
    }

    delete [] aRows;
    delete [] aIndices;
    return TRUE;
}

int CXListCtrl::_CompareItems(int nItem1, int nItem2)
{
	const CString s1 = GetItemText(nItem1, m_nSortCol);
	const CString s2 = GetItemText(nItem2, m_nSortCol);

	DWORD dw1, dw2;
	if(_ITEM_COMPARE_FUNCS::_IsHexNumber(s1, dw1) && _ITEM_COMPARE_FUNCS::_IsHexNumber(s2, dw2))
		return _ITEM_COMPARE_FUNCS::_HexNumberCompare(dw1, dw2);

	double f1, f2;
	if(_ITEM_COMPARE_FUNCS::_IsDecNumber(s1, f1) && _ITEM_COMPARE_FUNCS::_IsDecNumber(s2, f2))
		return _ITEM_COMPARE_FUNCS::_DecNumberCompare(f1, f2);

	COleDateTime date1, date2;
	if(_ITEM_COMPARE_FUNCS::_IsDate(s1, date1) && _ITEM_COMPARE_FUNCS::_IsDate(s2, date2))
		return _ITEM_COMPARE_FUNCS::_DateCompare(date1, date2);
	
	// plain text.
	return s1.Compare(s2);
}

BOOL CXListCtrl::SetSortable(BOOL bSet)
{
    if (!HasColumnHeader()){
        return FALSE;
    }

    LONG lStyle = ::GetWindowLong(GetHeaderCtrl()->GetSafeHwnd(), GWL_STYLE);
    if (bSet)
    {
        lStyle |= HDS_BUTTONS;
    }
    else
    {
        lStyle &= ~HDS_BUTTONS;
	m_nSortCol = -1;
	m_bSortAscending = TRUE;
    }

    ::SetWindowLong(GetHeaderCtrl()->GetSafeHwnd(), GWL_STYLE, lStyle);
    CListCtrl::GetHeaderCtrl()->RedrawWindow();
    return TRUE;
}
