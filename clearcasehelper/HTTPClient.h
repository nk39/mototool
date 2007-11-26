#ifndef __HTML_CLIENT_H__
#define __HTML_CLIENT_H__

#include <string>
#include <map>
#include <vector>
#include <afxmt.h>
#include <Wininet.h>
#include <Winsock2.h>
#pragma comment(lib, "Wininet")
// 
// This client expect UTF-8 encoding when retrieving HTML pages
//   it will transform the received page to unicode

typedef std::map<std::string, std::string> PostArgumentMap;
class CHTTPClient {
  static const TCHAR * DEFAULT_AGENT_NAME;
  static const TCHAR * HTTP_VERB_GET;
  static const TCHAR * HTTP_VERB_POST;
  static const TCHAR * HTTP_ACCEPT_TYPE;
  static const TCHAR * HTTP_ACCEPT;

protected:
  const static DWORD MAX_WAIT_TIME = 30000; // 30 seconds
  const static DWORD MAX_HTTP_HEAD_SIZE = (1 << 14); // 8k HEADER BUFFER

  enum State{
    StateConnect,
    StateOpenRequest,
    StateSendRequest,
    StateEndRequest,
    StateReadResponse
  } m_state;

  char *m_pResponse;   // response
  DWORD m_length;               // response length
  
  char m_header[MAX_HTTP_HEAD_SIZE];
  DWORD m_headerLength;

  std::string m_cookie;         // cookie string stored ...
  std::string m_sHost;          // host name of the server to contact
  WORD        m_port;           // port

  HINTERNET m_hInternet;        // internet open handle
  HINTERNET m_hHTTPConnection;  // internet connection hadle
  HINTERNET m_hHTTPRequest;     // internet request hadle
  DWORD     m_dwError;          // error code

  // helper functions
  bool TryConnect(const std::string &url, bool bPost = false);
  bool CheckResponse();
  bool Redirect(HINTERNET hRequest);
public:
  CHTTPClient();
  virtual ~CHTTPClient();

  void Reset();                 // reset for next operation
  bool TryGet(const CString &sUrl);
  bool TryPost(const CString &sUrl, PostArgumentMap &arguments, bool bEncoded = true);


  const char *GetContent() {
    return m_pResponse;
  }

  DWORD GetContentLength() {
    return m_length;
  }

  const char *GetHeader() {
    return &m_header[0];
  }

  DWORD GetHeaderLength() {
    return m_headerLength;
  }

  void SetHost(const std::string &sHost){
    m_sHost = sHost;
  }

  const std::string& GetHost() const{
    return m_sHost;
  }

  WORD GetPort() const{
    return m_port;
  }

  void SetPort(WORD port){
    m_port = port;
  }

  void SetCookie(const std::string &cookie){
    m_cookie = cookie;
  }

  const std::string &GetCookie() const{
    return m_cookie;
  }
};


std::string RawPostForm(sockaddr_in &addr, std::string &url, std::string &cookie, PostArgumentMap &args, PostArgumentMap &files, PostArgumentMap &filenames = PostArgumentMap());
#endif
