#include "stdafx.h"
#include "HTTPClient.h"
#include <sstream>
#include <list>
#include <map>

const TCHAR * CHTTPClient::DEFAULT_AGENT_NAME = _T("NETLIFE(0.9/;p)");
const TCHAR * CHTTPClient::HTTP_VERB_GET = _T("GET");
const TCHAR * CHTTPClient::HTTP_VERB_POST = _T("POST");
const TCHAR * CHTTPClient::HTTP_ACCEPT_TYPE = _T("*/*");
const TCHAR * CHTTPClient::HTTP_ACCEPT = _T("Accept: */*\r\n");

CHTTPClient::CHTTPClient()
{
  m_hInternet = 0;
  m_hHTTPConnection = 0;
  m_hHTTPRequest = 0;
  m_dwError = 0;

  m_sHost = _T("");
  m_port = INTERNET_DEFAULT_HTTP_PORT;
  m_pResponse = 0;
  m_length = 0;
}

CHTTPClient::~CHTTPClient()
{
  Reset();
}

void CHTTPClient::Reset()
{
  try{
    if(m_hHTTPRequest){
      ::InternetCloseHandle(m_hHTTPRequest);
      m_hHTTPRequest=NULL;
    }

    if(m_hHTTPConnection){
      ::InternetCloseHandle(m_hHTTPConnection);
      m_hHTTPConnection=NULL;
    }

    if(m_hInternet){
      ::InternetCloseHandle(m_hInternet);
      m_hInternet=NULL;
    }

    if(m_pResponse){
      delete [] m_pResponse;
    }

    m_pResponse = 0;

    m_length = 0;
  }catch(...){
    //ignore any exception
    TRACE(_T("Exception omitted.\n"));
  }

  m_dwError = 0;
}

bool CHTTPClient::TryConnect(const std::string &url, bool bPost)
{
  m_hInternet=::InternetOpen(DEFAULT_AGENT_NAME, INTERNET_OPEN_TYPE_PRECONFIG, _T(""), _T(""), 0);

  if(!m_hInternet){
    m_dwError=::GetLastError();
    return false;
  }

  m_state = StateConnect;
  m_hHTTPConnection=::InternetConnect(m_hInternet, m_sHost.c_str(), m_port, 0, 0, INTERNET_SERVICE_HTTP, 
                                      INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE, 0);

  if(!m_hHTTPConnection){
    m_dwError = ::GetLastError();
    return false;
  }

  if(::InternetAttemptConnect(NULL)!=ERROR_SUCCESS){		
    m_dwError = ::GetLastError();
    return false;
  }

  // OK, now we can start request ..
  m_state = StateOpenRequest;
  m_hHTTPRequest=::HttpOpenRequest(m_hHTTPConnection, bPost ? HTTP_VERB_POST : HTTP_VERB_GET, url.c_str(),
                                   HTTP_VERSION, _T(""), &(HTTP_ACCEPT_TYPE),
                                   bPost ? (INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_FORMS_SUBMIT) :
                                   (INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE), 0);

  if(!m_hHTTPRequest){
    m_dwError=::GetLastError();
    return false;
  }

  // REPLACE HEADER
  std::string accept = "Accept: */*\r\n";
  if(FALSE == ::HttpAddRequestHeaders(m_hHTTPRequest, accept.c_str(), accept.size(), HTTP_ADDREQ_FLAG_REPLACE)){
    m_dwError=::GetLastError();
    return false;
  }

  /*
  std::string referer = "Referer: http://cdma.gtss.mot.com/scm/cipt/";
  if(FALSE == ::HttpAddRequestHeaders(m_hHTTPRequest, referer.c_str(), referer.size(), HTTP_ADDREQ_FLAG_REPLACE)){
    m_dwError=::GetLastError();
  }
  */
  if(m_cookie.size()){
    std::stringstream ss;
    ss << "Cookie: " << m_cookie << "\r\n";
    std::string sCookie = ss.str();
    if(!::HttpAddRequestHeaders(m_hHTTPRequest, sCookie.c_str(), sCookie.size(), HTTP_ADDREQ_FLAG_REPLACE)){
      m_dwError=::GetLastError();
      return false;
    }
  }
  return true;
}

bool CHTTPClient::CheckResponse()
{
  ASSERT(m_pResponse == 0);
  const DWORD MAX_SIZE = (1<<18);
  m_pResponse = new char[MAX_SIZE];
  m_length = 0;

  while(m_length < MAX_SIZE){
    DWORD dwRead = 0;
    if(FALSE == ::InternetReadFile(m_hHTTPRequest, m_pResponse + m_length, (MAX_SIZE - m_length), &dwRead)){
      m_dwError = ::GetLastError();
      return false;
    }

    if (dwRead == 0){
      break;
    }

    m_length += dwRead;
  }

  m_pResponse[m_length] = '\0';
  m_headerLength = MAX_HTTP_HEAD_SIZE;
  if(!::HttpQueryInfo(m_hHTTPRequest, HTTP_QUERY_RAW_HEADERS_CRLF, &(m_header[0]), &m_headerLength, NULL)){
    m_dwError=::GetLastError();
    return false;
  }

  DWORD dwSize = m_length;

  // Now, make sure we have read in a full HTML page ... we judge this by checking for </html> at end of buffer ..
  char *pPos = strrchr(m_pResponse, '/');
  if(!pPos){
      return false;
  }

  pPos ++;
  if(strnicmp(pPos, "html>", 5) == 0){
      return true;
  }

  if(strnicmp(pPos, "body>", 5) == 0){
      return true;
  }

  return false;
}

bool CHTTPClient::Redirect(HINTERNET hRequest)
{
    char url[1025];
    memset(url, 0, sizeof(url));
    DWORD length = 1024;
    DWORD index = 0;
    if(FALSE == ::HttpQueryInfo(hRequest, HTTP_QUERY_LOCATION, url, &length, &index)){
        return false;
    }

    return TryGet(url);
}

// 
// Try to retrieve a URL, return true on success, return false on error
//   Try to transform the received HTML to local encoding ... 
bool CHTTPClient::TryGet(const CString &sURL)
{
  //try 10 times ...
  int count = 10;
  while(count){
    Reset();

    if(!TryConnect(std::string(sURL), false)){
      m_dwError = ::GetLastError();
      Sleep(1000);
      count --;
      continue;
    }

    // SEND REQUEST
    m_state = StateSendRequest;
    if(!::HttpSendRequest(m_hHTTPRequest, NULL, 0, NULL, 0)){
      m_dwError = ::GetLastError();
      if(m_dwError == ERROR_HTTP_REDIRECT_NEEDS_CONFIRMATION){
          return Redirect(m_hHTTPRequest);
      }

      Sleep(1000);
      count --;
      continue;
    }

    m_state = StateReadResponse;
    // 
    // now check the response ...
    if(CheckResponse()){
      return true;
    }

    m_dwError = ::GetLastError();
    Sleep(1000);
    count --;
  }

  return false;
}

// 
// TryPost a URL ...
bool CHTTPClient::TryPost(const CString &sUrl, PostArgumentMap &arguments, bool bEncoded)
{
    struct _Quote{
        std::string operator()(std::string s){
          const static char HEX_TABLE[] = "0123456789ABCDEF";
          std::string out;
          out = "";

          for(UINT32 i = 0 ; i < s.size(); i++){
            if( isalpha(s[i]) || isdigit(s[i]) ){
              out += s[i];
            }else{
              out += ('%');
              out += (HEX_TABLE[(s[i] >> 4) & 0x0F]);
              out += (HEX_TABLE[s[i] & 0x0F]);
            }
          }
          
          return out;
        }
    }Quote;

// compose the post arguments first
  std::stringstream ss;

  PostArgumentMap::iterator it = arguments.begin();
  while(true){
    if(it != arguments.end()){
      ss << Quote(it->first) << "=" << Quote(it->second);
    }

    it ++;
    if(it == arguments.end()){
      break;
    }

    ss << "&";
  }

  std::string sArgument = ss.str();
  //sArgument.erase(sArgument.end()); // remove the last '&'

  //std::string cType = "Content-Type: application/x-www-form-urlencoded\r\n";
    std::string cType = "";
    if(bEncoded){
        cType = "Content-Type: application/x-www-form-urlencoded\r\n";
    }

  //try 10 times ...
  int count = 10;
  while(count){
    Reset();

    if(!TryConnect(std::string(sUrl), true)){
      m_dwError = ::GetLastError();
#ifndef NDEBUG
        CString sError;
        sError.Format("Connect Error : %#08x encountered, waiting 1 sec. to retry", m_dwError);
        AfxMessageBox(sError);
#endif
      Sleep(1000);
      count --;
      continue;
    }

    // SEND REQUEST
    if(!::HttpSendRequest( m_hHTTPRequest, cType.c_str(), cType.size(),
                           (LPVOID)sArgument.c_str(), sArgument.size())){
      m_dwError = ::GetLastError();
      if(m_dwError == ERROR_HTTP_REDIRECT_NEEDS_CONFIRMATION){
          return Redirect(m_hHTTPRequest);
      }

#ifndef NDEBUG
      CString sError;
      sError.Format("SendRequest Error : %#08x encountered, waiting 1 sec. to retry", m_dwError);
      AfxMessageBox(sError);
#endif

      Sleep(1000);
      count --;
      continue;
    }

    // 
    // now check the response ...
    if(CheckResponse()){
      return true;
    }

    m_dwError = ::GetLastError();
#ifndef NDEBUG
    CString sError;
    sError.Format("Check response Error : %#08x encountered, waiting 1 sec. to retry", m_dwError);
    AfxMessageBox(sError);
#endif

    Sleep(1000);
    count --;
  }

  return false;
}

std::string RawPostForm(sockaddr_in &addr, std::string &url, std::string &cookie, PostArgumentMap &fields, PostArgumentMap &files, PostArgumentMap &filenames)
{
    SOCKET sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == INVALID_SOCKET){
        ASSERT(0);
        //AfxMessageBox("Cannot create socket to retrieve CIPT comparison result.");
        throw std::string("FAILED: No Resource");
    }

    if(connect(sock,(struct sockaddr*)&addr, sizeof(addr)))
    {
        ASSERT(0);
        closesocket(sock);
        //AfxMessageBox("Cannot connect to CIPT server to retrieve CIPT comparison result.");
        //PostMessage(WM_MY_MESSAGE, MSG_FAILURE);
        throw std::string("FAILED: No Resource");
    }

    std::string sFields = "";
    std::string boundary = "-----CIPTDATA";
    // fields
    {
        for (PostArgumentMap::iterator it = fields.begin(); it != fields.end(); it++)
        {
            std::string name = (*it).first;

            sFields += "--" + boundary + "\r\n"
                "content-disposition: form-data; name=\"" + name + "\"\r\n"
                "\r\n" + it->second + "\r\n";
        }
    }

    // files
    {
        for (PostArgumentMap::iterator it = files.begin(); it != files.end(); it++)
        {
            std::string name = (*it).first;
            std::string filename = (*it).second;

            std::string content_type = "application/octet-stream";
            if(name.find_last_of('.') != name.npos){
                std::string suffix = name.substr(name.find_last_of('.') + 1);
                //make it to upper
                for(int i = 0; i < suffix.size(); i++){
                    if(suffix[i] >= 'a' && suffix[i] <= 'z'){
                        suffix[i] += ('A' - 'a');
                    }
                }

                if(suffix == std::string("htm") ||
                    suffix == std::string("html")){
                        content_type = "text/html";
                }
            }

            std::string ssname = filename;
            if(filenames.find(name) != filenames.end()){
                ssname = filenames[name];
            }

            if(ssname.size() > 128 ) {
                std::string trunkname = "...";
                trunkname += ssname.substr(ssname.size() - 120);
                ssname = trunkname;
            }

            sFields += "--" + boundary + "\r\n"
                "content-disposition: form-data; name=\"" + name + "\"; filename=\"" + ssname + "\"\r\n"
                "content-type: " + content_type + "\r\n"
                "\r\n";

            // read in file
            wchar_t widename[2049]; //maximum name allowed
            std::string fname = "\\\\?\\";
            fname += filename;

            int ret = MultiByteToWideChar(CP_OEMCP, 0, fname.c_str(), fname.size(), widename, 2048);
            if(ret < 0 || ret >=2048){
                throw (std::string("FAILED: Too long file ") + filename);
            }
            widename[ret] = _T('\0');
            HANDLE hHandle = CreateFileW(widename, FILE_READ_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
            if(hHandle == INVALID_HANDLE_VALUE){
                TRACE("Error: %#08x\n", GetLastError());
                throw (std::string("FAILED: Cannot read ") + filename);
            }

            char buf[1024];
            DWORD bytes = 0;
            while(TRUE == ReadFile(hHandle, buf, 1024, &bytes, NULL)){
                if(bytes == 0) {
                    break;
                }

                sFields += std::string(buf, bytes);
            }

            CloseHandle(hHandle);

            sFields += "\r\n";
        }
    }

    // end
    sFields += "--" + boundary + "--\r\n";

    std::string sHeader = "POST ";
    sHeader += url;
    sHeader += " HTTP/1.1\r\n";
    sHeader += "Accept: text/html, text/plain, */*;q=0.01\r\n";
    sHeader += "Connection: Keep-Alive\r\n";
    sHeader += "Host: ciptweb.cig.mot.com:8000\r\n";
    if(!cookie.empty()){
        sHeader += "Cookie: " + cookie + "\r\n";
    }

    sHeader += "Content-type: multipart/form-data; boundary=" + boundary + "\r\n";
    if(1)
    {
        char tmp[100];
        sprintf(tmp,"Content-length: %ld",sFields.size());
        sHeader += tmp;
    }else{
        sHeader += "Content-length: 0";
    }
    sHeader += "\r\n\r\n";

    TRACE("Header: %s\n", sHeader.c_str());

    int ret = send(sock, sHeader.c_str(), sHeader.size(), 0);
    ASSERT(ret == sHeader.size());

    int sent = 0;
    while(sent < sFields.size()){
        int count = 500;
        if((sent + count) > sFields.size()){
            count = sFields.size() - sent;
        }

        ret = send(sock, sFields.c_str() + sent, count, 0);
        if(ret < 0){
            //TRACE("Error: %#08x\n", WSAGetLastError());
            ASSERT(0);
            throw std::string("FAILED: System Failure");
        }

        TRACE("Send: %s\n", sFields.substr(sent, count).c_str());

        ASSERT(ret == count);
        sent += count;
    }

    //now, let's read ...
    // read in htmp response
    char buf[513];
    ret = 0;

    std::string content;
    while((ret = recv(sock, buf, 512, 0)) > 0){
        buf[ret] = '\0';
        content += std::string(buf, ret);
    }

    if(ret < 0){
        TRACE("Error: %#08x\n", WSAGetLastError());
        ASSERT(0);
        throw std::string("FAILED: System Failure");
    }

    return content;
}
