// ChildDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ChildDlg.h"
#include "afxdialogex.h"


// CChildDlg dialog

IMPLEMENT_DYNAMIC(CChildDlg, CDialogEx)

CChildDlg::CChildDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CChildDlg::IDD, pParent)
{

}

CChildDlg::~CChildDlg()
{
}

void CChildDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CChildDlg, CDialogEx)
END_MESSAGE_MAP()


// CChildDlg message handlers
