#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    512

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

DWORD WINAPI ProcessInputSend(LPVOID arg);
char userID[10];		// 유저ID

// 데이터 통신에 사용할 변수
char buf[BUFSIZE+1];
int len;

SOCKET sock;

int main(int argc, char *argv[])
{
	int retval;

	printf("Input ID : ");	// 유저ID 입력
	gets(userID);

	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	HANDLE hThread; // 스레드


	// socket()
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("connect()");


	// 서버와 데이터 통신
    // 스레드 생성
		hThread = CreateThread(NULL, 0, ProcessInputSend, NULL, 0, NULL);
		if (hThread == NULL) {
			printf("fail make thread\n");
		}
		else {
			CloseHandle(hThread);
		}

	while(1){
		// 데이터 받기
		retval = recv(sock, buf,  BUFSIZE+1, 0);
		if(retval == SOCKET_ERROR){
			err_display("recv()");
			break;
		}
		else if(retval == 0)
			break;

		// 받은 데이터 출력
		buf[retval] = '\0';
		// printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval);
		printf(" %s\n", buf);
	}

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

/* 사용자 입력 */

DWORD WINAPI ProcessInputSend(LPVOID arg)
{

	int retval;		// 데이터 입력
	char line[BUFSIZE+11];
	while(1) {


		// printf("\n[보낼 데이터] ");
		fgets(buf, BUFSIZE+1, stdin);

		// '\n' 문자 제거
		len = strlen(buf);
		if(buf[len-1] == '\n')
			buf[len-1] = '\0';

		sprintf(line, "[%s] : %s", userID, buf);

		// 데이터 보내기
		retval = send(sock, line, strlen(line), 0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
			return 0;
		}
		// printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);
	}
}