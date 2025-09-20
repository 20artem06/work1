#if !defined(AFX_AVIDLG_H__50472CE8_6C8D_4D54_8435_DD65BDB83F17__INCLUDED_)
#define AFX_AVIDLG_H__50472CE8_6C8D_4D54_8435_DD65BDB83F17__INCLUDED_



#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AviDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAviDlg dialog

class CChild_2_Frame;

class CAviDlg : public CDialog
{
    // Construction
public:
    CAviDlg(CWnd* pParent = NULL);   // standard constructor

    CChild_2_Frame* pGraph;

    int start_frame;
    int frames;

    // Dialog Data
        //{{AFX_DATA(CAviDlg)
    enum { IDD = IDD_AVI_DLG };
    CComboBox	m_Time_end;
    CComboBox	m_Time_start;
    CString	m_FileName;
    BOOL	m_MinMaxMode;
    int		m_Delay;
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CAviDlg)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CAviDlg)
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AVIDLG_H__50472CE8_6C8D_4D54_8435_DD65BDB83F17__INCLUDED_)
