/*******************************************************************************
	Author						: Aravindan Premkumar
	Unregistered Copyright 2003	: Aravindan Premkumar
	All Rights Reserved
	
	This piece of code does not have any registered copyright and is free to be 
	used as necessary. The user is free to modify as per the requirements. As a
	fellow developer, all that I expect and request for is to be given the 
	credit for intially developing this reusable code by not removing my name as 
	the author.
*******************************************************************************/

#include "stdafx.h"
#include "InPlaceCombo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInPlaceCombo
CInPlaceCombo::CInPlaceCombo()
{
	m_iRowIndex = -1;
	m_iColumnIndex = -1;
	m_bESC = FALSE;
}

CInPlaceCombo::~CInPlaceCombo()
{
}

BEGIN_MESSAGE_MAP(CInPlaceCombo, CAdvComboBox)
	//{{AFX_MSG_MAP(CInPlaceCombo)
	ON_WM_CREATE()
	ON_WM_KILLFOCUS()
	ON_WM_CHAR()
	ON_CONTROL_REFLECT(CBN_CLOSEUP, OnCloseup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CInPlaceCombo::InitializeList(int nItem, int nSubItem, const CString &sText, const CStringList *pDropDownList)
{
  ResetContent(); 

  if(!pDropDownList){
    return;
  }

  for (POSITION Pos_ = pDropDownList->GetHeadPosition(); Pos_ != NULL;){
    CString s = (LPCTSTR) (pDropDownList->GetNext(Pos_));
    s.Trim();
    if(!s.IsEmpty()){
      AddString(s);
      TRACE("Add string: %s\n", s);
    }
  }

  m_iRowIndex = nItem;
  m_iColumnIndex = nSubItem;
  m_strWindowText = sText;

  if(SelectString(0, sText) < 0){
    SetCurSel(0);
  }
}


void CInPlaceCombo::InitializeList(int nItem, int nSubItem, const CString &sText, const CStringArray *pDropDownList)
{
  ResetContent(); 

  if(!pDropDownList){
    return;
  }

  for (int i = 0; i < pDropDownList->GetSize(); i++)
  {
    CString s = pDropDownList->GetAt(i);
    s.Trim();
    if (!s.IsEmpty()){
      AddString(s);
      TRACE("Add string: %s\n", s);
    }
  }

  m_iRowIndex = nItem;
  m_iColumnIndex = nSubItem;
  m_strWindowText = sText;

  if(SelectString(0, sText) < 0){
    SetCurSel(0);
  }
}

/////////////////////////////////////////////////////////////////////////////
// CInPlaceCombo message handlers

int CInPlaceCombo::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
  if (CAdvComboBox::OnCreate(lpCreateStruct) == -1){
    return -1;
  }
	
  // Set the proper font
  CFont* pFont = GetParent()->GetFont();
  SetFont(pFont);
	
  return 0;
}

BOOL CInPlaceCombo::PreTranslateMessage(MSG* pMsg) 
{
  // If the message if for "Enter" or "Esc"
  // Do not process
  if (pMsg->message == WM_KEYDOWN)
  {
    if (VK_ESCAPE == pMsg->wParam)
    {
      m_bESC = TRUE;

      GetParent()->SetFocus();
      return TRUE;				
    }else if(VK_RETURN == pMsg->wParam){
      GetParent()->SetFocus();

      GetParent()->PostMessage(WM_KEYDOWN, VK_RETURN);

      return TRUE;				
    }
  }
	
  return CAdvComboBox::PreTranslateMessage(pMsg);
}

void CInPlaceCombo::OnKillFocus(CWnd* pNewWnd) 
{
  CAdvComboBox::OnKillFocus(pNewWnd);
	
  FinishEdit();

  ShowWindow(SW_HIDE);
}

void CInPlaceCombo::FinishEdit()
{
  // Get the current selection text
  CString str;
  GetLBText(GetCurSel(), str);

  // Send Notification to parent of ListView ctrl
  LV_DISPINFO dispinfo;
  dispinfo.hdr.hwndFrom = GetParent()->m_hWnd;
  dispinfo.hdr.idFrom = GetDlgCtrlID();
  dispinfo.hdr.code = LVN_ENDLABELEDIT;

  dispinfo.item.mask = LVIF_TEXT;
  dispinfo.item.iItem = m_iRowIndex;
  dispinfo.item.iSubItem = m_iColumnIndex;
  dispinfo.item.pszText = m_bESC ? LPTSTR((LPCTSTR)m_strWindowText) : LPTSTR((LPCTSTR)str);
  dispinfo.item.cchTextMax = m_bESC ? m_strWindowText.GetLength() : str.GetLength();
	
  GetParent()->SendMessage(WM_NOTIFY, GetParent()->GetDlgCtrlID(), (LPARAM)&dispinfo);
}

void CInPlaceCombo::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  // If the key is "Esc" set focus back to the list control
  if (nChar == VK_ESCAPE || nChar == VK_RETURN)
  {
    if (nChar == VK_ESCAPE)
    {
      m_bESC = TRUE;

      GetParent()->SetFocus();
    }else{
      FinishEdit();
      GetParent()->PostMessage(WM_KEYDOWN, VK_RETURN);
    }

    return;
  }
	
  CAdvComboBox::OnChar(nChar, nRepCnt, nFlags);
}

void CInPlaceCombo::OnCloseup() 
{
  // Set the focus to the parent list control
  GetParent()->SetFocus();
}
