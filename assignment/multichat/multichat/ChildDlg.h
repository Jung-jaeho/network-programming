#pragma once


// CChildDlg dialog

class CChildDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CChildDlg)

public:
	CChildDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChildDlg();

// Dialog Data
	enum { IDD = IDD_ChilDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
