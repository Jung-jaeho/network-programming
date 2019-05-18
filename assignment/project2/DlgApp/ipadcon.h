pragma once


// ipadcon dialog

class ipadcon : public CDialogEx
{
	DECLARE_DYNAMIC(ipadcon)

public:
	ipadcon(CWnd* pParent = NULL);   // standard constructor
	virtual ~ipadcon();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
