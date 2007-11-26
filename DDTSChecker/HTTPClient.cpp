#include "stdafx.h"
#include "HTTPClient.h"
#include <sstream>

////////////////////////////////////////////////////////////////////////////////
// Base64 encoding
////////////////////////////////////////////////////////////////////////////////

class CBase64Coding
{
private:
    // Don't allow canonical behavior (i.e. don't allow this class
    // to be passed by value)
    CBase64Coding( const CBase64Coding& ) {};
    CBase64Coding& operator=( const CBase64Coding& ) { return( *this ); };
public:
    // Construction
    CBase64Coding();
    /*
    ** Destructor should be virtual according to MSJ article in Sept 1992
    ** "Do More with Less Code:..."
    */
    virtual ~CBase64Coding();
    static bool Encode(const std::string& source, std::string& destination );
};

const static unsigned char CARRIAGE_RETURN = 13;
const static unsigned char LINE_FEED       = 10;
const static unsigned char END_OF_BASE64_ENCODED_DATA = '=';
const static unsigned char BASE64_END_OF_BUFFER       = 0xFD;
const static unsigned char BASE64_IGNORABLE_CHARACTER = 0xFE;
const static unsigned char BASE64_UNKNOWN_VALUE       = 0xFF;
const static unsigned char BASE64_NUMBER_OF_CHARACTERS_PER_LINE = 72;

typedef unsigned char BYTE;

static inline BYTE __get_character( const BYTE * buffer, const BYTE * decoder_table, int& index, int size_of_buffer )
{
    BYTE return_value = 0;

    do{
        if ( index >= size_of_buffer ){
            return( BASE64_END_OF_BUFFER );
        }

        return_value = buffer[ index ];
        index++;
    }
    while( return_value != END_OF_BASE64_ENCODED_DATA &&
           decoder_table[ return_value ] == BASE64_IGNORABLE_CHARACTER );

    return( return_value );
}

CBase64Coding::CBase64Coding()
{
}

CBase64Coding::~CBase64Coding()
{
}


bool CBase64Coding::Encode(const std::string &source, std::string &destination)
{
    const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    int loop_index                = 0;
    int number_of_bytes_to_encode = source.size();

    BYTE byte_to_add = 0;
    BYTE byte_1      = 0;
    BYTE byte_2      = 0;
    BYTE byte_3      = 0;

    // Now add in the CR/LF pairs, each line is truncated at 72 characters
    destination.clear();

    UINT number_of_bytes_encoded = 0;

    while( loop_index < number_of_bytes_to_encode ){
        // Output the first byte
        byte_1 = source[ loop_index ];
        byte_to_add = alphabet[ ( byte_1 >> 2 ) ];

        destination.push_back(byte_to_add);
        number_of_bytes_encoded++;

        loop_index++;

        if ( loop_index >= number_of_bytes_to_encode ){
            // We're at the end of the data to encode

            byte_2 = 0;
            byte_to_add = alphabet[ ( ( ( byte_1 & 0x03 ) << 4 ) | ( ( byte_2 & 0xF0 ) >> 4 ) ) ];

            destination.push_back(byte_to_add);
            number_of_bytes_encoded++;

            destination.push_back(END_OF_BASE64_ENCODED_DATA);
            number_of_bytes_encoded++;

            destination.push_back(END_OF_BASE64_ENCODED_DATA);

            // 1999-09-01
            // Thanks go to Yurong Lin (ylin@dial.pipex.com) for finding a bug here.
            // We must NULL terminate the string before letting CString have the buffer back.

            //destination.push_back('\0');

            return true;
        }else{
            byte_2 = source[ loop_index ];
        }

        byte_to_add = alphabet[ ( ( ( byte_1 & 0x03 ) << 4 ) | ( ( byte_2 & 0xF0 ) >> 4 ) ) ];

        destination.push_back(byte_to_add);
        number_of_bytes_encoded++;

        loop_index++;

        if ( loop_index >= number_of_bytes_to_encode ){
            // We ran out of bytes, we need to add the last half of byte_2 and pad
            byte_3 = 0;

            byte_to_add = alphabet[ ( ( ( byte_2 & 0x0F ) << 2 ) | ( ( byte_3 & 0xC0 ) >> 6 ) ) ];

            destination.push_back(byte_to_add);
            number_of_bytes_encoded++;

            destination.push_back(END_OF_BASE64_ENCODED_DATA);

            // 1999-09-01
            // Thanks go to Yurong Lin (ylin@dial.pipex.com) for finding a bug here.
            // We must NULL terminate the string before letting CString have the buffer back.

            // destination.push_back('\0');

            return true;
        }else{
            byte_3 = source[ loop_index ];
        }

        loop_index++;

        byte_to_add = alphabet[ ( ( ( byte_2 & 0x0F ) << 2 ) | ( ( byte_3 & 0xC0 ) >> 6 ) ) ];


        destination.push_back(byte_to_add);
        number_of_bytes_encoded++;

        byte_to_add = alphabet[ ( byte_3 & 0x3F ) ];

        destination.push_back(byte_to_add);
        number_of_bytes_encoded++;

        if ( ( number_of_bytes_encoded % BASE64_NUMBER_OF_CHARACTERS_PER_LINE ) == 0 ){
            destination.push_back(CARRIAGE_RETURN);
            number_of_bytes_encoded++;

            destination.push_back(LINE_FEED);
            number_of_bytes_encoded++;
        }
    }

    destination.push_back(END_OF_BASE64_ENCODED_DATA);

    // 1999-09-01
    // Thanks go to Yurong Lin (ylin@dial.pipex.com) for finding a bug here.
    // We must NULL terminate the string before letting CString have the buffer back.
    // destination.push_back('\0');

    return true;
}

const char * CHTTPClient::DEFAULT_AGENT_NAME = "NETLIFE(0.9/;p)";
const char * CHTTPClient::HTTP_VERB_GET = "GET";
const char * CHTTPClient::HTTP_VERB_POST = "POST";
const char * CHTTPClient::HTTP_ACCEPT_TYPE = "*/*";
const char * CHTTPClient::HTTP_ACCEPT = "Accept: */*\r\n";

CHTTPClient::CHTTPClient()
{
  m_hInternet = 0;
  m_hHTTPConnection = 0;
  m_hHTTPRequest = 0;
  m_dwError = 0;

  m_sHost = "";
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
    TRACE("Exception omitted.\n");
  }

  m_dwError = 0;
}

bool CHTTPClient::TryConnect(const std::string &url, bool bPost)
{
  m_hInternet=::InternetOpen(DEFAULT_AGENT_NAME, INTERNET_OPEN_TYPE_PRECONFIG, "", "", 0);

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
                                   HTTP_VERSION, "", &(HTTP_ACCEPT_TYPE),
                                   bPost ? (INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_FORMS_SUBMIT) :
                                   (INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE), 0);

  if(!m_hHTTPRequest){
    m_dwError=::GetLastError();
    return false;
  }

  // REPLACE HEADER
  std::string headers = "Accept: */*\r\nConnection:Keep-Alive\r\nHost:";
  headers += m_sHost;
  headers += "\r\nUser-Agent:HTTPClient/1.0\r\n";

  if(m_cookie.size()){
    std::stringstream ss;
    ss << "Cookie: " << m_cookie << "\r\n";
    headers += ss.str();
  }

  if(m_user.size()){
      std::stringstream ss;
      ss << m_user << ":" << m_password;

      std::string out;
      CBase64Coding::Encode(ss.str(), out);

      headers +="Authorization:Basic ";
      headers += out;
      headers += "\r\n";
  }

  if(FALSE == ::HttpAddRequestHeaders(m_hHTTPRequest, headers.c_str(), headers.size(), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE)){
    m_dwError=::GetLastError();
    return false;
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
  while(dwSize && m_pResponse[dwSize] != '<'){
    dwSize --;
  }

  dwSize ++;                // omit '<'
  dwSize ++;                // omit '</'

  if(!strnicmp(m_pResponse + dwSize, "html>", 5)){
      return true;
  }
  /*
  if((m_pResponse[dwSize] == 'h') && 
                       (m_pResponse[dwSize + 1] == 't') &&
                       (m_pResponse[dwSize + 2] == 'm') && 
                       (m_pResponse[dwSize + 3] == 'l') &&
                       (m_pResponse[dwSize + 4] == '>')){
    return true;
  }
 */

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


