#pragma once


// ipinsert dialog

class ipinsert : public CDialogEx
{
	DECLARE_DYNAMIC(ipinsert)

public:
	ipinsert(CWnd* pParent = NULL);   // standard constructor
	virtual ~ipinsert();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:

	char m_staticName;
	afx_msg void OnBnClickedButton1();
};
