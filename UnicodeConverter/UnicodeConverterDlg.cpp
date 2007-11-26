// UnicodeConverterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UnicodeConverter.h"
#include "UnicodeConverterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CUnicodeConverterDlg dialog




CUnicodeConverterDlg::CUnicodeConverterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUnicodeConverterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUnicodeConverterDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT1, m_raw);
    DDX_Control(pDX, IDC_EDIT2, m_unicode);
}

BEGIN_MESSAGE_MAP(CUnicodeConverterDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BUTTON1, &CUnicodeConverterDlg::OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON2, &CUnicodeConverterDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CUnicodeConverterDlg message handlers

BOOL CUnicodeConverterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
    CButton *pBut = (CButton *)GetDlgItem(IDC_UNICODE);
    if(pBut){
        pBut->SetCheck(BST_CHECKED);
    }
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CUnicodeConverterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CUnicodeConverterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CUnicodeConverterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CUnicodeConverterDlg::OnBnClickedButton1()
{
    CString s;
    m_raw.GetWindowText(s);
    if(s.IsEmpty()){
        AfxMessageBox(_T("Please input RAW string."));
        m_raw.SetFocus();
        return;
    }

    CString result;
    CButton *pBut = (CButton *)GetDlgItem(IDC_UNICODE);
    if(pBut && pBut->GetCheck() == BST_CHECKED){
        //now perform unicode transition
        // each unicode has 4 bytes ...so ...
        int i = 0;
        while(i < s.GetLength()){
            // now see what it is ...
            TCHAR ch = s[i];
            char c = (char)((ch >> 12) & 0xF);
            if(c >= 0 && c <=9){
                result.AppendChar(c + '0');
            }else{
                c -= 10;
                result.AppendChar(c + 'A');
            }

            c = (char)((ch >> 8) & 0xF);
            if(c >= 0 && c <=9){
                result.AppendChar(c + '0');
            }else{
                c -= 10;
                result.AppendChar(c + 'A');
            }

            c = (char)((ch >> 4) & 0xF);
            if(c >= 0 && c <=9){
                result.AppendChar(c + '0');
            }else{
                c -= 10;
                result.AppendChar(c + 'A');
            }

            c = (char)(ch & 0xF);
            if(c >= 0 && c <=9){
                result.AppendChar(c + '0');
            }else{
                c -= 10;
                result.AppendChar(c + 'A');
            }
            i ++;
        }
    } else {
        //now perform ASCII transition
        // each ASCII has 2 bytes ...so ...
        int i = 0;
        while(i < s.GetLength()){
            // now see what it is ...
            TCHAR ch = s[i];

            char c = (char)((ch >> 4) & 0xF);
            if(c >= 0 && c <=9){
                result.AppendChar(c + '0');
            }else{
                c -= 10;
                result.AppendChar(c + 'A');
            }

            c = (char)(ch & 0xF);
            if(c >= 0 && c <=9){
                result.AppendChar(c + '0');
            }else{
                c -= 10;
                result.AppendChar(c + 'A');
            }
            i ++;
        }
    }

    m_unicode.SetWindowText(result);
}

void CUnicodeConverterDlg::OnBnClickedButton2()
{
    CString s;
    m_unicode.GetWindowText(s);
    if(s.IsEmpty()){
        AfxMessageBox(_T("Please input UNICODE string."));
        m_unicode.SetFocus();
        return;
    }


    CString result;
    CButton *pBut = (CButton *)GetDlgItem(IDC_UNICODE);
    if(pBut && pBut->GetCheck() == BST_CHECKED){
        //now perform unicode transition
        // each unicode has 4 bytes ...so ...
        int i = 0;

        while(i < s.GetLength()){
            if((i + 4) > s.GetLength()){
                result.AppendChar('?');
                break;
            }

            TCHAR ch[4] = {s[i], s[i + 1], s[i + 2], s[i + 3]};
            i += 4;

            // now see what it is ...
            WORD c = 0;
            for(int j = 0; j < 4; j ++){
                c <<= 4;
                if(ch[j] >= '0' && ch[j] <= '9'){
                    c |= (ch[j] - '0');
                }else if(ch[j] >= 'a' && ch[j] <= 'f'){
                    c |= (ch[j] - 'a' + 0xa);
                }else if(ch[j] >= 'A' && ch[j] <= 'F'){
                    c |= (ch[j] - 'A' + 0xa);
                }else{
                    c = '?';
                }
            }

            result.AppendChar((TCHAR)c);
            /*
            if(c <= 128){
                result.AppendChar((char)c);
            }else{
                result.AppendChar('?');
            }
            */
        }
    }else {
        int i = 0;

        while(i < s.GetLength()){
            if((i + 2) > s.GetLength()){
                result.AppendChar('?');
                break;
            }

            TCHAR ch[2] = {s[i], s[i + 1]};
            i += 2;

            // now see what it is ...
            WORD c = 0;
            for(int j = 0; j < 2; j ++){
                c <<= 4;
                if(ch[j] >= '0' && ch[j] <= '9'){
                    c |= (ch[j] - '0');
                }else if(ch[j] >= 'a' && ch[j] <= 'f'){
                    c |= (ch[j] - 'a' + 0xa);
                }else if(ch[j] >= 'A' && ch[j] <= 'F'){
                    c |= (ch[j] - 'A' + 0xa);
                }else{
                    c = '?';
                }
            }

            result.AppendChar((TCHAR)c);
        }
    }

    m_raw.SetWindowText(result);
}
