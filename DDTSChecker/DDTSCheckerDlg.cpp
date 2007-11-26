// DDTSCheckerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DDTSChecker.h"
#include "DDTSCheckerDlg.h"
#include "HTTPClient.h"
#include <Winnetwk.h>
#pragma comment(lib, "Mpr")

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


// CDDTSCheckerDlg dialog




CDDTSCheckerDlg::CDDTSCheckerDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CDDTSCheckerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_host = "wsd-ddts.pcs.mot.com";
    m_HTTPClient.SetHost("wsd-ddts.pcs.mot.com");
    m_HTTPClient.SetPort(80);
    m_state = StateIdle;
}

void CDDTSCheckerDlg::DoDataExchange(CDataExchange* pDX)
{
    CResizableDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ITEMS, m_items);
    DDX_Control(pDX, IDC_CR, m_cr);
    DDX_Control(pDX, IDC_RULE, m_rule);
    DDX_Control(pDX, IDC_QUERY, m_btnQuery);
    DDX_Control(pDX, IDC_BROWSE, m_btnBrowse);
    DDX_Control(pDX, IDC_ABORT, m_btnAbort);
    DDX_Control(pDX, IDC_OPEN, m_btnOpen);
    DDX_Control(pDX, IDC_VALIDATE, m_btnValidate);
}

BEGIN_MESSAGE_MAP(CDDTSCheckerDlg, CResizableDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_WM_SIZING()
    ON_WM_SIZE()
    ON_EN_CHANGE(IDC_CR, &CDDTSCheckerDlg::OnEnChangeCr)
    ON_BN_CLICKED(IDC_ABORT, &CDDTSCheckerDlg::OnBnClickedAbort)
    ON_BN_CLICKED(IDC_QUERY, &CDDTSCheckerDlg::OnBnClickedQuery)
    ON_BN_CLICKED(IDC_BROWSE, &CDDTSCheckerDlg::OnBnClickedBrowse)
    ON_MESSAGE(WM_THREAD_DONE, &CDDTSCheckerDlg::OnThreadDone)
    ON_BN_CLICKED(IDC_OPEN, &CDDTSCheckerDlg::OnBnClickedOpen)
    ON_BN_CLICKED(IDC_VALIDATE, &CDDTSCheckerDlg::OnBnClickedValidate)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CDDTSCheckerDlg)
    EASYSIZE(IDC_CRNUMBER, ES_BORDER, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, 0)
    EASYSIZE(IDC_CR, ES_BORDER, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, 0)
    EASYSIZE(IDC_QUERY, ES_BORDER, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, 0)
    EASYSIZE(IDOK, ES_KEEPSIZE, ES_BORDER, ES_BORDER, ES_KEEPSIZE, 0)
    EASYSIZE(IDC_ITEMS, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
    EASYSIZE(IDC_USERULE, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, 0)
    EASYSIZE(IDC_RULE, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
    EASYSIZE(IDC_BROWSE, ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP


// CDDTSCheckerDlg message handlers

BOOL CDDTSCheckerDlg::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
    INIT_EASYSIZE;

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
    m_items.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_items.InsertColumn(0, "Item", LVCFMT_LEFT, 100);
    m_items.InsertColumn(1, "Rule", LVCFMT_LEFT, 250);
    m_items.InsertColumn(2, "Results", LVCFMT_LEFT, 50);

    ResetColumnWidth();

    //get current user name
    DWORD length = 255;
    char name[256];
    memset(name, 0, sizeof(name));
    ::WNetGetUser(NULL, name, &length);

    CString sTmp = name;
    sTmp.MakeLower();

    m_user = sTmp.GetBuffer(sTmp.GetLength());

    CString sRule = theApp.GetProfileString("Settings", "rule", "\\\\zch49app08\\pcu\\RSD\\BR_Checklist\\rules.xml");
    m_rule.SetWindowText(sRule);
    LoadRule();
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDDTSCheckerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CDDTSCheckerDlg::OnPaint()
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
HCURSOR CDDTSCheckerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDDTSCheckerDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
    CResizableDialog::OnSizing(fwSide, pRect);

//        EASYSIZE_MINSIZE(m_initRect.Width(), m_initRect.Height(), fwSide, pRect);
}

void CDDTSCheckerDlg::OnSize(UINT nType, int cx, int cy)
{
    CResizableDialog::OnSize(nType, cx, cy);

    if(m_items.GetSafeHwnd()){
        UPDATE_EASYSIZE;

        ResetColumnWidth();
    }
}

void CDDTSCheckerDlg::ResetColumnWidth()
{
    CRect rc;
    m_items.GetClientRect(&rc);
    int cx = rc.Width();
    
    //the first, 3rd, 4th column keep size, while the 2nd change its'size
    int width = m_items.GetColumnWidth(0) + m_items.GetColumnWidth(1);
    if(width < cx){
        width = cx - width;
    }else{
        width = 0;
    }
    
    if(width < 100){
        width = 100;
    }
    
    m_items.SetColumnWidth(2, width - 15);
}

void CDDTSCheckerDlg::OnEnChangeCr()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CResizableDialog::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
    CString sText;
    m_cr.GetWindowText(sText);
    sText.Trim();
    if(sText.IsEmpty()){
        m_btnQuery.EnableWindow(FALSE);
        m_btnValidate.EnableWindow(FALSE);
    }else{
        m_btnQuery.EnableWindow(TRUE);
        m_btnValidate.EnableWindow(TRUE);
    }
}

void CDDTSCheckerDlg::OnBnClickedAbort()
{
    // TODO: Add your control notification handler code here
    m_cr.EnableWindow(TRUE);
    m_btnQuery.EnableWindow(TRUE);
    m_btnBrowse.EnableWindow(TRUE);
    m_btnAbort.EnableWindow(FALSE);

    m_HTTPClient.Reset();

    ASSERT(m_state != StateIdle);
    m_state = StateIdle;
}

void CDDTSCheckerDlg::OnBnClickedQuery()
{
    if(m_rules.size() == 0){
        AfxMessageBox("Rule file may not be valid. Please browse a usable rule file and try again.");
        return;
    }

    UpdateData();

    m_cr.EnableWindow(FALSE);
    m_btnQuery.EnableWindow(FALSE);
    m_btnBrowse.EnableWindow(FALSE);
    m_btnAbort.EnableWindow(TRUE);

    ASSERT(m_state == StateIdle);
    m_state = StateGetHTML;

    CString sCR;
    m_cr.GetWindowText(sCR);
    sCR.Trim();
    
    CString part1 = sCR.Left(3);
    CString part2 = sCR.Mid(3, 2);
    CString part3 = sCR.Mid(5);

    part1.MakeUpper();
    part2.MakeLower();
    sCR = part1 + part2 + part3;
    m_bug = sCR.GetBuffer(sCR.GetLength());

    // start up get HTML thread
    AfxBeginThread(&CDDTSCheckerDlg::GetCRThread, this);
}

void CDDTSCheckerDlg::OnBnClickedBrowse()
{
    CFileDialog dlg(TRUE, ".xml", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "Rule Files (*.xml)|*.xml|All Files (*.*)|*.*||");

    if(dlg.DoModal() != IDOK){
        return;
    }

    m_rule.SetWindowText(dlg.GetPathName());

    if(LoadRule()){
        theApp.WriteProfileString("Settings", "rule", dlg.GetPathName());
    }
}


LRESULT CDDTSCheckerDlg::OnThreadDone(WPARAM wParam, LPARAM lParam)
{

    switch(wParam){
        case WPARAM_NEEDLOGIN:
            if(lParam){
                if(IDYES != AfxMessageBox("Cannot login to DDTS. \r\nPlease make sure you have provided correct information. \r\nWould you like to retry?",
                    MB_ICONQUESTION | MB_YESNO)){
                        HandleException();

                        break;
                }
            }

            HandleLogin();
            break;
        case WPARAM_LOGINDONE:
            AfxBeginThread(&CDDTSCheckerDlg::GetCRThread, this);
            break;
        case WPARAM_GETHTML:
            HandleHTML();
            break;
        case WPARAM_EXCEPTION:
            HandleException();
            break;
        default:
            TRACE("Something wrong?\n");
            ASSERT(0);
            break;
    }

    return 0;
}

UINT CDDTSCheckerDlg::LoginThread(LPVOID pParam)
{
    ASSERT(pParam);
    CDDTSCheckerDlg *pDlg = (CDDTSCheckerDlg *)pParam;

    // handle login here ...
    CHTTPClient client;
    client.SetHost(pDlg->m_host);
    PostArgumentMap params;
    params[std::string("destination")] = std::string("ddts/ddts_main");
    params[std::string("credential_0")] = pDlg->m_user;
    params[std::string("credential_1")] = pDlg->m_password;

    TRACE("Try to request LOGIN\n");
    if(!client.TryPost("/LOGIN", params)){
        TRACE("LOGIN FAILED\n");
        pDlg->PostMessage(WM_THREAD_DONE, WPARAM_NEEDLOGIN, 1);
        return 0;
    }

    //get back data ...
    const char *pHtml = client.GetContent();
    CString html = "";
    if(pHtml){
        html = pHtml;
    }

    TRACE("Data got: %s\n", html);
    if(html.Find("login_form") > 0){
        TRACE("LOGIN FAILED\n");
        pDlg->PostMessage(WM_THREAD_DONE, WPARAM_NEEDLOGIN, 1);
    }else{
        TRACE("LOGIN OK\n");
        pDlg->PostMessage(WM_THREAD_DONE, WPARAM_LOGINDONE, 1);
    }
    return 0;
}

UINT CDDTSCheckerDlg::GetCRThread(LPVOID pParam)
{
    ASSERT(pParam);
    CDDTSCheckerDlg *pDlg = (CDDTSCheckerDlg *)pParam;

    pDlg->m_HTTPClient.Reset();

    PostArgumentMap params;
    params[std::string("LastForm")] = std::string("DumpBug");
    params[std::string("REMOTE_USER")] = pDlg->m_user;
    params[std::string("bug_id")] = pDlg->m_bug;

    if(!pDlg->m_HTTPClient.TryPost("/ddts_auth/ddts_main", params)){
        TRACE("Failed to post CR information\n");
        pDlg->PostMessage(WM_THREAD_DONE, WPARAM_EXCEPTION);
        return 0;
    }

    pDlg->PostMessageA(WM_THREAD_DONE, WPARAM_GETHTML);

    return 0;
}

#include "LoginDlg.h"
void CDDTSCheckerDlg::HandleLogin()
{
    CLoginDlg login;
    login.m_user = m_user.c_str();
    login.m_pass = m_password.c_str();

    if(login.DoModal() != IDOK){
        HandleException();
        return;
    }

    // start the login process by starting login thread ...
    login.m_user.MakeLower();
    m_user = login.m_user.GetBuffer(login.m_user.GetLength());
    m_password = login.m_pass.GetBuffer(login.m_pass.GetLength());

    //TRACE("user: %s, pass: %s\n", m_user.c_str(), m_password.c_str());

    //AfxBeginThread(&CDDTSCheckerDlg::LoginThread, this);
    LoginThread(this);
}

void CDDTSCheckerDlg::HandleHTML()
{
    //get back data ...
    const char *pHtml = m_HTTPClient.GetContent();
    CString html = "";
    if(pHtml){
        html = pHtml;
    }

    if(html.Find("login_form") > 0){
        PostMessage(WM_THREAD_DONE, WPARAM_NEEDLOGIN);
        return;
    }

    if(html.Find("Bad Defect Identifier") > 0){
        AfxMessageBox("Bad Defect Identifier");
        HandleException();
        return;
    }

    m_items.DeleteAllItems();
    //
    //now ... list the items ...
    for(size_t rule = 0; rule < m_rules.size(); rule ++){
        ApplyRule(html, m_rules[rule]);
        m_items.Invalidate();
    }

    OnBnClickedAbort();
}

void CDDTSCheckerDlg::HandleException()
{
    AfxMessageBox("Current operation cannot be finished due to exception.", MB_OK | MB_ICONSTOP);
    OnBnClickedAbort();
}

void CDDTSCheckerDlg::ApplyRule(CString html, Rule &rule)
{
    int nItem = m_items.InsertItem(m_items.GetItemCount(), rule.item);
    CString sRule;
    switch(rule.op){
        case rule.OPEqual:
            sRule.Format("%s %s", "Equal", rule.value);
            m_items.SetItemText(nItem, 1, sRule);
            break;
        case rule.OPLarger:
            sRule.Format("%s %s", "Larger Than", rule.value);
            m_items.SetItemText(nItem, 1, sRule);
            break;
        case rule.OPMust:
            m_items.SetItemText(nItem, 1, "Must Present");
            break;
        case rule.OPContain:
            sRule.Format("Contains %s", rule.value);
            m_items.SetItemText(nItem, 1, sRule);
            break;
        case rule.OPLink:
            sRule.Format("Has Link %s", rule.item);
            m_items.SetItemText(nItem, 1, sRule);
            break;
        default:
            m_items.SetItemText(nItem, 1, "Unknown rule");
            break;
    }

    int nPos = html.Find(rule.item);
    if(nPos < 0){
        m_items.SetItemText(nItem, 2, "item not found");
        m_items.SetItemColors(nItem, 2, RGB(255, 0, 0), RGB(255, 255, 255));
        return;
    }

    // well, now search for <td> ...</td>
    int nStart = html.Find("<td", nPos);
    int nEnd = html.Find("</td>", nPos);
    if(nStart >= 0){
        while(html[nStart] != '>'){
            nStart ++;
        }

        nStart ++;
    }

    if(nStart < 0 || nEnd < nStart){
        m_items.SetItemText(nItem, 2, "value");
        m_items.SetItemColors(nItem, 2, RGB(255, 0, 0), RGB(255, 255, 255));
    }

    CString sValue = html.Mid(nStart, nEnd - nStart);
    sValue.Trim();
    // now see the rule ...
    switch(rule.op){
        case rule.OPEqual:
            if(sValue.CompareNoCase(rule.value)){
                m_items.SetItemText(nItem, 2, "FAILED");
                m_items.SetItemColors(nItem, 2, RGB(255, 0, 0), RGB(255, 255, 255));
            }else{
                m_items.SetItemText(nItem, 2, "OK");
                m_items.SetItemColors(nItem, 2, RGB(0, 0, 255), RGB(255, 255, 255));
            }
            break;
        case rule.OPLarger:
            {
                int value = atoi(sValue);
                if(value <= atoi(rule.value)){
                    m_items.SetItemText(nItem, 2, "FAILED");
                    m_items.SetItemColors(nItem, 2, RGB(255, 0, 0), RGB(255, 255, 255));
                }else{
                    m_items.SetItemText(nItem, 2, "OK");
                    m_items.SetItemColors(nItem, 2, RGB(0, 0, 255), RGB(255, 255, 255));
                }
            }
            break;
        case rule.OPMust:
            if(sValue.IsEmpty()){
                m_items.SetItemText(nItem, 2, "FAILED");
                m_items.SetItemColors(nItem, 2, RGB(255, 0, 0), RGB(255, 255, 255));
            }else{
                m_items.SetItemText(nItem, 2, "OK");
                m_items.SetItemColors(nItem, 2, RGB(0, 0, 255), RGB(255, 255, 255));
            }
            break;
        case rule.OPContain:
            if(sValue.Find(rule.value) < 0){
                m_items.SetItemText(nItem, 2, "FAILED");
                m_items.SetItemColors(nItem, 2, RGB(255, 0, 0), RGB(255, 255, 255));
            }else{
                m_items.SetItemText(nItem, 2, "OK");
                m_items.SetItemColors(nItem, 2, RGB(0, 0, 255), RGB(255, 255, 255));
            }
            break;
        case rule.OPLink:
            {
                CString sLink;
                sLink.Format("%s</A>", rule.item);
                if(html.Find(sLink) < 0){
                    m_items.SetItemText(nItem, 2, "FAILED");
                    m_items.SetItemColors(nItem, 2, RGB(255, 0, 0), RGB(255, 255, 255));
                }else{
                    m_items.SetItemText(nItem, 2, "OK");
                    m_items.SetItemColors(nItem, 2, RGB(0, 0, 255), RGB(255, 255, 255));
                }
            }
            break;
        default:
            m_items.SetItemText(nItem, 2, "Invalid");
            m_items.SetItemColors(nItem, 2, RGB(255, 0, 0), RGB(255, 255, 255));
            break;
    }
}

#include "DOM.h"

bool CDDTSCheckerDlg::LoadRule()
{
    CString ruleFile;
    m_rule.GetWindowText(ruleFile);

    ruleFile.Trim();
 
    CppDom::Document doc;
    try{
        doc.load(ruleFile);
    }catch(...){
        AfxMessageBox("Failed to load rule file. Make sure the rule file format is correct.");
        return false;
    }

    // now we shall load the rules
    m_rules.clear();

    CppDom::Node *pRootAll = doc.getDocumentElement();
    if(pRootAll == NULL){
        AfxMessageBox("Failed to load rule file. Make sure the rule file format is correct.");
        return false;
    }

    CppDom::NodeListPtr rules = pRootAll->selectNodes("./rule");
    if(rules == NULL){
        AfxMessageBox("Failed to load rule file. Make sure the rule file format is correct.");
        return false;
    }

    for(int i = 0; i < rules->getLength(); i++){
        CppDom::Node *pRule = rules->item(i);
        if(pRule == NULL){
            ASSERT(0);
            continue;
        }

        CppDom::NamedNodeMap *pAttributes = pRule->getAttributes();
        if(!pAttributes){
            ASSERT(0);
            continue;
        }

        CppDom::Node *pItem = pAttributes->getNamedItem("item");
        CppDom::Node *pOP = pAttributes->getNamedItem("op");
        CppDom::Node *pValue = pAttributes->getNamedItem("value");

        if(!pItem || !pOP){
            ASSERT(0);
            continue;
        }
        CString sItem = pItem->getNodeValue();
        CString sOP = pOP->getNodeValue();
        CString sValue = "";
        if(pValue){
            sValue = pValue->getNodeValue();
        }

        sItem.Trim();
        sOP.Trim();
        sValue.Trim();
        if(sItem.IsEmpty()){
            ASSERT(0);
            continue;
        }

        //check type ...
        Rule rule;
        rule.item = sItem;
        rule.value = sValue;
        if(!sOP.CompareNoCase("must")){
            rule.op = rule.OPMust;
        }else if(!sOP.CompareNoCase("larger")){
            rule.op = rule.OPLarger;
            if(sValue.IsEmpty()){
                ASSERT(0);
                continue;
            }
        }else if(!sOP.CompareNoCase("contain")){
            rule.op = rule.OPContain;
            if(sValue.IsEmpty()){
                ASSERT(0);
                continue;
            }
        }else if(!sOP.CompareNoCase("link")){
            rule.op = rule.OPLink;
        }else if(!sOP.CompareNoCase("equal")){
            rule.op = rule.OPEqual;
            if(sValue.IsEmpty()){
                ASSERT(0);
                continue;
            }
        }else{
            ASSERT(0);
            continue;
        }

        m_rules.push_back(rule);
    }

    if(!m_rules.size()){
        AfxMessageBox("Empty rule file or no invalid rule found.");
        return false;
    }

    return true;
    //try to load the rule files here ...
    /*    
    Rule rule;
    rule.item = "Status Full Name";
    rule.op = rule.OPEqual;
    rule.value = "Resolved";

    m_rules.push_back(rule);

    rule.op = (Rule::Operation)5;
    rule.item = "Some item";
    m_rules.push_back(rule);

    rule.item = "Actual Analysis Hours";
    rule.op = rule.OPLarger;
    rule.value = "0";
    m_rules.push_back(rule);

    rule.item = "Inspection ID(s)";
    rule.op = rule.OPMust;
    rule.value = "";
    m_rules.push_back(rule);

    rule.item = "ConfigSpec";
    rule.op = rule.OPLink;
    rule.value = "";
    m_rules.push_back(rule);

    rule.item = "ConfigSpec";
    rule.op = rule.OPContain;
    rule.value = "";
    m_rules.push_back(rule);
    */
}

void CDDTSCheckerDlg::OnBnClickedOpen()
{

}

void CDDTSCheckerDlg::OnBnClickedValidate()
{
    UpdateData();

    CString sCR;
    m_cr.GetWindowText(sCR);
    sCR.Trim();
    
    CString part1 = sCR.Left(3);
    CString part2 = sCR.Mid(3, 2);
    CString part3 = sCR.Mid(5);

    part1.MakeUpper();
    part2.MakeLower();
    sCR = part1 + part2 + part3;
    m_bug = sCR.GetBuffer(sCR.GetLength());

    if(m_password.size() == 0) {
        CLoginDlg login;
        login.m_user = m_user.c_str();
        login.m_pass = m_password.c_str();

        if(login.DoModal() != IDOK){
            return;
        }

        login.m_user.MakeLower();
        m_user = login.m_user.GetBuffer(login.m_user.GetLength());
        m_password = login.m_pass.GetBuffer(login.m_pass.GetLength());
    }

    // TODO: Add your control notification handler code here
    CHTTPClient client;
    client.SetCredential(m_user, m_password);
    client.SetHost("uis.pcs.mot.com");
    //client.TryGet("/synergy/decompress/gsm_cr_report.cfm");

    PostArgumentMap params;
    params[std::string("crs")] = m_bug;//std::string("libnn00455");
//    params[std::string("submit")] = std::string("Submit");
//    params[std::string("reset")] = std::string("Reset");
    client.TryPost("/synergy/decompress/gsm_cr_report.cfm", params);

    TRACE("Response: \n");
    UINT length = client.GetContentLength();
    char *pResp = (char *)client.GetContent();
    while(length > 0) {
        if(length <= 400) {
            TRACE(pResp);
            break;
        }

        char ch = *(pResp + 400);
        *(pResp + 400) = 0;
        TRACE(pResp);
        *(pResp + 400) = ch;
        pResp += 400;
        length -= 400;
    }

    // search for "v01:", "v02:" etc ...
    std::map<string, string> rules;
    try {
        const char *pContent = client.GetContent();
        while(true) {
            const char *pos = strstr(pContent, "\r\n");
            if(pos == 0) {
                throw std::string("Invalid start for rules.");
            }

            while(*pos == '\r' || *pos == '\n') {
                pos ++;
            }

            pContent = pos; //search start of rule
            if(pos[3] == ':') {
                // found a rule ...
                break;
            }
        }

        if(pContent == 0) {
            throw std::string("invalid rule start");
        }

        while(true) {
            const char *pos = pContent;

            if(pos[3] != ':') {
                break;
            }

            std::string rule = std::string(pos, 3);
            pos += 4;

            const char *pos2 = strstr(pos, "\r\n");
            if(pos2 <= pos) {
                throw std::string("Invalid content for rules end");
            }
            std::string desc = std::string(pos, pos2 - pos);
            rules[rule] = desc;

            while(*pos2 == '\r' || *pos2 == '\n') {
                pos2 ++;
            }

            pContent = pos2;
        }

        if(rules.size() == 0) {
            throw std::string("No rules found.");
        }

        // ok, here, we search for CR start ...
        string crline;
        while(true) {
            const char *pos = pContent;

            while(*pos == ' ' || *pos == '\t') {
                pos ++;
            }

            const char *pos2 = strstr(pos, "\r\n");
            if(pos2 <= pos) {
                throw std::string("Invalid content for CR");
            }

            std::string cr = std::string(pos, m_bug.size());
            if(!stricmp(cr.c_str(), m_bug.c_str())) {
                crline = std::string(pos, pos2 - pos);
                break;  // we got the CR line
            }

            while(*pos2 == '\r' || *pos2 == '\n') {
                pos2 ++;
            }

            pContent = pos2;
        }

        // here, let's check the CR line ...
        //seperate to 3 fields, cr #, inspection #, and violated rules
        if(crline.size() == 0) {
            throw std::string("no CR line found.");
        }

        std::string::size_type p1 = crline.find_first_of('|');
        std::string::size_type p2 = crline.find_first_of('|', p1 + 1);
        CString cr = crline.substr(0, p1).c_str();
        p1 ++;
        CString id = crline.substr(p1, p2 - p1).c_str();
        CString miss = crline.substr(p2 + 1).c_str();

        cr.Trim();
        id.Trim();
        miss.Trim();

        if(miss.GetLength() == 0) {
            throw std::string("Invalid CR line found.");
        }

        if(miss.CompareNoCase("None") == 0) {
            AfxMessageBox("No rule violated!");
            return;
        }

        CString error = "Following item violated:\n";
        while(true) {
            int n = miss.Find(',');
            if(n < 0) {
                std::string item = miss;
                error += item.c_str();
                error += ":";
                error += rules[item].c_str();
                error += "\r\n";
                break;
            }

            std::string item = miss.Left(n);
            error += item.c_str();
            error += ":";
            error += rules[item].c_str();
            error += "\r\n";
            miss = miss.Mid(n + 1);
            miss.Trim();
        }

        AfxMessageBox(error);
    }
    catch(const std::string &err) {
        AfxMessageBox(err.c_str(), MB_OK | MB_ICONSTOP);
    }
    catch(...) {
        AfxMessageBox("Unknown error got. Please retry.", MB_OK | MB_ICONSTOP);
    }
}
