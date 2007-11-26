// ComboBoxBold.cpp : implementation file
//

#pragma warning ( disable : 4786 )

#include "stdafx.h"
#include "ImgComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const UINT CImgComboBox::DEFAULT_COLUMN_COUNT = 100;
const UINT CImgComboBox::DEFAULT_COLUMN_WIDTH = 50;

CImgComboBox::CImgComboBox()
: m_pImageList(NULL)
, m_bUseImage(TRUE)
{
	m_vecColumnWidth.resize(DEFAULT_COLUMN_COUNT);
	for (int i=0; i<DEFAULT_COLUMN_COUNT;i++)
	{
		m_vecColumnWidth[i] = DEFAULT_COLUMN_WIDTH;
	}
}

CImgComboBox::~CImgComboBox()
{
}


BEGIN_MESSAGE_MAP(CImgComboBox, CComboBox)
	//{{AFX_MSG_MAP(CImgComboBox)
	ON_WM_DELETEITEM()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*********************************************************************************************
 * 
 * @ChangesHistory
 ********************************************************************************************/
void CImgComboBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	if (GetCount() == 0 || lpDrawItemStruct->itemID > GetCount()) return;

	ItemData* pItemData = NULL;

	if (lpDrawItemStruct->itemData!=NULL)
	{
		pItemData = (ItemData*)lpDrawItemStruct->itemData;
	}

	CString str;
	GetLBText(lpDrawItemStruct->itemID, str);
	CDC dc;
	BOOL bSelected = FALSE;

	dc.Attach(lpDrawItemStruct->hDC);

	// Save these value to restore them when done drawing.
	COLORREF crOldTextColor = dc.GetTextColor();
	COLORREF crOldBkColor = dc.GetBkColor();

	// If this item is selected, set the background color and the text color to appropriate 
	// values. Erase the rect by filling it with the background color.
	if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
		(lpDrawItemStruct->itemState & ODS_SELECTED))
	{
		dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		dc.SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
		dc.FillSolidRect(&lpDrawItemStruct->rcItem, ::GetSysColor(COLOR_HIGHLIGHT));
		bSelected = TRUE;
	}
	else
	{
		dc.FillSolidRect(&lpDrawItemStruct->rcItem, crOldBkColor);
	}

	CRect rect(lpDrawItemStruct->rcItem);
	rect.DeflateRect(1,0);

	// If we use images, and there is data, and the index is valid:
	if (m_pImageList && m_bUseImage && pItemData && pItemData->nImageIndex!=-1)
	{
		DrawIconEx(dc.GetSafeHdc(),rect.left,rect.top,
			m_pImageList->ExtractIcon(pItemData->nImageIndex),0, 0, 0, NULL, DI_NORMAL);
	}

	// If we use images - move text to the right:
	if (m_bUseImage && m_pImageList)
	{
		IMAGEINFO sImageInfo;
		m_pImageList->GetImageInfo(0, &sImageInfo);
		rect.left += sImageInfo.rcImage.right;
	}

	CFont* pOldFont;
	CFont boldFont;
	if (pItemData && pItemData->bBold)
	{
		CFont* curFont = dc.GetCurrentFont();
		LOGFONT lf;
		curFont->GetLogFont(&lf);
		lf.lfWeight = FW_BOLD;
		boldFont.CreateFontIndirect(&lf);
		pOldFont = dc.SelectObject(&boldFont);
	}

	// If the item has its own color, replace text color (exception - color is black, and
	// the item is selected - then we leave the highlight text color)
	if (pItemData && (!bSelected || (bSelected && pItemData->crTextColor != RGB(0,0,0))))
	{
		dc.SetTextColor(pItemData->crTextColor);
	}

	// If we need to display columns - a bit more complicated...
	if (m_vecColumnWidth.size()>1)
	{
		CPen linePen(PS_SOLID, 0, RGB(192,192,192));
		CPen* pOldPen = dc.SelectObject(&linePen);
		int nCurX=0;
		for (int i=0; i<m_vecColumnWidth.size(); i++)
		{
			if (i!=m_vecColumnWidth.size()-1)
			{
				dc.MoveTo(rect.left+nCurX+m_vecColumnWidth[i],rect.top);
				dc.LineTo(rect.left+nCurX+m_vecColumnWidth[i],rect.bottom);
			}
			CRect rc(rect);
			rc.left=rect.left+nCurX+1;
			if (i!=m_vecColumnWidth.size()-1)
			{
				rc.right=rect.left+nCurX+m_vecColumnWidth[i];
			}
			else
			{
				rc.right = rect.right;
			}
			nCurX += m_vecColumnWidth[i];

			if (i==0)
			{
				dc.DrawText(str, -1, &rc, DT_LEFT|DT_SINGLELINE|DT_VCENTER);
			}
			else if (pItemData)
			{
				if (pItemData->mapStrings.find(i)!=pItemData->mapStrings.end())
				{
					dc.DrawText(pItemData->mapStrings[i], -1, &rc, DT_LEFT|DT_SINGLELINE|DT_VCENTER);
				}
			}
		}

		dc.SelectObject(pOldPen);
	}
	else
	{
		// Normal one column text display:
		dc.DrawText(str, -1, &rect, DT_LEFT|DT_SINGLELINE|DT_VCENTER);
	}

	if (pItemData && pItemData->bBold)
	{
		dc.SelectObject(pOldFont);
		boldFont.DeleteObject();
	}

	// Reset the background color and the text color back to their original values.
	dc.SetTextColor(crOldTextColor);
	dc.SetBkColor(crOldBkColor);

	dc.Detach();
}

/*********************************************************************************************
 * 
 * @ChangesHistory
 ********************************************************************************************/
void CImgComboBox::SetItemBold(int nItemIndex,  BOOL bBold)
{
	ItemData* pItemData = GetOrCreateItemData(nItemIndex);
	if (pItemData)
	{
		pItemData->bBold = bBold;
		Invalidate();
	}
}

/*********************************************************************************************
 * 
 * @ChangesHistory
 ********************************************************************************************/
CString CImgComboBox::GetItemText(int nItemIndex,int nColumn)
{
	if (0==nColumn)
	{
		CString str;
		GetLBText(nItemIndex, str);
		return str;
	}

	ItemData* pItemData = GetOrCreateItemData(nItemIndex);
	if (pItemData)
	{
		return pItemData->mapStrings[nColumn];
	}
	else
	{
		return "";
	}
}

/*********************************************************************************************
 * 
 * @ChangesHistory
 ********************************************************************************************/
void CImgComboBox::SetItemImage(int nItemIndex, int nImageIndex)
{
	ItemData* pItemData = GetOrCreateItemData(nItemIndex);
	if (pItemData)
	{
		pItemData->nImageIndex = nImageIndex;
		Invalidate();
	}
}

/*********************************************************************************************
 * 
 * @ChangesHistory
 ********************************************************************************************/
void CImgComboBox::SetItemColor(int nItemIndex, COLORREF rcTextColor)
{
	ItemData* pItemData = GetOrCreateItemData(nItemIndex);
	if (pItemData)
	{
		pItemData->crTextColor = rcTextColor;
		Invalidate();
	}
}

/*********************************************************************************************
 * 
 * @ChangesHistory
 ********************************************************************************************/
void CImgComboBox::SetItemText(int nItemIndex, int nColumn, CString str)
{
	ItemData* pItemData = GetOrCreateItemData(nItemIndex);
	if (pItemData)
	{
		pItemData->mapStrings[nColumn] = str;
	}
}

/*********************************************************************************************
 * 
 * @ChangesHistory
 ********************************************************************************************/
void CImgComboBox::SetColumnWidth(int nColumnIndex, int nWidth)
{
	if (nColumnIndex<0 || nColumnIndex>=m_vecColumnWidth.size()) return;

	m_vecColumnWidth[nColumnIndex] = nWidth;

	Invalidate();
}

/*********************************************************************************************
 * 
 * @ChangesHistory
 ********************************************************************************************/
void CImgComboBox::OnDeleteItem(int nIDCtl, LPDELETEITEMSTRUCT lpDeleteItemStruct)
{
	if (lpDeleteItemStruct->itemData)
	{
		ItemData* pItemData = (ItemData*)lpDeleteItemStruct->itemData;
		delete pItemData;
		lpDeleteItemStruct->itemData = NULL;
	}

	CComboBox::OnDeleteItem(nIDCtl, lpDeleteItemStruct);
}

/*********************************************************************************************
 * 
 * @ChangesHistory
 ********************************************************************************************/
CImgComboBox::ItemData* CImgComboBox::GetOrCreateItemData(int nItemIndex)
{
	if (nItemIndex<0 || nItemIndex>=GetCount()) return NULL;

	ItemData* pItemData = (ItemData*)(CComboBox::GetItemData(nItemIndex));

	if (NULL==pItemData)
	{
		pItemData = new ItemData;
		CComboBox::SetItemData(nItemIndex,(DWORD)(pItemData));
	}
	
	return pItemData;
}

/*********************************************************************************************
 * 
 * @ChangesHistory
 ********************************************************************************************/
void CImgComboBox::SetItemData(int nItemIndex, DWORD dwData)
{
	ItemData* pItemData = GetOrCreateItemData(nItemIndex);
	if (pItemData)
	{
		pItemData->dwItemData = dwData;
	}
}

/*********************************************************************************************
 * 
 * @ChangesHistory
 ********************************************************************************************/
DWORD CImgComboBox::GetItemData(int nItemIndex)
{
	ItemData* pItemData = GetOrCreateItemData(nItemIndex);
	if (pItemData)
	{
		return pItemData->dwItemData;
	}

	return 0;
}

/*********************************************************************************************
 * 
 * @ChangesHistory
 ********************************************************************************************/
void CImgComboBox::SetColumnCount(int nColumnCount )
{
	int nPrevColumnCount = m_vecColumnWidth.size();
	m_vecColumnWidth.resize(nColumnCount);

	for (int i=nPrevColumnCount; i<m_vecColumnWidth.size(); i++)
	{
		m_vecColumnWidth[i] = DEFAULT_COLUMN_WIDTH;
	}

	Invalidate();
}
