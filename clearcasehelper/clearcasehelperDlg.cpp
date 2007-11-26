// clearcasehelperDlg.cpp : implementation file
//

#include "stdafx.h"
#include "clearcasehelper.h"
#include "clearcasehelperDlg.h"
#include "HttpClient.h"
#include "w3c.h"
#include "LoginDlg.h"
#include "WebPage.h"
#include "CIPTOptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static struct _wsa_struct {
    sockaddr_in cipt_addr;

    _wsa_struct() {
	    WSADATA wsaData;
        WSAStartup(0x101,&wsaData);

        memset(&cipt_addr, 0, sizeof(cipt_addr));
        cipt_addr.sin_family=AF_INET;
        cipt_addr.sin_port=htons(8000);

        struct hostent *hp = gethostbyname("ciptweb.cig.mot.com");
	    if(hp==NULL)
	    {
            ASSERT(0);
            cipt_addr.sin_addr.s_addr = inet_addr("10.17.192.194");;
        }else{
            cipt_addr.sin_addr.s_addr = *((unsigned long*)hp->h_addr);
        }
    }

    ~_wsa_struct() {
        WSACleanup();
    }
} _cipt_init;

static PostArgumentMap gPostArguments;

static UINT gControls[] = {IDC_BRANCH, IDC_SOURCE, IDC_OUTPUT, IDC_BROWSE, IDC_BROWSEOUT, 
IDC_REFRESH, IDC_CIPT, IDC_DIFF, IDC_BASE, IDC_PREV,
IDC_SELALL, IDC_CLRALL};
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


// CclearcasehelperDlg dialog




CclearcasehelperDlg::CclearcasehelperDlg(CWnd* pParent /*=NULL*/)
: CResizableDialog(CclearcasehelperDlg::IDD, pParent), m_cleartool(m_files)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_state = StateLoginPrepare;
}

void CclearcasehelperDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_FILES, m_files);
    DDX_Control(pDX, IDC_SOURCE, m_source);
    DDX_Control(pDX, IDC_OUTPUT, m_output);
    DDX_Control(pDX, IDC_BRANCH, m_branch);
    DDX_Control(pDX, IDC_WEB, m_loginWeb);
    DDX_Control(pDX, IDC_STATUS, m_status);
}

BEGIN_MESSAGE_MAP(CclearcasehelperDlg, CResizableDialog)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BROWSE, &CclearcasehelperDlg::OnBnClickedBrowse)
    ON_BN_CLICKED(IDC_BROWSEOUT, &CclearcasehelperDlg::OnBnClickedBrowseout)
    ON_BN_CLICKED(IDC_CIPT, &CclearcasehelperDlg::OnBnClickedCipt)
    ON_BN_CLICKED(IDC_DIFF, &CclearcasehelperDlg::OnBnClickedDiff)
    ON_BN_CLICKED(IDC_REFRESH, &CclearcasehelperDlg::OnBnClickedRefresh)
    ON_BN_CLICKED(IDC_BASE, &CclearcasehelperDlg::OnBnClickedBase)
    ON_BN_CLICKED(IDC_PREV, &CclearcasehelperDlg::OnBnClickedPrev)
    ON_BN_CLICKED(IDC_SELALL, &CclearcasehelperDlg::OnBnClickedSelall)
    ON_BN_CLICKED(IDC_CLRALL, &CclearcasehelperDlg::OnBnClickedClrall)
    ON_WM_SIZE()
    ON_MESSAGE(WM_MY_MESSAGE, &CclearcasehelperDlg::OnMyMessage)
    ON_BN_CLICKED(IDOK, &CclearcasehelperDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDC_CIPTOPTION, &CclearcasehelperDlg::OnBnClickedCiptoption)
END_MESSAGE_MAP()


BEGIN_EASYSIZE_MAP(CclearcasehelperDlg)
    EASYSIZE(IDC_FILES,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)
    EASYSIZE(IDC_BRANCH,ES_BORDER, ES_BORDER, ES_BORDER, ES_KEEPSIZE, 0)
    EASYSIZE(IDC_SOURCE,ES_BORDER, ES_BORDER, ES_BORDER, ES_KEEPSIZE, 0)
    EASYSIZE(IDC_OUTPUT,ES_BORDER, ES_BORDER, ES_BORDER, ES_KEEPSIZE, 0)
    EASYSIZE(IDC_BASE,ES_KEEPSIZE, ES_BORDER, ES_BORDER, ES_KEEPSIZE, 0)
    EASYSIZE(IDC_PREV,ES_KEEPSIZE, ES_BORDER, ES_BORDER, ES_KEEPSIZE, 0)
    EASYSIZE(IDC_SELALL,ES_KEEPSIZE, ES_BORDER, ES_BORDER, ES_KEEPSIZE, 0)
    EASYSIZE(IDC_CLRALL,ES_KEEPSIZE, ES_BORDER, ES_BORDER, ES_KEEPSIZE, 0)
    EASYSIZE(IDC_BROWSE,ES_KEEPSIZE, ES_BORDER, ES_BORDER, ES_KEEPSIZE, 0)
    EASYSIZE(IDC_BROWSEOUT,ES_KEEPSIZE, ES_BORDER, ES_BORDER, ES_KEEPSIZE, 0)
    EASYSIZE(IDOK, ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
    EASYSIZE(IDC_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
    EASYSIZE(IDC_WEB,ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

BEGIN_EVENTSINK_MAP(CclearcasehelperDlg, CResizableDialog)
    //{{AFX_EVENTSINK_MAP(CSWDLBrowserDialog)
    ON_EVENT(CclearcasehelperDlg, IDC_WEB, 259 /* DocumentComplete */, OnDocumentComplete, VTS_DISPATCH VTS_PVARIANT)
    //}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

// CclearcasehelperDlg message handlers
void CclearcasehelperDlg::EnableSubControls(BOOL bEnable)
{
    for(int i = 0; i < sizeof(gControls) / sizeof(gControls[0]); i++){
        CWnd *pWnd = GetDlgItem(gControls[i]);
        if(pWnd){
            pWnd->EnableWindow(bEnable);
        }
    }
}

BOOL CclearcasehelperDlg::OnInitDialog()
{
    CResizableDialog::OnInitDialog();
    CCIPTOptions::GetOptionsDlg().Initialize();

    INIT_EASYSIZE;
    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        pSysMenu->RemoveMenu(SC_CLOSE, MF_BYCOMMAND);

        CString strAboutMenu;
        strAboutMenu.LoadString(IDS_ABOUTBOX);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    // TODO: Add extra initialization here
    CButton *pCipt = (CButton *)GetDlgItem(IDC_CIPT);
    if(pCipt){
        pCipt->SetCheck(BST_CHECKED);
    }

    m_status.SetWindowText("Please choose operation.");

    m_files.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
    m_files.EnableToolTips(TRUE);
    m_files.InsertColumn(FileInfo_Name, "Name", LVCFMT_LEFT, 150);
    m_files.InsertColumn(FileInfo_Base, "Base", LVCFMT_LEFT, 150);
    m_files.InsertColumn(FileInfo_Curr, "Curr", LVCFMT_LEFT, 50);
    m_files.InsertColumn(FileInfo_Prev, "Prev", LVCFMT_LEFT, 50);
    m_files.InsertColumn(FileInfo_Status, "Status", LVCFMT_LEFT, 100);


    CString s = theApp.GetProfileString("CLEARCASEHELPER", "BRANCH", "");
    m_branch.SetWindowText(s);

    s = theApp.GetProfileString("CLEARCASEHELPER", "SOURCE", "");
    m_source.SetWindowText(s);

    s = theApp.GetProfileString("CLEARCASEHELPER", "OUTPUT", "");
    m_output.SetWindowText(s);
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CclearcasehelperDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CResizableDialog::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CclearcasehelperDlg::OnPaint()
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
        CResizableDialog::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CclearcasehelperDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


void CclearcasehelperDlg::OnBnClickedBrowse()
{
    // TODO: Add your control notification handler code here
    TCHAR buf[MAX_PATH + 1];
    memset(buf, 0, sizeof(buf));

    BROWSEINFO pbi;
    memset(&pbi, 0, sizeof(pbi));
    pbi.hwndOwner = GetSafeHwnd();
    pbi.lpszTitle = _T("Please select clearcase view directory.");
    pbi.pszDisplayName = buf;

    pbi.ulFlags = BIF_VALIDATE | BIF_EDITBOX;

    ITEMIDLIST *pIDList = SHBrowseForFolder(&pbi);
    if(pIDList == 0){
        return;
    }

    if(pIDList)
    {
        SHGetPathFromIDList(pIDList, buf);
        m_source.SetWindowText(buf);
    }

    LPMALLOC lpMalloc;
    if(FAILED(SHGetMalloc(&lpMalloc))){
        return;
    }

    lpMalloc->Free(pIDList);
    lpMalloc->Release();
}

void CclearcasehelperDlg::OnBnClickedBrowseout()
{
    // TODO: Add your control notification handler code here
    TCHAR buf[MAX_PATH + 1];
    memset(buf, 0, sizeof(buf));

    BROWSEINFO pbi;
    memset(&pbi, 0, sizeof(pbi));
    pbi.hwndOwner = GetSafeHwnd();
    pbi.lpszTitle = _T("Please select clearcase view directory.");
    pbi.pszDisplayName = buf;

    pbi.ulFlags = BIF_VALIDATE | BIF_EDITBOX;

    ITEMIDLIST *pIDList = SHBrowseForFolder(&pbi);
    if(pIDList == 0){
        return;
    }

    if(pIDList)
    {
        SHGetPathFromIDList(pIDList, buf);
        m_output.SetWindowText(buf);
    }

    LPMALLOC lpMalloc;
    if(FAILED(SHGetMalloc(&lpMalloc))){
        return;
    }

    lpMalloc->Free(pIDList);
    lpMalloc->Release();
}

void CclearcasehelperDlg::OnBnClickedCipt()
{
    // TODO: Add your control notification handler code here
}

void CclearcasehelperDlg::OnBnClickedDiff()
{
    // TODO: Add your control notification handler code here
}

void CclearcasehelperDlg::OnBnClickedRefresh()
{
    CString sBranch;
    m_branch.GetWindowText(sBranch);
    sBranch.Trim();
    if(sBranch.IsEmpty()){
        AfxMessageBox(_T("Please set branch name to continue."), MB_OK | MB_ICONSTOP);
        m_branch.SetFocus();
        return;
    }

    CString sPath;
    m_source.GetWindowText(sPath);

    //see if this is a valid directory
    DWORD attrib = ::GetFileAttributes(sPath);
    if((attrib == INVALID_FILE_ATTRIBUTES) || ((attrib & FILE_ATTRIBUTE_DIRECTORY) == 0)){
        AfxMessageBox(_T("Please select a valid clearcase view source."), MB_OK | MB_ICONSTOP);
        m_source.SetSel(0, -1);
        m_source.SetFocus();
        return;
    }

    m_files.DeleteAllItems();

    // ok, now, we retrieve the changed files ...
    CString sCommand = _T("cleartool find \"");
    sCommand += sPath;
    sCommand += _T("\" -version \"version(.../");
    sCommand += sBranch;
    sCommand += _T("/LATEST)\" -print");

    TRACE(_T("Command: %s\n"), sCommand);

    // let's run the command ... and get the output and status ...
    m_cleartool.SetCallback(GetSafeHwnd(), WM_MY_MESSAGE, MSG_DONE);
    m_cleartool.SetPrefix(sPath);
    m_cleartool.Close();
    m_cleartool.Open(sCommand);

    CString sStatus;
    sStatus.Format("Retrieving changed files in branch %s", sBranch);
    m_status.SetWindowText(sStatus);
    EnableSubControls(FALSE);
}


void CclearcasehelperDlg::OnBnClickedOk()
{
    if(IDYES == AfxMessageBox("Are you sure to quit?", MB_YESNO | MB_ICONQUESTION)){
        CString s;
        m_branch.GetWindowText(s);
        theApp.WriteProfileString("CLEARCASEHELPER", "BRANCH", s);

        m_source.GetWindowText(s);
        theApp.WriteProfileString("CLEARCASEHELPER", "SOURCE", s);

        m_output.GetWindowText(s);
        theApp.WriteProfileString("CLEARCASEHELPER", "OUTPUT", s);

        CResizableDialog::OnOK();
    }
}

void CclearcasehelperDlg::OnBnClickedBase()
{
    m_bCompareBase = true;

    if(m_files.GetSelectedCount() == 0){
        AfxMessageBox(_T("Please select an item to perform this operation."));
        return;
    }

    bool bCipt = false;
    CButton *pCipt = (CButton *) GetDlgItem(IDC_CIPT);
    if(pCipt){
        if(BST_CHECKED == pCipt->GetCheck()){
            bCipt = true;
        }
    }

    if(bCipt){
        // see output directory OK?
        CString sPath;
        m_output.GetWindowText(sPath);

        //see if this is a valid directory
        DWORD attrib = ::GetFileAttributes(sPath);
        if((attrib == INVALID_FILE_ATTRIBUTES) || ((attrib & FILE_ATTRIBUTE_DIRECTORY) == 0)){
            AfxMessageBox(_T("Please select a valid output directory."), MB_OK | MB_ICONSTOP);
            m_output.SetSel(0, -1);
            m_output.SetFocus();
            return;
        }
    }

    EnableSubControls(FALSE);

    m_items.clear();

    for(int index = 0; index < m_files.GetItemCount(); index ++){
        m_files.SetItemText(index, FileInfo_Status, "");
        m_files.SetItemToolTipText(index, FileInfo_Status, ""); 
        if(m_files.GetItemState(index, LVIS_SELECTED) != LVIS_SELECTED){
            m_files.SetItemText(index, FileInfo_Status, "Not Selected");
            continue;
        }

        /*
        CString sFile;
        m_files.GetText(index, sFile);
        int pos1 = sFile.ReverseFind(_T('\\'));
        int pos2 = sFile.ReverseFind(_T('/'));

        int pos = (pos1 > pos2) ? pos1 : pos2;
        sFile = sFile.Mid(0, pos + 1);
        CString sBase = sFile + _T("0");
        CString sLatest = sFile + _T("LATEST");
        */
        CString sFile = m_files.GetItemText(index, FileInfo_Name);
        CString sBase = m_files.GetItemText(index, FileInfo_Base);
        CString sCurr = m_files.GetItemText(index, FileInfo_Curr);
        CString sLatest = sBase + sCurr;
        sBase = sBase += "0";
        TRACE(_T("Base: %s\n"), sBase);
        TRACE(_T("Current: %s\n"), sLatest);

        if(bCipt){
            CIPTItem item;
            item.item = index;
            /*
            pos = sBase.Find("@@");
            CString file = sBase.Mid(0, pos);

            pos1 = file.ReverseFind(_T('\\'));
            pos2 = file.ReverseFind(_T('/'));

            if(pos1 < pos2) {
                pos1 = pos2;
            }

            item.path = file.Mid(0, pos1 + 1);
            item.name = file.Mid(pos1 + 1);

            pos += 2;       // ignore "@@"
            item.vold = (const char *)sBase.Mid(pos);
            item.vnew = (const char *)sLatest.Mid(pos);
            */
            int pos1 = sFile.ReverseFind(_T('\\'));
            int pos2 = sFile.ReverseFind(_T('/'));
            if(pos1 < pos2) {
                pos1 = pos2;
            }
            item.path = sFile.Mid(0, pos1 + 1);
            item.name = sFile.Mid(pos1 + 1);
            item.vold = (const char *)sBase;
            item.vnew = (const char *)sLatest;

            m_items.push_back(item);

            m_files.SetItemText(index, FileInfo_Status, "Queued");
        }else{
            CString sArg = sFile + _T("@@") + sLatest + _T(" ") + sFile + _T("@@") + sBase;
            ::ShellExecute(0, _T("open"), _T("cleardiffmrg"), sArg, NULL, SW_SHOW);
        }
    }

    if(bCipt){
        //kick off now ...
        PrepareCIPT();
    }else{
        EnableSubControls(TRUE);
    }
}

void CclearcasehelperDlg::OnBnClickedPrev()
{
    m_bCompareBase = false;

    if(m_files.GetSelectedCount() == 0){
        AfxMessageBox(_T("Please select an item to perform this operation."));
        return;
    }


    bool bCipt = false;
    CButton *pCipt = (CButton *) GetDlgItem(IDC_CIPT);
    if(pCipt){
        if(BST_CHECKED == pCipt->GetCheck()){
            bCipt = true;
        }
    }

    if(bCipt){
        // see output directory OK?
        CString sPath;
        m_output.GetWindowText(sPath);

        //see if this is a valid directory
        DWORD attrib = ::GetFileAttributes(sPath);
        if((attrib == INVALID_FILE_ATTRIBUTES) || ((attrib & FILE_ATTRIBUTE_DIRECTORY) == 0)){
            AfxMessageBox(_T("Please select a valid output directory."), MB_OK | MB_ICONSTOP);
            m_output.SetSel(0, -1);
            m_output.SetFocus();
            return;
        }
    }

    EnableSubControls(FALSE);

    m_items.clear();
    for(int index = 0; index < m_files.GetItemCount(); index ++){
        m_files.SetItemText(index, FileInfo_Status, "");
        m_files.SetItemToolTipText(index, FileInfo_Status, ""); 
        if(m_files.GetItemState(index, LVIS_SELECTED) != LVIS_SELECTED){
            m_files.SetItemText(index, FileInfo_Status, "Not Selected");
            continue;
        }

        /*
        CString sFile;
        m_files.GetText(index, sFile);
        int pos1 = sFile.ReverseFind(_T('\\'));
        int pos2 = sFile.ReverseFind(_T('/'));

        int pos = (pos1 > pos2) ? pos1 : pos2;
        CString sVer = sFile.Mid(pos + 1);
        sFile = sFile.Mid(0, pos + 1);

        int ver = _ttoi(sVer);
        if(ver > 0){
            ver --;
        }

        CString sPrev;
        sPrev.Format(_T("%s%d"), sFile, ver);
        CString sLatest = sFile + _T("LATEST");
        */
        CString sFile = m_files.GetItemText(index, FileInfo_Name);
        CString sBase = m_files.GetItemText(index, FileInfo_Base);
        CString sCurr = m_files.GetItemText(index, FileInfo_Curr);
        CString sLatest = sBase + sCurr;
        CString sPrev = sBase + m_files.GetItemText(index, FileInfo_Prev);

        TRACE(_T("Prev: %s\n"), sPrev);
        TRACE(_T("Current: %s\n"), sLatest);

        if(bCipt){
            CIPTItem item;
            item.item = index;
            //pos = sPrev.Find("@@");
            //CString file = sPrev.Mid(0, pos);

            CString file = sFile;
            int pos1 = file.ReverseFind(_T('\\'));
            int pos2 = file.ReverseFind(_T('/'));

            if(pos1 < pos2) {
                pos1 = pos2;
            }

            item.path = file.Mid(0, pos1 + 1);
            item.name = file.Mid(pos1 + 1);

            //pos += 2;       // ignore "@@"
            item.vold = (const char *)sPrev;
            item.vnew = (const char *)sLatest;

            m_items.push_back(item);
            m_files.SetItemText(index, FileInfo_Status, "Queued");
        }else{
            //well, this item is selected
            CString sArg = sFile + _T("@@") + sLatest + _T(" ") + sFile + _T("@@") + sPrev;
            ::ShellExecute(0, _T("open"), _T("cleardiffmrg"), sArg, NULL, SW_SHOW);
        }
    }

    if(bCipt){
        PrepareCIPT();
    }else{
        EnableSubControls(TRUE);
    }
}

void CclearcasehelperDlg::OnBnClickedSelall()
{
    for(int index = 0; index < m_files.GetItemCount(); index ++){
        m_files.SetItemState(index, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
    }
}

void CclearcasehelperDlg::OnBnClickedClrall()
{
    for(int index = 0; index < m_files.GetItemCount(); index ++){
        m_files.SetItemState(index, 0, LVIS_FOCUSED | LVIS_SELECTED);

        TRACE("GetItemState: %d\n", m_files.GetItemState(index, LVIS_SELECTED));
    }
}

void CclearcasehelperDlg::OnSize(UINT nType, int cx, int cy)
{
    CResizableDialog::OnSize(nType, cx, cy);

    // TODO: Add your message handler code here
    if(m_files.GetSafeHwnd()){
        UPDATE_EASYSIZE;
    }
}


/////////////////////////////////////////////////////////

void CclearcasehelperDlg::CClearRedirector::WriteStdOut(LPCTSTR pszOutput)
{
    m_cache += pszOutput;
    int pos = m_cache.FindOneOf(_T("\r\n"));
    while(pos >= 0){
        CString sItem = m_cache.Mid(0, pos);
        sItem.Trim();
        if(sItem.IsEmpty() == false){
            TRACE(_T("Item: %s\n"), sItem);
            //only if it start with the prefix
            if(m_prefix.CompareNoCase(sItem.Mid(0, m_prefix.GetLength())) == 0){
                // get out the name, version, last version, previous version ...
                int pos1 = sItem.ReverseFind(_T('\\'));
                int pos2 = sItem.ReverseFind(_T('/'));
                int pos = (pos1 > pos2) ? pos1 : pos2;

                CString sFile = sItem.Mid(0, pos + 1);
                CString sCurr = sItem.Mid(pos + 1);
                int ver = _ttoi(sCurr);
                if(ver > 0){
                    ver --;
                }

                CString sPrev;
                sPrev.Format("%d", ver);

                pos = sFile.Find("@@");
                CString sName = sFile.Mid(0, pos);
                CString sBase = sFile.Mid(pos + 2);
                
                //CString sLatest = sFile + _T("LATEST");
                
                int nItem = m_list.InsertItem(m_list.GetItemCount(), sName);
                m_list.SetItemText(nItem, FileInfo_Base, sBase);
                m_list.SetItemText(nItem, FileInfo_Curr, sCurr);
                m_list.SetItemText(nItem, FileInfo_Prev, sPrev);

                m_list.SetItemToolTipText(nItem, FileInfo_Name, sName);
                m_list.SetItemToolTipText(nItem, FileInfo_Base, sBase);
            }
        }

        while(m_cache[pos] == _T('\r') || m_cache[pos] == _T('\n')){
            pos ++;
        }

        m_cache = m_cache.Mid(pos);
        pos = m_cache.FindOneOf(_T("\r\n"));
    }

    TRACE(_T("Output: %s:\n"), pszOutput);
}

void CclearcasehelperDlg::CClearRedirector::WriteStdError(LPCTSTR pszError)
{
    TRACE(_T("Error: %s\n"), pszError);
}

//////////////////////////////////////////////////////
void CclearcasehelperDlg::PrepareCIPT()
{
    ASSERT(m_state == StateLoginPrepare);

    CString	strAddress = "http://cdma.gtss.mot.com/scm/cipt/";

    VARIANT v;
    v.vt=VT_I4;
    v.lVal=navNoHistory;	

    VARIANT k;
    k.vt=VT_BSTR;
    k.bstrVal=A2BSTR("_self");

    m_state = StateLoginCheck;

    m_loginWeb.Navigate(strAddress,&v,&k,0,0);

    m_status.SetWindowText("Checking CIPT status ...");

    return; // wait DOCFINISH ...
}

void CclearcasehelperDlg::GenerateCIPT(CString sOld, CString sNew)
{
}

static UINT gGetTwoFileHomeThread ( void* pv )
{
    CclearcasehelperDlg* pDlg = (CclearcasehelperDlg*) pv;

    pDlg->GetTwoFileHomeThread();

    return 0;
}


static UINT gWorkerThread ( void* pv )
{
    CclearcasehelperDlg* pDlg = (CclearcasehelperDlg*) pv;

    pDlg->RetrieveCIPTThreadProc();

    return 0;
}

static void DownloadFile(CString sUrl, CString sFile)
{
    SOCKET conn;
    conn=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(conn==INVALID_SOCKET){
        ASSERT(0);
        throw std::string("FAILED: No Resource");
    }

    if(connect(conn,(struct sockaddr*)&(_cipt_init.cipt_addr),sizeof(_cipt_init.cipt_addr)))
    {
        ASSERT(0);
        closesocket(conn);
        throw std::string("FAILED: No Resource");
    }

    char buff[512];
    sprintf(buff,"GET %s\r\n\r\n", sUrl);
    send(conn,buff,strlen(buff),0);

    CFile f;
    if(TRUE != f.Open(sFile,CFile::modeCreate | CFile::modeWrite)){
        throw (std::string("FAILED: Cannot open: ") + (const char *)sFile);
    }

    int y;
    while(y=recv(conn,buff,512,0))
    {
        f.Write(buff,y);
    }

    f.Close();

    TRACE("File downloaded and saved :- %s\n",sFile);
    closesocket(conn);
}

LRESULT CclearcasehelperDlg::OnDocumentComplete(LPDISPATCH pDisp, VARIANT FAR* URL)
{
    VARIANT v;
    v.vt=VT_I4;
    v.lVal=navNoHistory;	

    VARIANT k;
    k.vt=VT_BSTR;
    k.bstrVal=A2BSTR("_self");

    //WELL, page finished ... check ..
    if(m_state == StateLoginCheck){
        CWebPage page;
        page.SetDocument(m_loginWeb.get_Document());

        CComVariant result;
        page.CallJScript("hasBeenLogin", &result);

        if(result == CComVariant(true)){
            m_state = StateLoginSuccess;
            //m_loginWeb.Navigate("http://ciptweb.cig.mot.com:8000/src/perl/cipt.cgi?menu=1", &v, &k, NULL, NULL);
            PostMessage(WM_MY_MESSAGE, MSG_OK);

            return 0;
        }

        m_status.SetWindowText("CIPT requires login information ... Please login ...");
        // well, we need to perform login now ...
        // insert a JS function to do this ...
        CLoginDlg dlg;
        if(dlg.DoModal() != IDOK){
            PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
            return 0;
        }

        char path[MAX_PATH + 1];
        DWORD dwRet = GetModuleFileName(0, path, MAX_PATH);
        if(dwRet && dwRet <= MAX_PATH){
            path[dwRet] = 0;
        }else{
            AfxMessageBox(_T("Unknown Error."), MB_OK | MB_ICONSTOP);
            PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
            return 0;
        }

        CString sHelper;
        sHelper.Format("res://%s/JS/#%d", path, IDR_HELPER);
        page.InsertJScript(sHelper);

        page.CallJScript(_T("perform_login"), dlg.m_user.MakeLower(), dlg.m_pass);

        m_state = StateLoginPerform;

        m_status.SetWindowText("CIPT checking login information ... Please wait ...");

        return 0;
    }

    if(m_state == StateLoginPerform){
        CWebPage page;
        page.SetDocument(m_loginWeb.get_Document());

        CComVariant result;

        if(!page.CallJScript("hasBeenLogin", &result)){
            m_state = StateLoginSuccess;
            PostMessage(WM_MY_MESSAGE, MSG_OK);
            //m_loginWeb.Navigate("http://ciptweb.cig.mot.com:8000/src/perl/cipt.cgi?menu=1", &v, &k, NULL, NULL);
            return 0;
        }

        if(result == CComVariant(true)){
            m_state = StateLoginSuccess;
            PostMessage(WM_MY_MESSAGE, MSG_OK);
            //m_loginWeb.Navigate("http://ciptweb.cig.mot.com:8000/src/perl/cipt.cgi?menu=1", &v, &k, NULL, NULL);
            return 0;
        }

        AfxMessageBox(_T("Failed to login to CIPT. Please retry."), MB_OK | MB_ICONSTOP);

        m_state = StateLoginFailed;
        PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
        return 0;
    }
/*
    if(m_state == StateLoginSuccess){
        // well, let's check the URL for two file compare ...
        // get the document ...
        CWebPage page;
        page.SetDocument(m_loginWeb.get_Document());

        CString content = page.GetHTMLContent();
        CString old = content;

        content.MakeUpper();

        // this is for SABLIME
        int pos = content.Find("CIPTPRODDISPLAY");

        // this if for CLEARQUEST
        if(pos > 0){
            pos = content.Find("CIPTPRODDISPLAY", pos + 1);
        }

        // this if for clearcase branch
        if(pos > 0){
            pos = content.Find("CIPTPRODDISPLAY", pos + 1);
        }

        // now we get to compare two files
        if(pos > 0){
            pos = content.Find("CIPTPRODDISPLAY", pos + 1);
        }

        if(pos > 0){
            while(pos >= 0 && content[pos] != '<'){
                pos --;
            }
        }

        if(pos > 0){
            pos = content.Find("HREF", pos + 1);
        }

        if(pos <= 0){
            AfxMessageBox("Cannot validate CIPT menu1. Please retry!", MB_OK | MB_ICONSTOP);
            PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
            return 0;
        }

        while(pos < content.GetLength() && content[pos] != '"'){
            pos ++;
        }

        if(pos >= content.GetLength()){
            AfxMessageBox("Cannot validate CIPT menu2. Please retry!", MB_OK | MB_ICONSTOP);
            PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
            return 0;
        }

        pos ++; //omit '"'
        int start = pos;
        while(pos < content.GetLength() && content[pos] != '"'){
            pos ++;
        }

        if(pos >= content.GetLength()){
            AfxMessageBox("Cannot validate CIPT menu3. Please retry!", MB_OK | MB_ICONSTOP);
            PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
            return 0;
        }

        CString url = "http://ciptweb.cig.mot.com:8000";
        url += old.Mid(start, pos - start);

        TRACE("URL: %s\n", url);

        m_sCIPTURL = url;

        m_state = StateCIPTTwoFile;
        m_loginWeb.Navigate(m_sCIPTURL, &v, &k, NULL, NULL);
        return 0;
    }

    if(m_state == StateCIPTTwoFile){
        char path[MAX_PATH + 1];
        DWORD dwRet = GetModuleFileName(0, path, MAX_PATH);
        if(dwRet && dwRet <= MAX_PATH){
            path[dwRet] = 0;
        }else{
            AfxMessageBox(_T("Unknown Error."), MB_OK | MB_ICONSTOP);
            PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
            return 0;
        }

        //well, we get two file. compare it and ...
        CWebPage page;
        page.SetDocument(m_loginWeb.get_Document());

        CString sHelper;
        sHelper.Format("res://%s/JS/#%d", path, IDR_HELPER);
        page.InsertJScript(sHelper);

        ASSERT(m_items.size());

        m_item = m_items.front();
        m_items.pop_front();

        // ok, let's handle this item
        TRACE("Handle item: %s%s, vold: %s, vnew: %s\n", m_item.path.c_str(), m_item.name.c_str(), m_item.vold.c_str(), m_item.vnew.c_str());
        char tempPath[MAX_PATH + 1];
        memset(tempPath, 0, sizeof(tempPath));

        GetTempPath(MAX_PATH, tempPath);

        // copy file to temp files for CIPT, CIPT won't recongize @@
        std::string fileold = m_item.path + m_item.name + "@@";
        fileold += m_item.vold;

        m_sOldPath = tempPath;
        m_sOldPath += "\\old.";
        m_sOldPath += m_item.name.c_str();

        ::CreateDirectory(m_sOldPath, NULL);

        m_sOld = m_sOldPath + "\\" + m_item.name.c_str();

        ::CopyFile(fileold.c_str(), m_sOld, FALSE); //todo

        std::string filenew = m_item.path + m_item.name + "@@";
        filenew += m_item.vnew;

        m_sNewPath = tempPath;
        m_sNewPath += "\\new.";
        m_sNewPath += m_item.name.c_str();

        ::CreateDirectory(m_sNewPath, NULL);

        m_sNew = m_sNewPath + "\\" + m_item.name.c_str();
        ::CopyFile(filenew.c_str(), m_sNew, FALSE);

        m_state = StateFileRequest;
        page.CallJScript(_T("perform_compare"), m_sOld, m_sNew);
        return 0;
    }

    if(m_state == StateFileRequest){
        CWebPage page;
        page.SetDocument(m_loginWeb.get_Document());

        CString content = page.GetHTMLContent();
        CString old = content;

        content.MakeUpper();

        // find the link ...
        int pos = content.Find("URL=");
        if(pos > 0){
            pos += 4;   //omit this
            int end = pos;
            while(end < 512 && content[end] != '"'){
                end ++;
            }

            if(end >= 512){
                AfxMessageBox("INVALID CIPT response. Please retry!", MB_OK | MB_ICONSTOP);
                PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
                return 0;
            }

            CString url = "http://ciptweb.cig.mot.com:8000";
            url += content.Mid(pos, end - pos);

            CString sPath;
            m_output.GetWindowText(sPath);
            sPath += "\\";
            sPath += m_item.name.c_str();
            sPath += ".pdf";

            TRACE("Download file: %s to file: %s\n", url, sPath);
            DownloadFile(url, sPath);
        }else{
            AfxMessageBox("INVALID CIPT response. Please retry!", MB_OK | MB_ICONSTOP);
            PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
        }

        if(!m_sOld.IsEmpty()){
            ::DeleteFile(m_sOld);
            m_sOld.Empty();
        }
        
        if(!m_sNew.IsEmpty()){
            ::DeleteFile(m_sNew);
            m_sNew.Empty();
        }

        if(!m_sOldPath.IsEmpty()){
            ::RemoveDirectory(m_sOldPath);
            m_sOldPath.Empty();
        }

        if(!m_sNewPath.IsEmpty()){
            ::RemoveDirectory(m_sNewPath);
            m_sNewPath.Empty();
        }

        if(m_items.size() == 0){
            PostMessage(WM_MY_MESSAGE, MSG_DONE);
            return 0;
        }

        // well, next time ...
        m_state = StateCIPTTwoFile;
        m_loginWeb.Navigate(m_sCIPTURL, &v, &k, NULL, NULL);

        return 0;
    }
    */

    return 0;
}

LRESULT CclearcasehelperDlg::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
    if(wParam == MSG_DONE){
        EnableSubControls(TRUE);

        m_status.SetWindowText("Please choose an operation.");
        m_state = StateLoginPrepare;
        return 0;
    }

    if(wParam == MSG_FAILURE){
        m_state = StateLoginPrepare;
        m_status.SetWindowText("Operation failed! Please retry.");
        EnableSubControls(TRUE);
        return 0;
    }

    switch(m_state){
case StateLoginFailed:
    break;
case StateLoginSuccess:
    // now we can try to download ...
    // let it navigate to "http://ciptweb.cig.mot.com:8000/src/perl/cipt.cgi?menu=1"
    m_status.SetWindowText("CIPT login status OK ... checking CIPT home ...");

    m_state = StateCIPTPrepare;
    {
        CWinThread* pWorkerThread;
        pWorkerThread = AfxBeginThread ( gGetTwoFileHomeThread, this,
            THREAD_PRIORITY_NORMAL, 0, 
            CREATE_SUSPENDED );

        if ( NULL != pWorkerThread ) {
            // Kick off the download!
            pWorkerThread->ResumeThread();
        }
        else {
            AfxMessageBox(_T("Failed to start to CIPT preparation thread. Please retry."), MB_OK | MB_ICONSTOP);
            PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
        }
    }
    break;
case StateCIPTPrepare:
    // ok, we can start the threads to generate CIPT package
    // start worker thread now ...
    m_status.SetWindowText("Generating CIPT packages ... Please wait ...");

    {
        std::list<CWinThread *> workers;

        while(workers.size() < 5){
            if(workers.size() >= m_items.size()){
                break;
            }

            CWinThread *pThread;
            pThread = AfxBeginThread(gWorkerThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
            if(NULL == pThread){
                break;
            }

            workers.push_back(pThread);
        }

        if(workers.size() == 0){
            AfxMessageBox("Cannot start CIPT generation thread!", MB_OK | MB_ICONSTOP);
            PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
        }else{
            while(workers.size()){
                CWinThread *pThread = workers.front();
                pThread->ResumeThread();
                workers.pop_front();
            }
        }
    }
    break;
default:
    ASSERT(0);
    break;
    }

    return 0;
}

void CclearcasehelperDlg::GetTwoFileHomeThread()
{
    ASSERT(m_state == StateCIPTPrepare);

    CHTTPClient client;

    client.SetHost("ciptweb.cig.mot.com");
    client.SetPort(8000);
    if(client.TryGet("/src/perl/cipt.cgi?menu=1") == false){
        AfxMessageBox("Cannot retrieve CIPT menu. Please retry!", MB_OK | MB_ICONSTOP);
        PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
        return;
    }

    // we check the content and find out the link for two file compare ...
    CString content = client.GetContent();
    CString old = content;

    content.MakeUpper();

    // this is for SABLIME
    int pos = content.Find("CIPTPRODDISPLAY");

    // this if for CLEARQUEST
    if(pos > 0){
        pos = content.Find("CIPTPRODDISPLAY", pos + 1);
    }

    // this if for clearcase branch
    if(pos > 0){
        pos = content.Find("CIPTPRODDISPLAY", pos + 1);
    }

    // now we get to compare two files
    if(pos > 0){
        pos = content.Find("CIPTPRODDISPLAY", pos + 1);
    }

    if(pos > 0){
        while(pos >= 0 && content[pos] != '<'){
            pos --;
        }
    }

    if(pos > 0){
        pos = content.Find("HREF", pos + 1);
    }

    if(pos <= 0){
        AfxMessageBox("Cannot validate CIPT menu1. Please retry!", MB_OK | MB_ICONSTOP);
        PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
        return;
    }

    while(pos < content.GetLength() && content[pos] != '"'){
        pos ++;
    }

    if(pos >= content.GetLength()){
        AfxMessageBox("Cannot validate CIPT menu2. Please retry!", MB_OK | MB_ICONSTOP);
        PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
        return;
    }

    pos ++; //omit '"'
    int start = pos;
    while(pos < content.GetLength() && content[pos] != '"'){
        pos ++;
    }

    if(pos >= content.GetLength()){
        AfxMessageBox("Cannot validate CIPT menu3. Please retry!", MB_OK | MB_ICONSTOP);
        PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
        return;
    }

    CString url = old.Mid(start, pos - start);

    TRACE("URL: %s\n", url);
    if(client.TryGet(url) == false){
        AfxMessageBox("Cannot retrieve CIPT comparison page. Please retry!", MB_OK | MB_ICONSTOP);
        PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
        return;
    }

    content = client.GetContent();
    old = content;
    content.MakeUpper();
    // for each <INPUT, get the name, value pair and put them to Post Argument ...

    gPostArguments.clear();

    pos = content.Find("<INPUT");
    while(pos >= 0){
        int start = -1, name_start = -1, name_end = -1, 
            value_start = -1, value_end = -1;
        start = pos;

        while(pos < content.GetLength() && content[pos] != '>'){
            pos ++;
        }

        if(pos >= content.GetLength()){
            AfxMessageBox("Cannot validate CIPT comparison page 1. Please retry!", MB_OK | MB_ICONSTOP);
            PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
            return;
        }

        CString input = content.Mid(start, pos - start);

        name_start = input.Find("NAME");
        if(name_start >= 0){
            while(name_start < input.GetLength() && (input[name_start] != '=')){
                name_start ++;
            }

            name_start ++;

            while(name_start < input.GetLength() && ((input[name_start] == ' ') || (input[name_start] == '\t'))){
                name_start ++;
            }

            char omit;
            bool bOmit = false;
            if(input[name_start] == '\"' || input[name_start] == '\''){
                bOmit = true;
                omit = input[name_start];
                name_start ++;
            }

            if(name_start < input.GetLength()){
                name_end = name_start + 1;

                if(bOmit){
                    while(name_end < input.GetLength() && input[name_end] != omit){
                        name_end ++;
                    }
                }else{
                    while(name_end < input.GetLength() && input[name_end] != ' ' && input[name_end] != '\t'){
                        name_end ++;
                    }
                }
            }
        }

        value_start = input.Find("VALUE");
        if(value_start >= 0){
            while(value_start < input.GetLength() && input[value_start ] != '='){
                value_start ++;
            }

            value_start ++;

            while(value_start < input.GetLength() && (input[value_start ] == ' ' || input[value_start] == '\t')){
                value_start ++;
            }

            char omit;
            bool bOmit = false;
            if(input[value_start] == '\"' || input[value_start] == '\''){
                bOmit = true;
                omit = input[value_start];
                value_start ++;
            }

            if(value_start < input.GetLength()){
                value_end = value_start + 1;

                if(bOmit){
                    while(value_end < input.GetLength() && input[value_end] != omit){
                        value_end ++;
                    }
                }else{
                    while(value_end < input.GetLength() && input[value_end] != ' ' && input[value_end] != '\t'){
                        value_end ++;
                    }
                }
            }
        }

        if(name_start < 0 ||
            name_end < 0 ||
            value_start < 0 ||
            value_end < 0){
                ;
        }else{
            std::string name = (const char *)old.Mid(start + name_start, name_end - name_start);
            std::string value = (const char *)old.Mid(start + value_start, value_end - value_start);

            if(name == "oldfile" || name == "newfile"){
                continue;   // omit this ...
            }

            gPostArguments[name] = value;
            TRACE("Add %s/%s to post map.\n", name.c_str(), value.c_str());
        }

        pos = content.Find("<INPUT", pos + 1);
    }

    PostMessage(WM_MY_MESSAGE, MSG_OK);
}

void CclearcasehelperDlg::RetrieveCIPTThreadProc()
{
    char tempPath[MAX_PATH + 1];
    memset(tempPath, 0, sizeof(tempPath));

    GetTempPath(MAX_PATH, tempPath);
    TRACE("Temporary path: %s\n", tempPath);

    CIPTItem item;

    bool bLast = false;
    
    CString sCookie = "";
    CCIPTOptions &options = CCIPTOptions::GetOptionsDlg();
    sCookie.Format("ccook=o_sbs%%3E%s%%23o_iws%%3E%s%%23o_plo%%3E%s%%23o_ncsl%%3E%s%%23o_cl%%3E%s%%23o_clnob%%3E%d%%23o_clnoa%%3E%d;",
        (TRUE == options.IsSideBySide()) ? "yes" : "no", 
        (TRUE == options.IgnoreWhitespace()) ? "yes" : "no", 
        (TRUE == options.ShowInPortait()) ? "portrait" : "landscape", 
        (TRUE == options.ShowNCSL()) ? "yes" : "no", 
        (TRUE == options.ShowLegend()) ? "yes" : "no", 
        options.GetBeforeLine(), options.GetAfterLine());

    while(true) {
        m_mutex.Lock();
        if(m_items.size() == 1){
            bLast = true;
        }else if(m_items.size() == 0){
            m_mutex.Unlock();
            break;
        }

        item = m_items.front();
        m_items.pop_front();
        m_mutex.Unlock();

        // ok, let's handle this item
        /*
        TRACE("Handle item: %s%s, vold: %s, vnew: %s\n", item.path.c_str(), item.name.c_str(), item.vold.c_str(), item.vnew.c_str());
        CString sStatus;
        sStatus.Format("Handling file [%s%s]", item.path.c_str(), item.name.c_str()); 
        m_status.SetWindowText(sStatus);
        */

        std::string fileold = item.path + item.name + "@@";
        fileold += item.vold;

        std::string filenew = item.path + item.name + "@@";
        filenew += item.vnew;

        PostArgumentMap files;
        PostArgumentMap filenames;
        files["oldfile"] = fileold;
        files["newfile"] = filenew;
        filenames["oldfile"] = (item.vold + "_" + item.name);
        filenames["newfile"] = item.name;

        int retries = 3;
        while(retries) try{
            m_files.SetItemText(item.item, FileInfo_Status, "Generating");
            CString content = RawPostForm(_cipt_init.cipt_addr, std::string("/src/perl/cipt.cgi"), std::string((const char *)sCookie), gPostArguments, files, filenames).c_str();
            CString old = content;

            content.MakeUpper();

            // find the link ...
            int length = content.GetLength();
            int pos = content.Find("URL=");
            if(pos > 0){
                pos += 4;   //omit this
                int end = pos;
                while(end < length && content[end] != '"'){
                    end ++;
                }

                if(end >= length){
                    throw std::string("INVALID CIPT response!");
                }else{
                    CString url = "http://ciptweb.cig.mot.com:8000";
                    url += old.Mid(pos, end - pos);

                    CString sPath;
                    m_output.GetWindowText(sPath);
                    sPath += "\\";
                    sPath += item.name.c_str();
                    sPath += ".pdf";

                    TRACE("Download file: %s to file: %s\n", url, sPath);
                    /*
                    sStatus.Format("Downloading CIPT result for [%s%s]", item.path.c_str(), item.name.c_str()); 
                    m_status.SetWindowText(sStatus);
                    */

                    m_files.SetItemText(item.item, FileInfo_Status, "Downloading");
                    DownloadFile(url, sPath);
                    m_files.SetItemText(item.item, FileInfo_Status, "Done");
                }
            }else{
                throw std::string("Invalid CIPT Response.");
                //AfxMessageBox("INVALID CIPT response. Please retry!", MB_OK | MB_ICONSTOP);
                //PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
            }        

            break;
        }catch(std::string &s){
            retries --;
            if(retries == 0) {
                m_files.SetItemText(item.item, FileInfo_Status, s.c_str());
                m_files.SetItemToolTipText(item.item, FileInfo_Status, s.c_str()); 
                m_files.SetItemColors(item.item, FileInfo_Status, RGB(255, 0, 0), (COLORREF) -1);
            } else {
                std::string sRetry = "Failed with message [" + s + "]. Waiting retry ...";
                m_files.SetItemText(item.item, FileInfo_Status, sRetry.c_str());
                m_files.SetItemToolTipText(item.item, FileInfo_Status, sRetry.c_str()); 
            }
        } 
    }

    if(bLast){
        PostMessage(WM_MY_MESSAGE, MSG_DONE);
    }
    /*
    item = m_items.front();
    m_items.pop_front();

    // ok, let's handle this item
    TRACE("Handle item: %s%s, vold: %s, vnew: %s\n", item.path.c_str(), item.name.c_str(), item.vold.c_str(), item.vnew.c_str());

    // copy file to temp files for CIPT, CIPT won't recongize @@
    std::string fileold = item.path + item.name + "@@";
    fileold += item.vold;

    CString sOldPath = tempPath;
    sOldPath += "\\old.";
    sOldPath += item.name.c_str();

    ::CreateDirectory(sOldPath, NULL);

    CString sOld = sOldPath + "\\" + item.name.c_str();

    ::CopyFile(fileold.c_str(), sOld, FALSE); //todo

    std::string filenew = item.path + item.name + "@@";
    filenew += item.vnew;

    CString sNewPath = tempPath;
    sNewPath += "\\new.";
    sNewPath += item.name.c_str();

    ::CreateDirectory(sNewPath, NULL);

    CString sNew = sNewPath + "\\" + item.name.c_str();
    ::CopyFile(filenew.c_str(), sNew, FALSE);

    W3Client client;
    if(client.Connect("http://ciptweb.cig.mot.com:8000") == false){
        ASSERT(0);;
        TRACE("Failed!!!!\n");
        return;
    }

    PostArgumentMap::iterator it = gPostArguments.begin();
    while(it != gPostArguments.end()){
        client.AddPostArgument(it->first.c_str(), it->second.c_str(), false);
        it ++;
    }


    client.AddPostArgument("oldfile", sOld, true);
    client.AddPostArgument("newfile", sNew, true);

    if(client.Request("/src/perl/cipt.cgi", W3Client::reqPostMultipartsFormdata) == false){
        ASSERT(0);
        TRACE("FAILED!!!!!!!!!!!!!!!!!!\n");
        return;
    }

    char buf[512]="\0";
    if(client.Response(reinterpret_cast<unsigned char*>(buf), 512)>0){
        //try to parse out the URL ...
        CString content = buf;
        int pos = content.Find("URL=");
        if(pos > 0){
            pos += 4;   //omit this
            int end = pos;
            while(end < 512 && content[end] != '"'){
                end ++;
            }

            if(end >= 512){
                ASSERT(0);
            }else{
                //CString url = "http://ciptweb.cig.mot.com:8000";
                //url += content.Mid(pos, end - pos);
                
                CString url = content.Mid(pos, end - pos);
                CString sPath;
                m_output.GetWindowText(sPath);
                sPath += "\\";
                sPath += item.name.c_str();
                sPath += ".pdf";

                TRACE("Download file: %s to file: %s\n", url, sPath);
                DownloadFile(url, sPath);
            }
        }else{
            ASSERT(0);
        }
    }

    ::DeleteFile(sOld);
    ::DeleteFile(sNew);
    ::RemoveDirectory(sOldPath);
    ::RemoveDirectory(sNewPath);

    PostMessage(WM_MY_MESSAGE, MSG_FINISHFILE);
    */
}

void CclearcasehelperDlg::OnBnClickedCiptoption()
{
    // TODO: Add your control notification handler code here
    CCIPTOptions::GetOptionsDlg().DoModal();
}
