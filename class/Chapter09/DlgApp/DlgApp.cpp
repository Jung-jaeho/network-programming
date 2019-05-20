#include <windows.h>
#include <stdio.h>
#include "resource.h"

#define BUFSIZE 25
//Class D 224.0.0.0 ~ 239.255.2555.255
// 대화상자 프로시저
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// 편집 컨트롤 출력 함수
void DisplayText(char *fmt, ...);

HWND hEdit1, hEdit2, ipaddress, portnum, nickname; // 편집 컨트롤
char realname[20];

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
	char buf1[6]="";
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
		return 0;
	}
}
// 대화상자 프로시저
BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static char buf[BUFSIZE+1];
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
			//reinput code 
			check=CHECKIP(buf);
			if(check==1)
			MessageBox(NULL,("Class D OK"), ("class check"), MB_OK);
			else
			MessageBox(NULL,("Class D NO"), ("class check"), MB_OK);

		case Portnum:
			while(portch)
		{
			GetDlgItemText(hDlg, IDC_EDIT3, buf, BUFSIZE+1);
			portch=CHECKPORT(buf);
			if(portch==0)
			{
			MessageBox(NULL,("Port OK"), ("class check"), MB_OK);
			break;
			}
			if(portch==1)
			{
			MessageBox(NULL,("rewrite"), ("port check"), MB_OK);
			break;
			}
		}
		case NICKname:
			GetDlgItemText(hDlg, IDC_EDIT4, buf, BUFSIZE+1);
			strcpy(realname,buf);
			MessageBox(NULL,("%s",buf), ("nickname setting"), MB_OK);
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