#if !defined(AFX_COLORSDLG_H__4D548003_FEC9_46B4_A0CC_7D3BC64267FD__INCLUDED_)
#define AFX_COLORSDLG_H__4D548003_FEC9_46B4_A0CC_7D3BC64267FD__INCLUDED_

#define MAXPOINTS 20
#define PRECISION 1e-7

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColorsDlg dialog

class CColorsDlg : public CDialog
{
    // Construction
public:
    CColorsDlg(CWnd* pParent = NULL);   // standard constructor

public:
    double colpoints[MAXPOINTS];
    int pointnum;
    COLORREF refpoints[MAXPOINTS];

    // Dialog Data
        //{{AFX_DATA(CColorsDlg)
    enum { IDD = IDD_COLORS };
    CStatic	m_ColorBox;
    double	m_val;
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CColorsDlg)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CColorsDlg)
    afx_msg void OnAdd();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORSDLG_H__4D548003_FEC9_46B4_A0CC_7D3BC64267FD__INCLUDED_)
