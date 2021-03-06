// 2019년 1학기 네트워크프로그래밍 숙제 1번 서버
// 성명: 정재호 학번: 14011078
// 플랫폼: VS2010
// 작동하는 도메인 네임:
//www.naver.com
//www.youtube.com
//www.daum.net


#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

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
// 도메인 이름 -> IPv4 주소
// my modified buf -> name
BOOL GetIPAddr(SOCKET sock, char *name, IN_ADDR *addr,SOCKADDR_IN claddr)
{
	SOCKADDR_IN clientaddr;
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	HOSTENT *ptr = gethostbyname(name);
	char *ip[100];
	char *nickname[100];
	char *standard;
	int i,j; //ip number, nickname number
	int x,y;
	clientaddr=claddr;
	
	if(!strcmp(name,"quit"))
	{
		closesocket(sock);
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
		closesocket(listen_sock);
		WSACleanup();
		exit(1);
	}


	if(ptr == NULL){
		err_display("gethostbyname()");

		/////no DNS
		int len = strlen("====존재 하지 않는 도메인입니다.=====\n");
		// 데이터 보내기(고정 길이)
		int retval = send(sock, (char *)&len, sizeof(int), 0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
		}
		// 데이터 보내기(가변 길이)
		retval = send(sock, "====존재 하지 않는 도메인입니다.=====\n", len, 0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
		}

		/////exit
		len = strlen("다음도메인 입력하기");
		// 데이터 보내기(고정 길이)
		retval = send(sock, (char *)&len, sizeof(int), 0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
		}
		// 데이터 보내기(가변 길이)
		retval = send(sock, "다음도메인 입력하기", len, 0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
		}
		return FALSE;
	}
	if(ptr->h_addrtype != AF_INET)
		return FALSE;
	

	//ip receive
	for(i=0;ptr->h_addr_list[i]!=NULL;i++)
	{
		ip[i]=(char *)malloc(sizeof(char)*50);
		strcpy(ip[i],inet_ntoa(*(struct in_addr*)ptr->h_addr_list[i]));
	}
	


	//nickname
	for(j=0;ptr->h_aliases[j]!=NULL;j++)
	{
		nickname[j]=(char *)malloc(sizeof(char)*50);
		strcpy(nickname[j],ptr->h_aliases[j]);
	}

	//standard name
	standard=(char *)malloc(sizeof(char)*50);
	strcpy(standard,ptr->h_name);

	/////// standard name send
	int len = strlen(standard);
		// 데이터 보내기(고정 길이)
		int retval = send(sock, (char *)&len, sizeof(int), 0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
		}

		// 데이터 보내기(가변 길이)
		retval = send(sock, standard, len, 0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
		}

/////// standard nickname send
	for(y=0;y<j;y++)
	{
			// '\n' 문자 제거
		int len = strlen(name);
		// 데이터 보내기(고정 길이)
		int retval = send(sock, (char *)&len, sizeof(int), 0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
		}

		// 데이터 보내기(가변 길이)
		retval = send(sock, name, len, 0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
		}
	}
	/////// standard ip send
	for(x=0;x<i;x++)
	{
	// '\n' 문자 제거
		int len = strlen(ip[x]);
		// 데이터 보내기(고정 길이)
		int retval = send(sock, (char *)&len, sizeof(int), 0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
		}
		// 데이터 보내기(가변 길이)
		retval = send(sock, &*ip[x], len, 0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
		}
	}
	/////exit
		len = strlen("다음도메인 입력하기");
		// 데이터 보내기(고정 길이)
		retval = send(sock, (char *)&len, sizeof(int), 0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
		}
		// 데이터 보내기(가변 길이)
		retval = send(sock, "다음도메인 입력하기", len, 0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
		}
		
	return TRUE;
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

int main(int argc, char *argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;

	printf("14011078 정재호 네트워크 프로그래밍 숙제1번 서버\n");
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if(retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	IN_ADDR addr;
	int addrlen;
	char buf[BUFSIZE+1];
	int len;

	while(1){
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if(client_sock == INVALID_SOCKET){
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// 클라이언트와 데이터 통신
		while(1){
			// 데이터 받기(고정 길이)
			retval = recvn(client_sock, (char *)&len, sizeof(int), 0);
			if(retval == SOCKET_ERROR){
				err_display("recv()");
				break;
			}
			else if(retval == 0)
				break;
			// 데이터 받기(가변 길이)
			retval = recvn(client_sock, buf, len, 0);
			if(retval == SOCKET_ERROR){
				err_display("recv()");
				break;
			}
			else if(retval == 0)
				break;

			// 받은 데이터 출력
			buf[retval] = '\0';
			printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),
				ntohs(clientaddr.sin_port), buf);
			if(GetIPAddr(client_sock, buf, &addr,clientaddr)){
			// 성공이면 결과 출력
			}
			else
			{
			}

		}

		// closesocket()
		closesocket(client_sock);
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	// closesocket()
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}