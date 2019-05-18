// ipinsert.cpp : implementation file
//

#include "stdafx.h"
#include "ipinsert.h"
#include "afxdialogex.h"


// ipinsert dialog

IMPLEMENT_DYNAMIC(ipinsert, CDialogEx)

ipinsert::ipinsert(CWnd* pParent /*=NULL*/)
	: CDialogEx(ipinsert::IDD, pParent)
	, multiip(20)
	, m_staticName(20)
{

}

ipinsert::~ipinsert()
{
}

void ipinsert::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ipinsert, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &ipinsert::OnBnClickedButton1)
END_MESSAGE_MAP()


// ipinsert message handlers


void ipinsert::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
}
