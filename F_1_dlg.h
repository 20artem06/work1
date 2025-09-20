// Field_1.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#ifndef F_1_DLG_H
#define F_1_DLG_H

#include <vector>

/////////////////////////////////////////////////////////////////////////////
// CdlgGraph - one-dim graph structure for dialog
struct CdlgGraph
{
    CProject* pProject;
    int     nField;
    BOOL    bPolar;
    BOOL    bFtoX;
    float   fFmin, fFmax;
    int     nArg1;
    int     nX1min, nX1max;
    COLORREF Color;
    int     Style;
    BOOL    bGridL;
    BOOL    bLog;
    CString sName;     //Graph name

    std::vector<int> INDEX;

    CdlgGraph* Next;            //Pointer to next
    CdlgGraph* Prev;            //Pointer to prev
    CdlgGraph();                //Constructor
    ~CdlgGraph() = default;     //Destructor


};

/////////////////////////////////////////////////////////////////////////////
// CField_1_Dlg dialog
class CField_1_Dlg : public CDialog
{
    // Construction
public:
    CField_1_Dlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CField_1_Dlg();

    BOOL	  bNewGraph;
    CdlgGraph* First;
    CdlgGraph* Current;
    CdlgGraph* AxesGraph;

    // Dialog Data
        //{{AFX_DATA(CField_1_Dlg)
    enum { IDD = IDD_FIELD_1 };
    int 	m_nGraph;
    int     m_nProject;
    int		m_nField;
    float	m_fFmin;
    float	m_fFmax;
    BOOL	m_bPolar;
    BOOL	m_bFtoX;
    int		m_nArg1;
    int 	m_nX1min;
    int 	m_nX1max;
    int 	m_nArgument;
    int		m_nParam;
    CString m_sArgName;
    BOOL	m_bGridLines;
    BOOL    m_bLog;
    int     m_nStyle;
    int		m_nAxeStatus;
    int     m_nAxeGraph;
    float	m_fXmax;
    float	m_fXmin;
    float	m_fYmax;
    float	m_fYmin;
    //}}AFX_DATA

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CField_1_Dlg)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    void ChangeAxeStatus();
    void ReadDataToCurrent();
    void ReadDataFromCurrent();
    void OnChangeGraphName(CString ttt);

    // Generated message map functions
    //{{AFX_MSG(CField_1_Dlg)
    virtual void OnOK();
    afx_msg void OnClickedOk();
    afx_msg void OnAxe0();
    afx_msg void OnAxe1();
    afx_msg void OnAxe2();
    afx_msg void OnSelectAxeGraph();
    virtual BOOL OnInitDialog();
    afx_msg void OnAddGraph();
    afx_msg void OnDelGraph();
    afx_msg void OnSelectGraph();
    afx_msg void OnSelectProject();
    afx_msg void OnFieldModify();
    afx_msg void OnChangeArg1();
    afx_msg void OnSelectParam();
    afx_msg void OnSelectNewArgument();
    afx_msg void OnChangeFmin();
    afx_msg void OnChangeFmax();
    afx_msg void OnSelectArg1max();
    afx_msg void OnSelectArg1min();
    afx_msg void OnChangeXmax();
    afx_msg void OnChangeXmin();
    afx_msg void OnChangeYmax();
    afx_msg void OnChangeYmin();
    afx_msg void OnPaint();
    afx_msg void OnColour();
    afx_msg void OnChangeStyle();
    afx_msg void OnBnClickedLoadpreset();
    afx_msg void OnBnClickedSavepreset();

    //	afx_msg void OnPolar();
        //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedLog();
};

/////////////////////////////////////////////////////////////////////////////
#endif // !F_1_DLG_H
