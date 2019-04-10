#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdio.h>

#define TESTNAME "www.naver.com"

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
BOOL GetIPAddr(char *name, IN_ADDR *addr)
{
	HOSTENT *ptr = gethostbyname(name);
	if(ptr == NULL){
		err_display("gethostbyname()");
		return FALSE;
	}
	if(ptr->h_addrtype != AF_INET)
		return FALSE;
	memcpy(addr, ptr->h_addr, ptr->h_length);
	return TRUE;
}



int main(int argc, char *argv[])
{
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	printf("도메인 이름(변환 전) = %s\n", TESTNAME);

	// 도메인 이름 -> IP 주소
	IN_ADDR addr;
	if(GetIPAddr(TESTNAME, &addr)){
		// 성공이면 결과 출력
		printf("IP 주소(변환 후) = %s\n", inet_ntoa(addr));
	}

	WSACleanup();
	return 0;
}