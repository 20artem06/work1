// F_2_dlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#ifndef F_2_DLG_H
#define F_2_DLG_H

#include <vector>

// CField_2_Dlg dialog
class CField_2_Dlg : public CDialog
{
    // Construction
public:
    CField_2_Dlg(CWnd* pParent = NULL);   // standard constructor
    ~CField_2_Dlg() = default;

    // Dialog Data
    BOOL	 bNewGraph;
    COLORREF ColorMin, ColorMax;
    CProject* pProject;

    std::vector<int> INDEX;

    //{{AFX_DATA(CField_2_Dlg)
    enum { IDD = IDD_FIELD_2 };
    int     m_nProject;
    int		m_nField;
    float	m_fFmin;
    float	m_fFmax;
    BOOL	m_bPolar;
    int		m_nArg1;
    int 	m_nX1min;
    int 	m_nX1max;
    int		m_nArg2;
    int 	m_nX2min;
    int 	m_nX2max;
    int 	m_nArgument;
    int		m_nParam;
    CString m_sArgName;
    int 	m_nGrids;
    BOOL	m_bGridLines;
    BOOL	m_lines;
    BOOL	m_cloud;
    BOOL	m_layers;
    int		m_Palette;
    BOOL    m_bLog;
    BOOL    m_bGridAuto;

    //}}AFX_DATA

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CField_2_Dlg)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    // Generated message map functions
    //{{AFX_MSG(CField_2_Dlg)
    virtual void OnOK();
    afx_msg void OnClickedOk();
    virtual BOOL OnInitDialog();
    afx_msg void OnSelectProject();
    afx_msg void OnFieldModify();
    afx_msg void OnChangeArg1();
    afx_msg void OnChangeArg2();
    afx_msg void OnSelectParam();
    afx_msg void OnSelectNewArgument();
    afx_msg void OnChangeFmin();
    afx_msg void OnChangeFmax();
    afx_msg void OnSelectArg1max();
    afx_msg void OnSelectArg1min();
    afx_msg void OnSelectArg2max();
    afx_msg void OnSelectArg2min();
    afx_msg void OnChangeNgrids();
    afx_msg void OnPaint();
    afx_msg void OnColorMin();
    afx_msg void OnColorMax();
    afx_msg void OnBnClickedLoadpreset();
    afx_msg void OnBnClickedSavepreset();

    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedLog();
    afx_msg void OnBnClickedLines();
    afx_msg void OnBnClickedCheck1();
};

/////////////////////////////////////////////////////////////////////////////
#endif // !F_2_DLG_H
