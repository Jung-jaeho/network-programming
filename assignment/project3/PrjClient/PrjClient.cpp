// 2019년 1학기 네트워크프로그래밍 숙제 3번
// 성명: 정재호 학번: 14011078
// 플랫폼: VS2010

#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "resource.h"

#define SERVERIPV4  "127.0.0.1"
#define SERVERIPV6  "::1"
#define SERVERPORT  9000

#define CHATSIZE	256
#define NICKSIZE    96
#define BUFSIZE     sizeof(int) + sizeof(int) + CHATSIZE + NICKSIZE + NICKSIZE // 전송 메시지 전체 크기


#define CHATTING		1000                   // 메시지 타입: 채팅
#define NICK			1002                   // 메시지 타입: 닉네임 설정
#define NICK_FAIL		1003                   // 메시지 타입: 닉네임 설정 실패
#define NICK_SUCCESS	1004                   // 메시지 타입: 닉네임 설정 성공
#define CHANNEL			1006                   // 메시지 타입: 채널 설정
#define CHANNEL_FAIL	1007                   // 메시지 타입: 채널 입장 실패
#define SHOW_NICK		1008                   // 메시지 타입: 전체 닉네임 공개

BOOL trans;

struct CHAT_MSG
{
	int  type;
	int  channel;
	char nick[NICKSIZE];
	char who[NICKSIZE]; 
	char buf[CHATSIZE]; // 메시지
};

static HINSTANCE     g_hInst; // 응용 프로그램 인스턴스 핸들
static HWND          g_hButtonSendMsg; // '메시지 전송' 버튼
static HWND          g_hButtonSendNick;	 // 닉변경 요청

static HWND          g_hButtonCh1Enter;	 // 1번방 입장
static HWND          g_hButtonCh2Enter;	 // 2번방 입장
static HWND          g_hButtonShowNicks; // 사용자 목록 보기

static HWND          g_hEditStatus; // 받은 메시지 출력
static char          g_ipaddr[64]; // 서버 IP 주소
static u_short       g_port; // 서버 포트 번호
static BOOL          g_isIPv4; // IPv4
static HANDLE        g_hClientThread; // 스레드 핸들
static volatile BOOL g_bStart; // 통신 시작 여부
static SOCKET        g_sock; // 클라이언트 소켓
static HANDLE        g_hReadEvent, g_hWriteEvent; // 이벤트 핸들
static CHAT_MSG      g_chatmsg; // 채팅 메시지 저장


// 대화상자 프로시저
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// 소켓 통신 스레드 함수
DWORD WINAPI ClientMain(LPVOID arg);
DWORD WINAPI ReadThread(LPVOID arg);
DWORD WINAPI WriteThread(LPVOID arg);
// 자식 윈도우 프로시저
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
// 편집 컨트롤 출력 함수
void DisplayText(char *fmt, ...);
// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags);
// 오류 출력 함수
void err_quit(char *msg);
void err_display(char *msg);

// 메인 함수
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0) return 1;

	// 이벤트 생성
	g_hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if(g_hReadEvent == NULL) return 1;
	g_hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(g_hWriteEvent == NULL) return 1;

	// 변수 초기화(일부)
	::memset( &g_chatmsg, 0, sizeof(g_chatmsg));
	int i = sizeof(g_chatmsg);
	g_chatmsg.type = CHATTING;

	// 대화상자 생성
	g_hInst = hInstance;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

	// 이벤트 제거
	CloseHandle(g_hReadEvent);
	CloseHandle(g_hWriteEvent);

	// 윈속 종료
	WSACleanup();
	return 0;
}

// 대화상자 프로시저
BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hButtonIsIPv6;
	static HWND hEditIPaddr;
	static HWND hEditPort;
	static HWND hButtonConnect;
	static HWND hEditMsg, hEditNick;
	int flag=0;


	switch(uMsg){
	case WM_INITDIALOG:
		// 컨트롤 핸들 얻기
		hButtonIsIPv6 = GetDlgItem(hDlg, IDC_ISIPV6);
		hEditIPaddr = GetDlgItem(hDlg, IDC_IPADDR);
		hEditPort = GetDlgItem(hDlg, IDC_PORT);
		hButtonConnect = GetDlgItem(hDlg, IDC_CONNECT);
		g_hButtonSendMsg = GetDlgItem(hDlg, IDC_BTN_SENDMSG);
		g_hButtonSendNick = GetDlgItem(hDlg, IDC_BTN_NICK_UPDATE);


		g_hButtonCh1Enter = GetDlgItem(hDlg, IDC_BTN_CHANNEL_1);
		g_hButtonCh2Enter = GetDlgItem(hDlg, IDC_BTN_CHANNEL_2);
		g_hButtonShowNicks = GetDlgItem(hDlg, IDC_BTN_SHOW_NICK);
		
		

		hEditMsg = GetDlgItem(hDlg, IDC_MSG);
		hEditNick = GetDlgItem(hDlg, IDC_TEXT_NICK);

		g_hEditStatus = GetDlgItem(hDlg, IDC_STATUS);

		// 컨트롤 초기화
		SendMessage(hEditMsg, EM_SETLIMITTEXT, CHATSIZE, 0);
		SendMessage(hEditNick, EM_SETLIMITTEXT, NICKSIZE, 0);

		SetDlgItemText(hDlg, IDC_TEXT_NICK, "");
		
		EnableWindow(g_hButtonSendMsg, FALSE);
		EnableWindow(g_hButtonSendNick, FALSE);
		EnableWindow(g_hButtonCh1Enter, FALSE);
		EnableWindow(g_hButtonCh2Enter, FALSE);
		EnableWindow(g_hButtonShowNicks, FALSE);

		SetDlgItemText(hDlg, IDC_IPADDR, SERVERIPV4);
		SetDlgItemInt(hDlg, IDC_PORT, SERVERPORT, FALSE);
	
		// 윈도우 클래스 등록
		WNDCLASS wndclass;
		wndclass.style = CS_HREDRAW|CS_VREDRAW;
		wndclass.lpfnWndProc = WndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = g_hInst;
		wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = "MyWndClass";
		if(!RegisterClass(&wndclass)) return 1;

		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam)){

		case IDC_CONNECT:
			GetDlgItemText(hDlg, IDC_IPADDR, g_ipaddr, sizeof(g_ipaddr));
			if(strcmp(g_ipaddr,"127.0.0.1")!=0)
			{
				MessageBox(NULL,("IP rewrite"), ("IP check"), MB_OK);
				return false;
			}
			trans = FALSE;
			g_port = GetDlgItemInt(hDlg, IDC_PORT, &trans, FALSE);
			//no int 
			if(g_port!=9000)
			{
				 MessageBox(NULL,("PORT rewrite"), ("port check"), MB_OK);
				return false;
			}
			
			// 소켓 통신 스레드 시작
			g_hClientThread = CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);
			if(g_hClientThread == NULL){
				MessageBox(hDlg, "클라이언트를 시작할 수 없습니다."
					"\r\n프로그램을 종료합니다.", "실패!", MB_ICONERROR);
				EndDialog(hDlg, 0);
			}
			else{
				while(g_bStart == FALSE); // 서버 접속 성공 기다림
				{
					EnableWindow(hButtonConnect, FALSE);
					EnableWindow(hButtonIsIPv6, FALSE);
					EnableWindow(hEditIPaddr, FALSE);
					EnableWindow(hEditPort, FALSE);
					EnableWindow(g_hButtonSendMsg, FALSE);
					EnableWindow(g_hButtonSendNick, FALSE);
					EnableWindow(g_hButtonCh1Enter, FALSE);
					EnableWindow(g_hButtonCh2Enter, FALSE);
					EnableWindow(g_hButtonShowNicks, FALSE);
				}
				EnableWindow(hEditIPaddr, TRUE);
				EnableWindow(hEditPort, TRUE);
				EnableWindow(g_hButtonSendMsg, TRUE);
				EnableWindow(g_hButtonSendNick, TRUE);
				EnableWindow(g_hButtonCh1Enter, TRUE);
				EnableWindow(g_hButtonCh2Enter, TRUE);
				EnableWindow(g_hButtonShowNicks, TRUE);

				SetFocus(hEditMsg);
			}
			return TRUE;

		case IDC_BTN_SENDMSG:
			// 읽기 완료를 기다림
			WaitForSingleObject(g_hReadEvent, INFINITE);

			::memset( &g_chatmsg, 0, sizeof(g_chatmsg));
			g_chatmsg.type = CHATTING; // 귓속말
			GetDlgItemText(hDlg, IDC_MSG, g_chatmsg.buf, CHATSIZE);
			// 쓰기 완료를 알림
			SetEvent(g_hWriteEvent);
			// 입력된 텍스트 전체를 선택 표시
			SendMessage(hEditMsg, EM_SETSEL, 0, -1);
			return TRUE;

			
		case IDC_BTN_NICK_UPDATE:
			// 읽기 완료를 기다림
			WaitForSingleObject(g_hReadEvent, INFINITE);
			::memset( &g_chatmsg, 0, sizeof(g_chatmsg));
			g_chatmsg.type = NICK; // 닉네임 변경
			GetDlgItemText(hDlg, IDC_TEXT_NICK, g_chatmsg.nick, NICKSIZE);

			// 쓰기 완료를 알림
			SetEvent(g_hWriteEvent);
			// 입력된 텍스트 전체를 선택 표시
			SendMessage(hEditMsg, EM_SETSEL, 0, -1);
			return TRUE;

		case IDC_BTN_CHANNEL_1:
			// 읽기 완료를 기다림
			WaitForSingleObject(g_hReadEvent, INFINITE);
			::memset( &g_chatmsg, 0, sizeof(g_chatmsg));
			g_chatmsg.type = CHANNEL; // 채널 입장
			g_chatmsg.channel = 1;

			// 쓰기 완료를 알림
			SetEvent(g_hWriteEvent);
			return TRUE;

		case IDC_BTN_CHANNEL_2:
			// 읽기 완료를 기다림
			WaitForSingleObject(g_hReadEvent, INFINITE);

			::memset( &g_chatmsg, 0, sizeof(g_chatmsg));
			g_chatmsg.type = CHANNEL; // 채널 입장
			g_chatmsg.channel = 2;

			// 쓰기 완료를 알림
			SetEvent(g_hWriteEvent);
			return TRUE;

		case IDC_BTN_SHOW_NICK:
			// 읽기 완료를 기다림
			WaitForSingleObject(g_hReadEvent, INFINITE);

			::memset( &g_chatmsg, 0, sizeof(g_chatmsg));
			g_chatmsg.type = SHOW_NICK;

			// 쓰기 완료를 알림
			SetEvent(g_hWriteEvent);
			return TRUE;

		case IDCANCEL:
			if(MessageBox(hDlg, "정말로 종료하시겠습니까?",
				"질문", MB_YESNO|MB_ICONQUESTION) == IDYES)
			{
				closesocket(g_sock);
				EndDialog(hDlg, IDCANCEL);
			}
			return TRUE;

		}
		return FALSE;
	}

	return FALSE;
}

// 소켓 통신 스레드 함수
DWORD WINAPI ClientMain(LPVOID arg)
{
	int retval;

		// socket()
		g_sock = socket(AF_INET, SOCK_STREAM, 0);
		if(g_sock == INVALID_SOCKET) err_quit("socket()");

		// connect()
		SOCKADDR_IN serveraddr;
		ZeroMemory(&serveraddr, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = inet_addr(g_ipaddr);
		serveraddr.sin_port = htons(g_port);
		retval = connect(g_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
		if(retval == SOCKET_ERROR) err_quit("connect()");
	
	MessageBox(NULL, "서버에 접속했습니다.", "성공!", MB_ICONINFORMATION);

	// 읽기 & 쓰기 스레드 생성
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, WriteThread, NULL, 0, NULL);
	if(hThread[0] == NULL || hThread[1] == NULL){
		MessageBox(NULL, "스레드를 시작할 수 없습니다."
			"\r\n프로그램을 종료합니다.",
			"실패!", MB_ICONERROR);
		exit(1);
	}

	g_bStart = TRUE;

	// 스레드 종료 대기
	retval = WaitForMultipleObjects(2, hThread, FALSE, INFINITE);
	retval -= WAIT_OBJECT_0;
	if(retval == 0)
		TerminateThread(hThread[1], 1);
	else
		TerminateThread(hThread[0], 1);
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);

	g_bStart = FALSE;

	MessageBox(NULL, "서버가 접속을 끊었습니다", "알림", MB_ICONINFORMATION);
	EnableWindow(g_hButtonSendMsg, FALSE);
	EnableWindow(g_hButtonSendNick, FALSE);
	EnableWindow(g_hButtonCh1Enter, FALSE);
	EnableWindow(g_hButtonCh2Enter, FALSE);
	EnableWindow(g_hButtonShowNicks, FALSE);

	closesocket(g_sock);
	return 0;
}

// 데이터 받기
DWORD WINAPI ReadThread(LPVOID arg)
{
	int retval;
	CHAT_MSG chat_msg;
	::memset( &chat_msg, 0, sizeof(chat_msg) );

	while(1){
		retval = recvn(g_sock, (char *)&chat_msg, BUFSIZE, 0);
		if(retval == 0 || retval == SOCKET_ERROR){
			break;
		}

		switch (chat_msg.type)
		{
		case CHATTING :
			DisplayText("%s : %s\r\n", chat_msg.nick, chat_msg.buf);
			break;
		case NICK :
		case NICK_FAIL :
		case NICK_SUCCESS :
		case CHANNEL_FAIL :
			DisplayText("[시스템 메시지] %s\r\n", chat_msg.buf);
			break;
		case SHOW_NICK :
			DisplayText("이 방의 닉네임 : %s\r\n", chat_msg.buf);
			break;
		case CHANNEL :
			DisplayText("%s%s\r\n", chat_msg.nick, chat_msg.buf);
			break;					
			
		default:
			break;
		}
	}

	return 0;
}

// 데이터 보내기
DWORD WINAPI WriteThread(LPVOID arg)
{
	int retval;

	// 서버와 데이터 통신
	while(1){
		// 쓰기 완료 기다리기
		WaitForSingleObject(g_hWriteEvent, INFINITE);


		// 데이터 보내기
		retval = send(g_sock, (char *)&g_chatmsg, BUFSIZE, 0);
		if(retval == SOCKET_ERROR){
			break;
		}

		// '메시지 전송' 버튼 활성화
		EnableWindow(g_hButtonSendMsg, TRUE);
		EnableWindow(g_hButtonSendNick, TRUE);
		EnableWindow(g_hButtonCh1Enter, TRUE);
		EnableWindow(g_hButtonCh2Enter, TRUE);
		EnableWindow(g_hButtonShowNicks, TRUE);

		// 읽기 완료 알리기
		SetEvent(g_hReadEvent);
	}

	return 0;
}

// 자식 윈도우 프로시저
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// 에디트 컨트롤에 문자열 출력
void DisplayText(char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	char cbuf[1024];
	vsprintf(cbuf, fmt, arg);

	int nLength = GetWindowTextLength(g_hEditStatus);
	SendMessage(g_hEditStatus, EM_SETSEL, nLength, nLength);
	SendMessage(g_hEditStatus, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

	va_end(arg);
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

// 소켓 함수 오류 출력 후 종료
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

