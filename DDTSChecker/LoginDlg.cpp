// LoginDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DDTSChecker.h"
#include "LoginDlg.h"


// CLoginDlg dialog

IMPLEMENT_DYNAMIC(CLoginDlg, CDialog)

CLoginDlg::CLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoginDlg::IDD, pParent)
    , m_user(_T(""))
    , m_pass(_T(""))
{

}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_USER, m_user);
    DDX_Text(pDX, IDC_PASSWORD, m_pass);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialog)
    ON_BN_CLICKED(IDOK, &CLoginDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CLoginDlg message handlers

void CLoginDlg::OnBnClickedOk()
{
    UpdateData();

    // TODO: Add your control notification handler code here
    OnOK();
}

BOOL CLoginDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  Add extra initialization here

    CEdit *pWnd = (CEdit *)GetDlgItem(IDC_PASSWORD);
    if(pWnd){
        pWnd->SetSel(0, -1);
        pWnd->SetFocus();
    }

    return TRUE;  // return TRUE unless you set the focus to a control
}
