// IP.cpp : implementation file
//

#include "stdafx.h"
#include "IP.h"
#include "afxdialogex.h"


// IP dialog

IMPLEMENT_DYNAMIC(IP, CDialogEx)

IP::IP(CWnd* pParent /*=NULL*/)
	: CDialogEx(IP::IDD, pParent)
{

}

IP::~IP()
{
}

void IP::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(IP, CDialogEx)
END_MESSAGE_MAP()


// IP message handlers
