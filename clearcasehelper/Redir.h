//////////////////////////////////////////////////////////////////////
//
// Redirector - to redirect the input / output of a console
//
// Developer: Jeff Lee
// Dec 10, 2001
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REDIR_H__4FB57DC3_29A3_11D5_BB60_006097553C52__INCLUDED_)
#define AFX_REDIR_H__4FB57DC3_29A3_11D5_BB60_006097553C52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CRedirector : public CObject  
{
public:
	CRedirector();
	virtual ~CRedirector();

private:
    static HWND m_hWnd;
    static UINT m_msg;
    static UINT m_param;

	HANDLE m_hThread;		// thread to receive the output of the child process
	HANDLE m_hEvtStop;		// event to notify the redir thread to exit
	DWORD m_dwThreadId;		// id of the redir thread
	DWORD m_dwWaitTime;		// wait time to check the status of the child process

protected:
	HANDLE m_hStdinWrite;	// write end of child's stdin pipe
	HANDLE m_hStdoutRead;	// read end of child's stdout pipe
	HANDLE m_hChildProcess;

	BOOL LaunchChild(LPCTSTR pszCmdLine,
		HANDLE hStdOut, HANDLE hStdIn, HANDLE hStdErr);
	int RedirectStdout();
	void DestroyHandle(HANDLE& rhObject);

	static DWORD WINAPI OutputThread(LPVOID lpvThreadParam);

protected:
	// overrides:
	virtual void WriteStdOut(LPCTSTR pszOutput);
	virtual void WriteStdError(LPCTSTR pszError);

public:
	BOOL Open(LPCTSTR pszCmdLine);
	virtual void Close();
	BOOL Printf(LPCTSTR pszFormat, ...);

	void SetWaitTime(DWORD dwWaitTime) { m_dwWaitTime = dwWaitTime; }
    static void SetCallback(HWND hWnd, UINT msg, UINT param){
        m_hWnd = hWnd;
        m_msg = msg;
        m_param = param;
    }
};

#endif // !defined(AFX_REDIR_H__4FB57DC3_29A3_11D5_BB60_006097553C52__INCLUDED_)
