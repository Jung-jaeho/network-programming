// 2019년 1학기 네트워크프로그래밍 숙제 3번
// 성명: 정재호 학번: 14011078
// 플랫폼: VS2010

#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
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

struct CHAT_MSG
{
	int  type;
	int  channel;
	char nick[NICKSIZE];
	char who[NICKSIZE]; // 귓말 대상
	char buf[CHATSIZE]; // F메시지
};


// 소켓 정보 저장을 위한 구조체와 변수
struct SOCKETINFO
{
	int	   channel;
	SOCKET sock;
	bool   isIPv4;
	char   buf[BUFSIZE];
	int    recvbytes;
	char   nick[NICKSIZE];
};

int nTotalSockets = 0;
SOCKETINFO *SocketInfoArray[FD_SETSIZE];

BOOL SetNick(SOCKET sock, char* nick, int channel); // 이름 설정, 중복이면 FALSE
void SendMsg(SOCKET sock, char* chat, int type); // 메시지 전송
void BroadCast(int channel, CHAT_MSG* msg); // 전체 메시지 
void BroadCast(int channel, char* chat, int msgType); // 전체 메시지 

// 소켓 관리 함수
BOOL AddSocketInfo(SOCKET sock, bool isIPv4);
void RemoveSocketInfo(int nIndex);

// 오류 출력 함수
void err_quit(char *msg);
void err_display(char *msg);

int main(int argc, char *argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0) return 1;

	/*----- IPv4 소켓 초기화 시작 -----*/
	// socket()
	SOCKET listen_sockv4 = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sockv4 == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddrv4;
	ZeroMemory(&serveraddrv4, sizeof(serveraddrv4));
	serveraddrv4.sin_family = AF_INET;
	serveraddrv4.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddrv4.sin_port = htons(SERVERPORT);
	retval = bind(listen_sockv4, (SOCKADDR *)&serveraddrv4, sizeof(serveraddrv4));
	if(retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sockv4, SOMAXCONN);
	if(retval == SOCKET_ERROR) err_quit("listen()");
	/*----- IPv4 소켓 초기화 끝 -----*/


	// 데이터 통신에 사용할 변수(공통)
	FD_SET rset;
	SOCKET client_sock;
	int addrlen, i, j;
	// 데이터 통신에 사용할 변수(IPv4)
	SOCKADDR_IN clientaddrv4;

	while(1){
		// 소켓 셋 초기화
		FD_ZERO(&rset);
		FD_SET(listen_sockv4, &rset);
		for(i=0; i<nTotalSockets; i++){
			FD_SET(SocketInfoArray[i]->sock, &rset);
		}

		// select()
		retval = select(0, &rset, NULL, NULL, NULL);
		if(retval == SOCKET_ERROR){
			err_display("select()");
			break;
		}

		// 소켓 셋 검사(1): 클라이언트 접속 수용
		if(FD_ISSET(listen_sockv4, &rset)){
			addrlen = sizeof(clientaddrv4);
			client_sock = accept(listen_sockv4, (SOCKADDR *)&clientaddrv4, &addrlen);
			if(client_sock == INVALID_SOCKET){
				err_display("accept()");
				break;
			}
			else{
				// 접속한 클라이언트 정보 출력
				printf("[TCPv4 서버] 클라이언트 접속: [%s]:%d\n",
					inet_ntoa(clientaddrv4.sin_addr), ntohs(clientaddrv4.sin_port));

				// 소켓 정보 추가
				AddSocketInfo(client_sock, false);
			}
		}

		// 소켓 셋 검사(2): 데이터 통신
		for(i=0; i<nTotalSockets; i++){
			SOCKETINFO *ptr = SocketInfoArray[i];
			if(FD_ISSET(ptr->sock, &rset)){
				// 데이터 받기
				retval = recv(ptr->sock, ptr->buf + ptr->recvbytes,
					BUFSIZE - ptr->recvbytes, 0);
				if(retval == 0 || retval == SOCKET_ERROR){
					RemoveSocketInfo(i);
					continue;
				}

				// 받은 바이트 수 누적
				ptr->recvbytes += retval;

				if(ptr->recvbytes == BUFSIZE){
					ptr->recvbytes = 0;
					CHAT_MSG* msg = (CHAT_MSG*)&ptr->buf;

					// 닉네임 변경
					if( msg->type == NICK )
					{
						if( TRUE == SetNick(ptr->sock, msg->nick, ptr->channel) )
						{
							SendMsg(ptr->sock, "닉네임 변경에 성공 했습니다.", NICK_SUCCESS);
						}
						else
						{
							SendMsg(ptr->sock, "닉네임 변경에 실패 했습니다.", NICK_FAIL);
						}
						continue;
					}
					else if( msg->type == CHANNEL ) // 채널 설정
					{
						if( ptr->channel != msg->channel )
						{
							if( TRUE == SetNick(ptr->sock, ptr->nick, msg->channel) )
							{
								ptr->channel = msg->channel;
								::memcpy( msg->buf, "님이 입장 했습니다.", sizeof(msg->buf) ); // 메시지 내용
								::memcpy( msg->nick, ptr->nick, sizeof(msg->nick) ); // 메시지 내용
								BroadCast(ptr->channel, msg);
							}
							else
							{
								SendMsg(ptr->sock, "닉네임을 변경해 주세요.", CHANNEL_FAIL);
							}
						}
						else
						{
							SendMsg(ptr->sock, "이미 입장한 채널 입니다.", CHANNEL_FAIL);
						}
						continue;
					}
					else if( msg->type == SHOW_NICK ) // 채널 설정
					{
						for(int j=0; j<nTotalSockets; j++){
							SOCKETINFO *ptr2 = SocketInfoArray[j];
								SendMsg(ptr->sock, ptr2->nick, SHOW_NICK);
						}
						continue;
					}
					else if( msg->type == CHATTING )
					{
						if( ptr->channel == 0 )
						{
							SendMsg(ptr->sock, "닉네임을 정한 뒤 채팅 방에 입장해야 대화 가능 합니다.", CHANNEL_FAIL);
							continue;
						}
					}
					// 현재 접속한 모든 클라이언트에게 데이터를 보냄!
					::memcpy( msg->nick, ptr->nick, sizeof(msg->nick) ); // 메시지 내용
					BroadCast( ptr->channel, msg );
				}
			}
		}
	}

	return 0;
}

BOOL SetNick(SOCKET sock, char* nick, int channel)
{
	char empty[NICKSIZE];
	memset( empty, 0, sizeof(empty) );
	if( 0 == ::strcmp(empty, nick) )
		return FALSE;

	if( nick[0] == '\0' )
		return FALSE;

	char* user;
	for(int i=0; i<nTotalSockets; i++)
	{
		if(0 == ::strcmp(SocketInfoArray[i]->nick, nick) &&  // 닉네임 비교
			channel == SocketInfoArray[i]->channel && 
			sock != SocketInfoArray[i]->sock ) 
			return FALSE;
	}

	for(int i=0; i<nTotalSockets; i++)
	{
		if(SocketInfoArray[i]->sock == sock)
		{
			::memccpy( SocketInfoArray[i]->nick, nick, 0, NICKSIZE );
			return TRUE;
		}
	}

	return FALSE;
}

void SendMsg(SOCKET sock, char* chat, int msgType) // 메시지 전송
{
	CHAT_MSG msg;
	::memset(&msg, 0, sizeof(msg));

	msg.type = msgType;
	::memcpy( msg.buf, chat, sizeof(msg.buf) ); // 메시지 내용

	int retval = send(sock, (char*)&msg, BUFSIZE, 0);
	if(retval == SOCKET_ERROR){
		err_display("send()");
	}
}

void BroadCast(int channel, char* chat, int msgType)
{
	CHAT_MSG msg;
	::memset(&msg, 0, sizeof(msg));
	msg.type = msgType;
	::memcpy( msg.buf, chat, sizeof(msg.buf) ); // 메시지 내용

	for(int j=0; j<nTotalSockets; j++){
		SOCKETINFO *ptr2 = SocketInfoArray[j];

		// 같은 채널에만 메시지를 보낸다.
		if( ptr2->channel == channel )
		{
			int retval = send(ptr2->sock, (char*)&msg, BUFSIZE, 0);
			if(retval == SOCKET_ERROR){
				err_display("send()");
				RemoveSocketInfo(j);
			}
		}
	}
}

void BroadCast(int channel, CHAT_MSG* msg)
{
	for(int j=0; j<nTotalSockets; j++){
		SOCKETINFO *ptr2 = SocketInfoArray[j];

		// 같은 채널에만 메시지를 보낸다.
		if( ptr2->channel == channel )
		{
			int retval = send(ptr2->sock, (char*)msg, BUFSIZE, 0);
			if(retval == SOCKET_ERROR){
				err_display("send()");
				RemoveSocketInfo(j);
			}
		}
	}
}

// 소켓 정보 추가
BOOL AddSocketInfo(SOCKET sock, bool isIPv4)
{
	if(nTotalSockets >= FD_SETSIZE){
		printf("[오류] 소켓 정보를 추가할 수 없습니다!\n");
		return FALSE;
	}

	SOCKETINFO *ptr = new SOCKETINFO;
	if(ptr == NULL){
		printf("[오류] 메모리가 부족합니다!\n");
		return FALSE;
	}

	ptr->channel = 0;
	ptr->sock = sock;
	ptr->isIPv4 = isIPv4;
	ptr->recvbytes = 0;
	SocketInfoArray[nTotalSockets++] = ptr;
	memset( &ptr->nick, 0, sizeof(ptr->nick) );
	return TRUE;
}

// 소켓 정보 삭제
void RemoveSocketInfo(int nIndex)
{
	SOCKETINFO *ptr = SocketInfoArray[nIndex];

	// 종료한 클라이언트 정보 출력
	
		SOCKADDR_IN clientaddrv4;
		int addrlen = sizeof(clientaddrv4);
		getpeername(ptr->sock, (SOCKADDR *)&clientaddrv4, &addrlen);
		printf("[TCPv4 서버] 클라이언트 종료: [%s]:%d\n", 
			inet_ntoa(clientaddrv4.sin_addr), ntohs(clientaddrv4.sin_port));

	closesocket(ptr->sock);
	delete ptr;

	if(nIndex != (nTotalSockets-1))
		SocketInfoArray[nIndex] = SocketInfoArray[nTotalSockets-1];

	--nTotalSockets;
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