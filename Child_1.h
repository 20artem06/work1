// Child_1.h : interface of the CChild_1_Frame class
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CdlgGraph - one-dim graph structure for dialog

#ifndef CHILD_1_H
#define CHILD_1_H

#include <vector>
#include "Session.h"

struct CGraph
{
    CProject* pProject;     // Attributes from CField_1_Dlg dialog
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
    CString sName;

    std::vector<int> INDEX;

    std::vector<float> Fun;         //Function values
    std::vector<float> X1;          //X1 values
    CString sFName;     //Function name
    CString sX1Name;    //X1-axe name
    int nX1;            //number of points
    float fX1min, fX1max, fX2min, fX2max;   //Axes MIN/MAX values (current)

    CGraph* Next = NULL;
    CGraph* Prev = NULL;
    CGraph() = default;
    ~CGraph() = default;


    VideoSession1Dim* getSession(std::vector<CProject*> &projects);
};

/////////////////////////////////////////////////////////////////////////////
class CChild_1_Frame : public CMDIChildWnd
{
    DECLARE_DYNCREATE(CChild_1_Frame)
public:
    CChild_1_Frame();
    CChild_1_Frame(CField_1_Dlg* pDlg);		//Main constructor
    CChild_1_Frame(OneDimWindowDto& sessions);

// Attributes
public:
    CGraph* Current;
    CGraph* AxesGraph;
    CGraph* First;
    CString sName;							//Window name
    BOOL    bAxes;

    CString MainDir;
    float	fX1min, fX1max, fX2min, fX2max;	//Axes MIN/MAX values for fixed axes
    int     AxeStatus;

    int PicsTotal;

    // Operations
public:

    // Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CChild_1_Frame)
public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    OneDimWindowDto GetSessions(std::vector<CProject*>& projects);
    //}}AFX_VIRTUAL

private:
    void SetData(CField_1_Dlg* pDlg);
// Implementation
public:
    virtual ~CChild_1_Frame();
    void PrepareData();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    // Generated message map functions
protected:
    //	void PrepareData(CField_1_Dlg *dlg);
        //{{AFX_MSG(CChild_1_Frame)
    afx_msg void OnGraphModify();
    afx_msg void OnFileSavedata();
    afx_msg void OnFileSavepict();
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnSaveWmf();
    afx_msg void OnResize();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif // !CHILD_1_H
