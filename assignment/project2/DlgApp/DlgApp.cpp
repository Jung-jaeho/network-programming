// 2019년 1학기 네트워크프로그래밍 숙제 2번

// 성명: 정재호 학번: 14011078

// 플랫폼: VS2010 

 

#pragma comment(lib, "ws2_32")
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include "resource.h"

#define BUFSIZE     512

char  MULTICASTIP[20];
int  REMOTEPORT;
char realname[20];
char tmp[10];
DWORD dwThreadId = 1;

//Class D 224.0.0.0 ~ 239.255.255.255
// 대화상자 프로시저
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// 편집 컨트롤 출력 함수
void DisplayText(char *fmt, ...);

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
void idcheck(char recename[],DWORD ReThreadId,char temp1[])
{
   int flag=0;
   flag=strcmp(realname,recename);


   // 받은이름이랑 내닉네임이랑 비교한다
   //1. 1개를 틀었을때 닉네임이 같을조건(0)
   //2. 2개를 틀었을때 닉네임이 같을조건(0)
   //3. 2개를 틀었을때 닉네임이 다를조건(1,-1)
   if(flag==0&& ReThreadId == atoi(temp1))
   {

   }
   else if (flag == 0 && (ReThreadId != atoi(temp1)))
   {
      MessageBox(NULL, ("내닉네임이랑 동일하다."), ("class check"), MB_OK);
	  strcpy(realname, "");
   }
   else if (flag != 0 && ReThreadId != atoi(temp1))
   {
   }

}
// 클라이언트와 데이터 통신

DWORD WINAPI Receiver(LPVOID arg)
{
    int retval;
   DWORD ReThreadId = GetCurrentThreadId();
   char temp[20];
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
    char name[20];
   //char chatperson[10][10];

    // 멀티캐스트 데이터 받기
    while(1){
        time_t timer;
        struct tm *t;
        timer = time(NULL); // 현재 시각을 초 단위로 얻기
        t = localtime(&timer);
      
      // 데이터 받기
        addrlen = sizeof(peeraddr);

      retval = recvfrom(sock, temp, 20, 0, (SOCKADDR *)&peeraddr, &addrlen);
        if(retval == SOCKET_ERROR){
            err_display("recvfrom()");
            continue;
        }
      temp[retval] = '\0';

        retval = recvfrom(sock, name, 20, 0, (SOCKADDR *)&peeraddr, &addrlen);
      idcheck(name,ReThreadId,temp);
        if(retval == SOCKET_ERROR){
            err_display("recvfrom()");
            continue;
        }
        name[retval+1] = '\0';
        retval = recvfrom(sock, buf, BUFSIZE, 0, (SOCKADDR *)&peeraddr, &addrlen);
        if(retval == SOCKET_ERROR){
            err_display("recvfrom()");
            continue;
        }

        // 받은 데이터 출력
      
        buf[retval] = '\0';
        DisplayText("%s [%s] (%d:%d:%d) : %s\r\n",name,inet_ntoa(peeraddr.sin_addr),t->tm_hour,t->tm_min,t->tm_sec, buf);
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



HWND hEdit1, hEdit2, ipaddress, portnum, nickname; // 편집 컨트롤


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    // 대화상자 생성
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
    return 0;
}
int CHECKIP(char buf[])
{
    //Class D 224.0.0.0 ~ 239.255.2555.255
  char buf1[4]="";
  int i=0;
 
  for(i=0;i<3;i++)
  {
     buf1[i]=buf[i];
  }
  if(atoi(buf) >=224 && atoi(buf) <= 239 )
  {
      return 1;
  }
  else
  {
      return 0;
  }
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
// 대화상자 프로시저
int retval;
SOCKET sock;
SOCKADDR_IN remoteaddr;

// 데이터 통신에 사용할 변수
char sendbuf[BUFSIZE+1];
int len;
int ttl = 2;
HANDLE hThread;

BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   char buf[BUFSIZE+1]={'\0'};
    int check=0;
    int portch=1;
    int flag=0;
   
    switch(uMsg){
    case WM_INITDIALOG: 
        hEdit1 = GetDlgItem(hDlg, IDC_EDIT1);
        hEdit2 = GetDlgItem(hDlg, IDC_EDIT2);
        ipaddress=GetDlgItem(hDlg, IDC_IPADDRESS1);
        portnum = GetDlgItem(hDlg, IDC_EDIT3);
        nickname = GetDlgItem(hDlg, IDC_EDIT4);



        return TRUE;
    
    case WM_COMMAND:
        switch(LOWORD(wParam)){
        case IPfind:
            GetDlgItemText(hDlg, IDC_IPADDRESS1, buf, BUFSIZE+1);
         strcpy(MULTICASTIP,buf);
         MULTICASTIP[strlen(buf)]='\0';
            //reinput code 
            check=CHECKIP(buf);
            if(check==1){
            MessageBox(NULL,("Class D OK"), ("class check"), MB_OK);
         }
            else
            MessageBox(NULL,("Class D NO"), ("class check"), MB_OK);
         return TRUE;

        case Portnum:
            GetDlgItemText(hDlg, IDC_EDIT3, buf, BUFSIZE+1);
            portch=CHECKPORT(buf);
         REMOTEPORT=atoi(buf);
            if(portch==0)
            {
            MessageBox(NULL,("Port OK"), ("class check"), MB_OK);
                 // 윈속 초기화
        WSADATA wsa;
        if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
            return 1;
         // socket()
         sock = socket(AF_INET, SOCK_DGRAM, 0);
         if(sock == INVALID_SOCKET) err_quit("socket()");

         // 멀티캐스트 TTL 설정
         retval = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
            (char *)&ttl, sizeof(ttl));
         if(retval == SOCKET_ERROR
            ) err_quit("setsockopt()");
         // 소켓 주소 구조체 초기화
         ZeroMemory(&remoteaddr, sizeof(remoteaddr));
         remoteaddr.sin_family = AF_INET;
         remoteaddr.sin_addr.s_addr = inet_addr(MULTICASTIP);
         remoteaddr.sin_port = htons(REMOTEPORT);

         //리시버 스레드 생성
         hThread = CreateThread(NULL, 0, Receiver,
         (LPVOID)sock, 0, &dwThreadId);
         if(hThread == NULL) { closesocket(sock); }
         else { CloseHandle(hThread); }
            }
         itoa(dwThreadId,tmp,10);
            if(portch==1)
            {
            MessageBox(NULL,("rewrite"), ("port check"), MB_OK);
         }
         return TRUE;
        case NICKname:
            GetDlgItemText(hDlg, IDC_EDIT4, buf, BUFSIZE+1);
            strcpy(realname,buf);
            MessageBox(NULL,("%s 확인\n",buf), ("nickname setting"), MB_OK);
         return TRUE;
        case IDOK:
            GetDlgItemText(hDlg, IDC_EDIT1, sendbuf, BUFSIZE+1);
            // 데이터 보내기
            retval = sendto(sock, tmp, strlen(tmp), 0,
                (SOCKADDR *)&remoteaddr, sizeof(remoteaddr));
            if(retval == SOCKET_ERROR){
                err_display("sendto()");
            }
         
         retval = sendto(sock, realname, strlen("realname"), 0,(SOCKADDR *)&remoteaddr, sizeof(remoteaddr));
            if(retval == SOCKET_ERROR){
                err_display("sendto()");
            }

            retval = sendto(sock, sendbuf, strlen(sendbuf), 0,
                (SOCKADDR *)&remoteaddr, sizeof(remoteaddr));
            if(retval == SOCKET_ERROR){
                err_display("sendto()");
            }
   
            //DisplayText("%s\r\n", buf);
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