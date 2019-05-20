//처음에 닉네임을 입력하면 멀티케스트에 가입후
//채팅이 이뤄집니다.

#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


//#define MULTICASTIP "235.7.8.1"
#define REMOTEPORT  9000
#define BUFSIZE     512


char * MULTICASTIP;
char name[10];


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

// 클라이언트와 데이터 통신
DWORD WINAPI Receiver(LPVOID arg)
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == INVALID_SOCKET) err_quit("socket()");	

	// SO_REUSEADDR 옵션 설정
	BOOL optval = TRUE;
	retval = setsockopt(sock, SOL_SOCKET,
		SO_REUSEADDR, (char *)&optval, sizeof(optval));
	if(retval == SOCKET_ERROR) err_quit("setsockopt()");

	// bind()
	SOCKADDR_IN localaddr;
	ZeroMemory(&localaddr, sizeof(localaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localaddr.sin_port = htons(REMOTEPORT);
	retval = bind(sock, (SOCKADDR *)&localaddr, sizeof(localaddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");
	
	// 멀티캐스트 그룹 가입
	struct ip_mreq mreq;
	
	mreq.imr_multiaddr.s_addr = inet_addr(MULTICASTIP);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	retval = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(char *)&mreq, sizeof(mreq));
	if(retval == SOCKET_ERROR) err_quit("setsockopt()");

	// 데이터 통신에 사용할 변수
	SOCKADDR_IN peeraddr;
	int addrlen;
	char buf[BUFSIZE+1];
	char name[10];
	// 멀티캐스트 데이터 받기
	while(1){
		// 데이터 받기
		addrlen = sizeof(peeraddr);

		retval = recvfrom(sock, name, 10, 0, 
			(SOCKADDR *)&peeraddr, &addrlen);
		if(retval == SOCKET_ERROR){
			err_display("recvfrom()");
			continue;
		}
		name[retval] = '\0';
		retval = recvfrom(sock, buf, BUFSIZE, 0, 
			(SOCKADDR *)&peeraddr, &addrlen);
		if(retval == SOCKET_ERROR){
			err_display("recvfrom()");
			continue;
		}

		// 받은 데이터 출력
		buf[retval] = '\0';
		//printf("\n[UDP/%s:%d] %s\n", inet_ntoa(peeraddr.sin_addr), 
		//	ntohs(peeraddr.sin_port), buf);
		printf("%s : %s\n",name , buf);
	}

	// 멀티캐스트 그룹 탈퇴
	retval = setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
		(char *)&mreq, sizeof(mreq));
	if(retval == SOCKET_ERROR) err_quit("setsockopt()");

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
void getip()
{
	
    MULTICASTIP=(char *)malloc(sizeof(char)*16);
	scanf("%s", MULTICASTIP);
	printf("start multicast chat! ip: %s\n", MULTICASTIP);
	printf("enter your nickname:");
	getchar();
	gets(name);
}
int main(int argc, char *argv[])
{
	getip();
	int retval;
	
	struct tm *t;
	time_t timer; //time 

	timer = time(NULL);
	t = localtime(&timer);




	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == INVALID_SOCKET) err_quit("socket()");

	// 멀티캐스트 TTL 설정
	int ttl = 2;
	retval = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
		(char *)&ttl, sizeof(ttl));
	if(retval == SOCKET_ERROR) err_quit("setsockopt()");

	// 소켓 주소 구조체 초기화
	SOCKADDR_IN remoteaddr;
	ZeroMemory(&remoteaddr, sizeof(remoteaddr));
	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_addr.s_addr = inet_addr(MULTICASTIP);
	remoteaddr.sin_port = htons(REMOTEPORT);

	// 데이터 통신에 사용할 변수
	char sendbuf[BUFSIZE+1];
	int len;
	HANDLE hThread;

	//리시버 스레드 생성
	hThread = CreateThread(NULL, 0, Receiver,
	(LPVOID)sock, 0, NULL);
	if(hThread == NULL) { closesocket(sock); }
	else { CloseHandle(hThread); }

	// 멀티캐스트 데이터 보내기
	while(1){
		// 데이터 입력
		//printf("\n[보낼 데이터] ");
		if(fgets(sendbuf, BUFSIZE+1, stdin) == NULL)
			break;

		// '\n' 문자 제거
		len = strlen(sendbuf);
		if(sendbuf[len-1] == '\n')
			sendbuf[len-1] = '\0';
		if(strlen(sendbuf) == 0)
			break;

		// 데이터 보내기
		retval = sendto(sock, name, strlen(name), 0,(SOCKADDR *)&remoteaddr, sizeof(remoteaddr));
		if(retval == SOCKET_ERROR){
			err_display("sendto()");
			continue;
		}

		retval = sendto(sock, sendbuf, strlen(sendbuf), 0,
			(SOCKADDR *)&remoteaddr, sizeof(remoteaddr));
		if(retval == SOCKET_ERROR){
			err_display("sendto()");
			continue;
		}
		printf("[UDP] %d바이트를 보냈습니다.\n", retval);
	}

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}