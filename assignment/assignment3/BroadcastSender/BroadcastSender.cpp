// 성명: 정재호
// 학번: 14011078
// 플랫폼: Visual Studio 2010

#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define REMOTEIP   "255.255.255.255"
#define REMOTEPORT 9000
#define BUFSIZE    512

<<<<<<< HEAD


// 소켓 함수 오류 출력 후 종료
=======
// �냼耳� �븿�닔 �삤瑜� 異쒕젰 �썑 醫낅즺
>>>>>>> 1e9a52a7f190b1fa05f52f3d1759098ca1f82fda
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

// �냼耳� �븿�닔 �삤瑜� 異쒕젰
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

int main(int argc, char *argv[])
{
	int retval;

	// �쐢�냽 珥덇린�솕
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == INVALID_SOCKET) err_quit("socket()");

	// 釉뚮줈�뱶罹먯뒪�똿 �솢�꽦�솕
	BOOL bEnable = TRUE;
	retval = setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
		(char *)&bEnable, sizeof(bEnable));
	if(retval == SOCKET_ERROR) err_quit("setsockopt()");

	// �냼耳� 二쇱냼 援ъ“泥� 珥덇린�솕
	SOCKADDR_IN remoteaddr;
	ZeroMemory(&remoteaddr, sizeof(remoteaddr));
	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_addr.s_addr = inet_addr(REMOTEIP);
	remoteaddr.sin_port = htons(REMOTEPORT);

	// �뜲�씠�꽣 �넻�떊�뿉 �궗�슜�븷 蹂��닔
	char buf[BUFSIZE+1];
	int len;

<<<<<<< HEAD
	connect(sock, (SOCKADDR *)&remoteaddr, sizeof(remoteaddr));

	// 브로드캐스트 데이터 보내기
=======

	// 釉뚮줈�뱶罹먯뒪�듃 �뜲�씠�꽣 蹂대궡湲�
>>>>>>> 1e9a52a7f190b1fa05f52f3d1759098ca1f82fda
	while(1){
		// �뜲�씠�꽣 �엯�젰
		printf("\n[蹂대궪 �뜲�씠�꽣] ");
		if(fgets(buf, BUFSIZE+1, stdin) == NULL)
			break;

		// '\n' 臾몄옄 �젣嫄�
		len = strlen(buf);
		if(buf[len-1] == '\n')
			buf[len-1] = '\0';
		if(strlen(buf) == 0)
			break;

		// �뜲�씠�꽣 蹂대궡湲�
		retval = send(sock, buf, strlen(buf),0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
			continue;
		}
		printf("[UDP] %d諛붿씠�듃瑜� 蹂대깉�뒿�땲�떎.\n", retval);
	}

	// closesocket()
	closesocket(sock);

	// �쐢�냽 醫낅즺
	WSACleanup();
	return 0;
}
