// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include <unordered_set>
#include <string>

class CMainFrame : public CMDIFrameWnd
{
    DECLARE_DYNAMIC(CMainFrame)
public:
    CMainFrame();

    // Attributes
public:
    CMenu m_menu, m_menuGraph;
    CString MainDir;

    // Operations
public:
    BOOL Create();
    BOOL OpenProject(CString name, bool silentMode = false);
protected:
    BOOL ErrorInProject(CProject* pProject
        , int Flag = 0, int nField = 0, int nAxe = 0, int nPoint = 0);
    void OnWindowCloseAll(CProject* pProject = NULL);
    void OnCloseAll();

    std::vector<CProject*> getProjects();

    // Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CMainFrame)
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CMainFrame();

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    // Control bar embedded members
public:
    CStatusBarCtrl* pStatusBar;
protected:
    CStatusBar  m_wndStatusBar;
    CToolBar    m_wndToolBar;

    std::unordered_set<std::string> getPathsoOfOpenedProjects();

    // Generated message map functions
protected:
    //{{AFX_MSG(CMainFrame)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnFileOpen();
    afx_msg void OnFileClose();
    afx_msg void OnClose();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnCloseAllProjects();
    afx_msg void OnUpdateWindowCloseAll(CCmdUI* pCmdUI);
    afx_msg void OnWindowClose_All();
    afx_msg void OnUpdateGraphNew(CCmdUI* pCmdUI);
    afx_msg void OnGraphNew();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnFileSaveSession();
    afx_msg void OnFileLoadSession();
};

/////////////////////////////////////////////////////////////////////////////

