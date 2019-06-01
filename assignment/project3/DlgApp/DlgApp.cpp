#pragma comment(lib, "ws2_32")
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <windows.h>
#include "resource.h"
#include <string.h>

#define BUFSIZE 512
#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000

int  REMOTEPORT;
char realname[20];

DWORD WINAPI ProcessInputSend(LPVOID arg);
char userID[10];		// 유저ID

// 데이터 통신에 사용할 변수
char buf[BUFSIZE+1];
int len;
SOCKET sock;

HWND hEdit1, hEdit2, ipaddress, portnum, nickname,roomnum; // 편집 컨트롤

// 대화상자 프로시저
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// 편집 컨트롤 출력 함수
void DisplayText(char *fmt, ...);
// checkport 함수
int CHECKPORT(char buf[]);
// checkroom 함수
int CHECKROOM(char buf[]);

//소켓 함수 오류 출력 후 종료
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while(left > 0){
		received = recv(s, ptr, left, flags);
		if(received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if(received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
	// 대화상자 생성
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
	return 0;
}

// 대화상자 프로시저
BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static char buf[BUFSIZE+1];
	int retval;
	HANDLE hThread; // 스레드
	//포트체크 위해서
	int portch=1;
	int portch1=1;
	switch(uMsg){
	case WM_INITDIALOG:
		ipaddress=GetDlgItem(hDlg, IDC_IPADDRESS1);
		portnum = GetDlgItem(hDlg, IDC_EDIT3);
        nickname = GetDlgItem(hDlg, IDC_EDIT4);
		roomnum = GetDlgItem(hDlg, IDC_EDIT5);
		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1);
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2);
		SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
	    case IPfind:
            MessageBox(NULL,("Class  OK"), ("class check"), MB_OK);
		return TRUE;

	   	case Portnum:
            GetDlgItemText(hDlg, IDC_EDIT3, buf, BUFSIZE+1);
			 portch=CHECKPORT(buf);
			 if(portch==0)
			 {
				 MessageBox(NULL,("Port OK"), ("port check"), MB_OK);
			 }
			 else
			 {
				 MessageBox(NULL,("rewrite"), ("port check"), MB_OK);
			 }
		return TRUE;

		case NICKname:
        GetDlgItemText(hDlg, IDC_EDIT4, buf, BUFSIZE+1);
        strcpy(realname,buf);
        MessageBox(NULL,("%s 확인\n",buf), ("nickname setting"), MB_OK);
        return TRUE;

		case Roomnum:
		GetDlgItemText(hDlg, IDC_EDIT5, buf, BUFSIZE+1);
		 portch1=CHECKROOM(buf);
			 if(portch1==0)
			 {
				 MessageBox(NULL,("ROOMNUMBER OK"), ("Roomnum check"), MB_OK);
			 }
			 else
			 {
				 MessageBox(NULL,("rewrite"), ("Roomnum check"), MB_OK);
			 }
		return TRUE;

		case IDOK:
			GetDlgItemText(hDlg, IDC_EDIT1, buf, BUFSIZE+1);
			DisplayText("%s\r\n", buf);
			SetFocus(hEdit1);
			SendMessage(hEdit1, EM_SETSEL, 0, -1);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

// 편집 컨트롤 출력 함수
void DisplayText(char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	char cbuf[BUFSIZE+256];
	vsprintf(cbuf, fmt, arg);

	int nLength = GetWindowTextLength(hEdit2);
	SendMessage(hEdit2, EM_SETSEL, nLength, nLength);
	SendMessage(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

	va_end(arg);
}

int CHECKPORT(char buf[])
{
    int i=0;
    int flag=0;
    int leng=strlen(buf);
    char buf1[20]="";
    strcpy(buf1,buf);

    //real 0 or no real
    if(leng>5)
    {
        return 1;
    }
    if(*buf=='0')
    {
        return 0;
    }
    else
    {
        for(i=0;i<leng;i++)
        {
            if(0<=buf1[i]-'0'&&buf1[i]-'0'<=9)
            {
            }
            else
            return 1;
        }
        if(atoi(buf1)>65535)
        {
            return 1;
        }
        return 0;
    }
}
int CHECKROOM(char buf[])
{
	int i=0;
    int flag=0;
    int leng=strlen(buf);
    char buf1[20]="";
    strcpy(buf1,buf);

	if(*buf=='0')
    {
        return 0;
    }
	else
    {
        for(i=0;i<leng;i++)
        {
            if(0<=buf1[i]-'0'&&buf1[i]-'0'<=9)
            {
            }
            else
            return 1;
        }
        return 0;
    }

}