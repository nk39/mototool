// CIPTOptions.cpp : implementation file
//

#include "stdafx.h"
#include "clearcasehelper.h"
#include "CIPTOptions.h"


// CCIPTOptions dialog
CCIPTOptions CCIPTOptions::dlgOptions;

IMPLEMENT_DYNAMIC(CCIPTOptions, CDialog)

CCIPTOptions::CCIPTOptions(CWnd* pParent /*=NULL*/)
	: CDialog(CCIPTOptions::IDD, pParent)
    , m_bSideBySide(TRUE)
    , m_bShowNCSL(TRUE)
    , m_bShowLegend(TRUE)
    , m_nBefore(100)
    , m_nAfter(100)
    , m_bPortait(TRUE)
    , m_bIgnoreWS(TRUE)
{
}

CCIPTOptions::~CCIPTOptions()
{
}

void CCIPTOptions::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_SIDEBYSIDE, m_bSideBySide);
    DDX_Check(pDX, IDC_NCSLNUM, m_bShowNCSL);
    DDX_Check(pDX, IDC_LEGEND, m_bShowLegend);
    DDX_Text(pDX, IDC_BEFORE, m_nBefore);
    DDX_Text(pDX, IDC_AFTER, m_nAfter);
    DDX_Check(pDX, IDC_PORTAIT, m_bPortait);
    DDX_Check(pDX, IDC_WHITESPACE, m_bIgnoreWS);
}


BEGIN_MESSAGE_MAP(CCIPTOptions, CDialog)
    ON_BN_CLICKED(IDOK, &CCIPTOptions::OnBnClickedOk)
END_MESSAGE_MAP()


// CCIPTOptions message handlers

void CCIPTOptions::OnBnClickedOk()
{
    UpdateData();

    if(m_nAfter < 10) {
        AfxMessageBox("At least 10 required for 'Lines After'");
        return;
    }

    if(m_nBefore < 10 ) {
        AfxMessageBox("At least 10 required for 'Lines Before'");
        return;
    }

    theApp.WriteProfileInt("CLEARCASEHELPER", "BEFORE", m_nBefore);
    theApp.WriteProfileInt("CLEARCASEHELPER", "AFTER", m_nAfter);
    theApp.WriteProfileInt("CLEARCASEHELPER", "SIDEBYSIDE", m_bSideBySide ? 1 : 0);
    theApp.WriteProfileInt("CLEARCASEHELPER", "NCSLNUM", m_bShowNCSL ? 1 : 0);
    theApp.WriteProfileInt("CLEARCASEHELPER", "LEGEND", m_bShowLegend ? 1 : 0);
    theApp.WriteProfileInt("CLEARCASEHELPER", "PORTAIT", m_bPortait ? 1 : 0);
    theApp.WriteProfileInt("CLEARCASEHELPER", "IGNOREWS", m_bIgnoreWS ? 1 : 0);
    OnOK();
}

void CCIPTOptions::Initialize() 
{
    m_nBefore = theApp.GetProfileInt("CLEARCASEHELPER", "BEFORE", 100);
    m_nAfter = theApp.GetProfileInt("CLEARCASEHELPER", "AFTER", 100);

    if(m_nAfter < 10) {
        m_nAfter = 10;
    }

    if(m_nBefore < 10 ) {
        m_nBefore = 10;
    }

    UINT nVal = theApp.GetProfileInt("CLEARCASEHELPER", "SIDEBYSIDE", 1);
    m_bSideBySide = (nVal != 0);

    nVal = theApp.GetProfileInt("CLEARCASEHELPER", "NCSLNUM", 1);
    m_bShowNCSL = (nVal != 0);

    nVal = theApp.GetProfileInt("CLEARCASEHELPER", "LEGEND", 1);
    m_bShowLegend = (nVal != 0);

    nVal = theApp.GetProfileInt("CLEARCASEHELPER", "PORTAIT", 1);
    m_bPortait = (nVal != 0);

    nVal = theApp.GetProfileInt("CLEARCASEHELPER", "IGNOREWS", 1);
    m_bIgnoreWS = (nVal != 0);
}

BOOL CCIPTOptions::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  Add extra initialization here
    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
