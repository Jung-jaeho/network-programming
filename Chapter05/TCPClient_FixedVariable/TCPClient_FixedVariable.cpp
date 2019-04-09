#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    50

typedef struct Person{   // 구조체 이름은 _Person
    char name[20];            // 공식명칭
    char Nickname[20];            // 별명
    char address[100];        // ipv4 address
}Person;                  // typedef를 사용하여 구조체 별칭을 Person으로 정의
 
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
//typedef receive

Person recv_Person( SOCKET sock ) {
int retval;
int len;
 
retval = recvn( sock, (char *)&len, sizeof( int ), 0 ); // 데이터 받기(고정 길이)
if ( retval == SOCKET_ERROR ) {
    err_display( "recv()" );
}
 
int GetSize;
char suBuffer[BUFSIZE];
Person *player;
GetSize = recv( sock, suBuffer, len, 0 );
if ( GetSize == SOCKET_ERROR ) {
    exit( 1 );
}
 
suBuffer[GetSize] = '\0';
player = (Person*)suBuffer;
 
return *player;
}

int main(int argc, char *argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("connect()");


	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE];
	char testdata[100]="";
	int len;
	Person a;
	

	// 서버와 데이터 통신
	for(int i=0; i<100; i++){
		scanf("%s",testdata);
		if(!strcmp(testdata,"quit")){
			retval = send(sock, (char *)&len, sizeof(int), 0);
			break;
		}
		else{
		// 데이터 입력(시뮬레이션)
		len = strlen(testdata);
		strncpy(buf, testdata, len);

		// 데이터 보내기(고정 길이)
		retval = send(sock, (char *)&len, sizeof(int), 0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
			break;
		}

		// 데이터 보내기(가변 길이)
		retval = send(sock, buf, len, 0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
			break;
		}
		printf("[TCP 클라이언트] %d+%d바이트를 "
			"보냈습니다.\n", sizeof(int), retval);


	}
		//recev
		a=recv_Person(sock);
	}

	


	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}