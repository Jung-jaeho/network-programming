#pragma once


// ipaddress dialog

class ipaddress : public CDialogEx
{
	DECLARE_DYNAMIC(ipaddress)

public:
	ipaddress(CWnd* pParent = NULL);   // standard constructor
	virtual ~ipaddress();

// Dialog Data
	enum { IDD = IDD_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnIpnFieldchangedIpaddress1(NMHDR *pNMHDR, LRESULT *pResult);
};
