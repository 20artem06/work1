/////////////////////////////////////////////////////////////////////////////
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "afx.h"

#include <fstream>
#include <cfloat>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <fstream>

#include "video.h"
#include "MainFrm.h"
#include "F_2_dlg.h"
#include "F_1_dlg.h"
#include "Child_1.h"
#include "Child_2.h"
#include "Session.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#include <io.h>
#include <fcntl.h>

/////////////////////////////////////////////////////////////////////////////
// CGraphSel dialog using in New graph
class CGraphSel : public CDialog
{
    // Construction
public:
    CGraphSel(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
    //{{AFX_DATA(CGraphSel)
    enum { IDD = IDD_SELECTGRAPH };
    int		m_nType;
    //}}AFX_DATA

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CGraphSel)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    // Generated message map functions
    //{{AFX_MSG(CGraphSel)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
CGraphSel::CGraphSel(CWnd* pParent /*=NULL*/)
    : CDialog(CGraphSel::IDD, pParent)
{
    //{{AFX_DATA_INIT(CGraphSel)
    m_nType = 0;
    //}}AFX_DATA_INIT
}

void CGraphSel::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CGraphSel)
    DDX_Radio(pDX, IDC_TYPE0, m_nType);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGraphSel, CDialog)
    //{{AFX_MSG_MAP(CGraphSel)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDelProjectDlg dialog using in Close project
class CDelProjectDlg : public CDialog
{
    // Construction
public:
    CDelProjectDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
    CProject* pProject;
    BOOL	m_bDelAll;

    //{{AFX_DATA(CDelProjectDlg)
    enum { IDD = IDD_DELPROJECT };
    int     m_nProject;
    //}}AFX_DATA

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CDelProjectDlg)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    // Generated message map functions
    //{{AFX_MSG(CDelProjectDlg)
    afx_msg void OnDelAll();
    virtual BOOL OnInitDialog();
    afx_msg void OnSelectProject();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
CDelProjectDlg::CDelProjectDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CDelProjectDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CDelProjectDlg)
    m_nProject = 0;
    //}}AFX_DATA_INIT
}

void CDelProjectDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDelProjectDlg)
    DDX_LBIndex(pDX, IDC_PROJECTLIST, m_nProject);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDelProjectDlg, CDialog)
    //{{AFX_MSG_MAP(CDelProjectDlg)
    ON_BN_CLICKED(ID_DEL_ALL, OnDelAll)
    ON_LBN_SELCHANGE(IDC_PROJECTLIST, OnSelectProject)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Beginning
BOOL CDelProjectDlg::OnInitDialog()
{
    CDialog::OnInitDialog();			// standart call
    m_bDelAll = FALSE;

    CVideoApp* pApp = (CVideoApp*)AfxGetApp();
    CListBox* pBox = (CListBox*)GetDlgItem(IDC_PROJECTLIST);

    // Project - all items
    CProject* pTmp = pApp->First;
    do
    {
        pBox->AddString(pTmp->m_sName);
        pTmp = pTmp->Next;
    } while (pTmp != pApp->First);

    // Project - current item
    pBox->SetCurSel(m_nProject);
    pProject = pApp->First;
    UpdateData(FALSE);

    return TRUE; //standart return
}

/////////////////////////////////////////////////////////////////////////////
// DeleteAll button selected
void CDelProjectDlg::OnDelAll()
{
    m_bDelAll = TRUE;	CDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// Select deleted project
void CDelProjectDlg::OnSelectProject()
{
    CVideoApp* pApp = (CVideoApp*)AfxGetApp();
    int i;
    UpdateData();
    pProject = pApp->First;
    for (i = 0; i < m_nProject; i++) pProject = pProject->Next;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
    //{{AFX_MSG_MAP(CMainFrame)
    ON_WM_CREATE()
    ON_COMMAND(ID_FILE_OPENPROJECT, OnFileOpen)
    ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
    ON_WM_CLOSE()
    ON_WM_SIZE()
    ON_COMMAND(ID_FILE_CLOSE_ALL, OnCloseAllProjects)
    ON_UPDATE_COMMAND_UI(ID_WINDOW_CLOSEALL, OnUpdateWindowCloseAll)
    ON_COMMAND(ID_WINDOW_CLOSEALL, OnWindowClose_All)
    ON_UPDATE_COMMAND_UI(ID_GRAPH_NEW, OnUpdateGraphNew)
    ON_COMMAND(ID_GRAPH_NEW, OnGraphNew)
    //}}AFX_MSG_MAP
    ON_COMMAND(ID_FILE_SAVESESSION, &CMainFrame::OnFileSaveSession)
    ON_COMMAND(ID_FILE_LOADSESSION, &CMainFrame::OnFileLoadSession)
END_MESSAGE_MAP()

static UINT indicators[] =
{
    ID_SEPARATOR,           // status line indicator
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame operations and commands

/////////////////////////////////////////////////////////////////////////////
// Close MainFrame
void CMainFrame::OnClose()
{
    OnCloseAll();
    CMDIFrameWnd::OnClose();
    m_menu.DestroyMenu();
    m_menuGraph.DestroyMenu();
}

/////////////////////////////////////////////////////////////////////////////
// Create MainFrame window
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (!m_wndToolBar.Create(this) ||
        !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
    {
        TRACE0("Failed to create toolbar\n");
        return -1;				// fail to create
    }

    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators,
            sizeof(indicators) / sizeof(int)))
    {
        TRACE0("Failed to create status bar\n");
        return -1;				// fail to create
    }

    m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
        CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&m_wndToolBar);

    return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    return CMDIFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// Create MainFrame resources
BOOL CMainFrame::Create()
{
    CString name;
    name.LoadString(IDR_MAINFRAME);

    m_menu.LoadMenu(IDR_MAINFRAME);
    m_menuGraph.LoadMenu(IDR_VIDEO);

    return CreateEx(0, AfxRegisterWndClass(0,
        NULL, NULL, AfxGetApp()->LoadIcon(IDR_MAINFRAME)),
        name, WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, m_menu);
}

/////////////////////////////////////////////////////////////////////////////
// Prepare status bar on change MainFrame size
void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
    CMDIFrameWnd::OnSize(nType, cx, cy);
    //Prepare status bar
    pStatusBar = (CStatusBarCtrl*)&m_wndStatusBar.GetStatusBarCtrl();
    CRect  WinRec;	GetClientRect(WinRec);	// window size	
    int p0 = WinRec.left + WinRec.Width() / 2;
    int pst = WinRec.Width() / 6;
    int Parts[4] = { p0,p0 + pst,p0 + 2 * pst,p0 + 3 * pst };
    pStatusBar->SetParts(4, Parts);
}

/////////////////////////////////////////////////////////////////////////////
//Close all child windows

// ready to close all if exist at least one
void CMainFrame::OnUpdateWindowCloseAll(CCmdUI* pCmdUI)
{
    if (MDIGetActive() == NULL) pCmdUI->Enable(FALSE);
}

// call by menu or tool bar
void CMainFrame::OnWindowClose_All()
{
    if (AfxMessageBox("Are you sure to close ALL graphs? "
        , MB_YESNO) != IDYES) return;
    OnWindowCloseAll();
}

/////////////////////////////////////////////////////////////////////////////
// Close all according to this project
void CMainFrame::OnWindowCloseAll(CProject* pProject)
{
    CMDIChildWnd* pChild;

    //Close all
    if (pProject == NULL)
        while ((pChild = MDIGetActive()) != NULL)
            pChild->SendMessage(WM_CLOSE);

    //Close only one project
    else
    {
        CString ttt;
        BOOL    bDoNext = TRUE;
        CMDIChildWnd* pChildIni = MDIGetActive();	//first	child window
        pChild = pChildIni;
        //Check every window
        while (pChild != NULL)
        {
            if (bDoNext) MDINext(); bDoNext = TRUE;
            pChild = MDIGetActive();				//checked child window
            pChild->GetWindowText(ttt);			//check title

    // Two-dim graph,
            if (ttt.Find("Two-dim graph ") == 0)
            {
                CChild_2_Frame* pChild2 = (CChild_2_Frame*)MDIGetActive();
                if (pChild2->pProject == pProject) 	// with current project,
                {
                    pChild2->SendMessage(WM_CLOSE);	// chould be closed.
                    bDoNext = FALSE;
                }
            }

            // One-dim graph,
            else
            {
                CChild_1_Frame* pChild1 = (CChild_1_Frame*)MDIGetActive();
                CGraph* pGraph = pChild1->First;
                do
                {
                    if (pGraph->pProject == pProject)	// with current project,
                    {
                        pChild1->SendMessage(WM_CLOSE);	// chould be closed.
                        bDoNext = FALSE;
                        break;
                    }
                    pGraph = pGraph->Next;
                } while (pGraph != pChild1->First);
            }
            if (pChild == pChildIni) break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Call OpenFile dialog 
void CMainFrame::OnFileOpen()
{
    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY,
        "Video files (*.vid)|*.vid| All files (*.*)|*.*||", this);
    if (dlg.DoModal() != IDOK)return;

    OpenProject(dlg.GetPathName());
}

/////////////////////////////////////////////////////////////////////////////
// Close one project
void CMainFrame::OnFileClose()
{
    CDelProjectDlg dlg;
    if (dlg.DoModal() != IDOK) return;			// don't close
    if (dlg.m_bDelAll)	OnCloseAllProjects();	// close all
    else										// close only one
    {
        if (AfxMessageBox("Are you shure to close project ["
            + dlg.pProject->m_sName + "] ?", MB_YESNO) != IDYES) return;

        CVideoApp* pApp = (CVideoApp*)AfxGetApp();
        //Close last project
        if (pApp->First->Next == pApp->First)
        {
            delete pApp->First;	pApp->First = NULL;

            OnWindowCloseAll();	//close all child windows
            SetMenu(&m_menu);	//change menu
        }
        //Close current project
        else
        {
            if (dlg.pProject == pApp->First) pApp->First = pApp->First->Next;

            dlg.pProject->Prev->Next = dlg.pProject->Next;
            dlg.pProject->Next->Prev = dlg.pProject->Prev;

            OnWindowCloseAll(dlg.pProject); //close all accordind to project
            delete dlg.pProject;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Close all projects
void CMainFrame::OnCloseAll()
{
    CVideoApp* pApp = (CVideoApp*)AfxGetApp();
    if (pApp->First)
    {
        CProject* pProject = pApp->First->Next;

        while (pProject != pApp->First)
        {
            pProject = pProject->Next;
            delete   pProject->Prev;
        }
        delete   pProject;

        pApp->First = NULL;

        OnWindowCloseAll();	//close all child windows
        SetMenu(&m_menu);	//change menu
    }
}

// call by menu or tool bar
void CMainFrame::OnCloseAllProjects()
{
    if (AfxMessageBox("Are you shure to close ALL projects ? "
        , MB_YESNO) == IDYES) OnCloseAll();
}

/////////////////////////////////////////////////////////////////////////////
// Error in project file
BOOL CMainFrame::ErrorInProject(CProject* pProject, int Flag,
    int nField, int nAxe, int nPoint)
{
    CString ttt;
    switch (Flag) //error type switch
    {
        //Project file not found
    case -1:
        ttt = "Data description file '" + pProject->m_sName + "' not found ";
        break;
        //Unexpected end of project file
    case  0:
        ttt = "Unexpected end of file '" + pProject->m_sName + "'";
        break;
        //Bad field number
    case  1:
    {
        ttt.Format("%i", pProject->m_nFields);
        ttt = "Error in number of fields:  " + ttt
            + " in file '" + pProject->m_sName + "'";
    }break;
    //Bad number of axes
    case  2:
    {
        ttt.Format("%i", pProject->m_field[nField].nAxis);
        ttt = "Error in number of axes:  " + ttt
            + " in field '" + pProject->m_field[nField].name
            + "' in file '" + pProject->m_sName + "'";
    }break;
    //Bad number of points in axe
    case  3:
        ttt = "Error in number of points  in axe '"
            + pProject->m_field[nField].axe[nAxe].name
            + "' in field '" + pProject->m_field[nField].name
            + "' in file '" + pProject->m_sName + "'";
        break;
        //Bad x[0] in standart axe
    case  4:
        ttt = "Error in initual value in axe '"
            + pProject->m_field[nField].axe[nAxe].name
            + "' in field '" + pProject->m_field[nField].name
            + "' in file '" + pProject->m_sName + "'";
        break;
        //Bad step in standart axe
    case  5:
        ttt = "Error in step in axe '"
            + pProject->m_field[nField].axe[nAxe].name
            + "' in field '" + pProject->m_field[nField].name
            + "' in file '" + pProject->m_sName + "'";
        break;
        //Bad x[k] in unstandart axe
    case  6:
    {
        ttt.Format("%i", nPoint + 1);
        ttt = "Error in value number:  " + ttt
            + " in axe '" + pProject->m_field[nField].axe[nAxe].name
            + "' in field '" + pProject->m_field[nField].name
            + "' in file '" + pProject->m_sName + "'";
    }break;
    //Data file not found
    case -2:
        ttt = "Data file not found in project '"
            + pProject->m_sName + "'";
        break;
        //Unexpected end of data file
    case -3:
        ttt = "Unexpected end of data file in project '"
            + pProject->m_sName + "'";
        break;
    }
    AfxMessageBox(ttt, MB_OK | MB_ICONERROR);//message about error
    delete pProject;
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Open new project - main MainFrame procedure:
// - prepare project name;
// - reading data from data description file ;
// - reading data from data file;
// - add project to project list.
BOOL CMainFrame::OpenProject(CString name, bool silentMode)
{
    CProject* pProject = new CProject;

    int k = name.ReverseFind('\\');

    // check is it relative or absolute path
    bool isRelative = false;
    if (k != -1) {
        // set project name by file name
        pProject->m_sName = name.Mid(k + 1);
        // get path to the working folder
        MainDir = name.Left(k + 1);
    } else {
        isRelative = true;
        // set project name by file name
        pProject->m_sName = name;
        // get path to the working folder
        GetCurrentDirectory(MAX_PATH, MainDir.GetBufferSetLength(MAX_PATH));
        MainDir.ReleaseBuffer();
        MainDir += '\\';
    }

    // get full path to the .dat file
    CString datFilePath = MainDir;
    pProject->projectPath = MainDir;

    // Reading data description file
    /////////////////////////////////////////////////////////////////////////////
    char szBuffer[100];
    std::fstream file;
    file.open(name, std::ios::_Nocreate | std::ios::in);
    if (file.fail() != 0) return ErrorInProject(pProject, -1);

    // Common: Data file name, T-name, T-dim, Nfields
    file >> szBuffer;  datFilePath += szBuffer;
    file >> szBuffer;  pProject->m_sTime = szBuffer;
    file >> szBuffer;  pProject->m_sTimeSize = szBuffer;
    if (szBuffer == "_")pProject->m_sTimeSize = "";

    pProject->m_nFieldPoints = 1;
    file >> k;		   pProject->m_nFields = k;
    if (file.eof() != 0) return ErrorInProject(pProject);
    if ((file.fail() != 0) || (k <= 0))
        return ErrorInProject(pProject, 1);

    pProject->m_field = new CField[pProject->m_nFields];

    // Cleaning fields axes, points, min and max
    int i, j;
    double x0, dx;
    for (i = 0; i < pProject->m_nFields; i++)
    {
        pProject->m_field[i].axe = NULL;
        pProject->m_field[i].point = NULL;
        pProject->m_field[i].min = FLT_MAX;
        pProject->m_field[i].max = -FLT_MAX;
    }

    ////////////////////////////////////////////////////////////
    for (i = 0; i < pProject->m_nFields; i++)  // for every field
    {
        // F-name, F-size, N-axes
        file >> szBuffer;  pProject->m_field[i].name = szBuffer;
        file >> szBuffer;  pProject->m_field[i].size = szBuffer;
        if (szBuffer == "_")pProject->m_field[i].size = "";

        file >> k;     pProject->m_field[i].nAxis = k;
        if (pProject->m_field[i].nAxis >= 1)	pProject->m_bOneDimOnly = FALSE;
        if (file.eof() != 0) return ErrorInProject(pProject);
        if ((file.fail() != 0) || (k < 0))
            return ErrorInProject(pProject, 2, i);

        if (pProject->m_field[i].nAxis)
            pProject->m_field[i].axe = new CAxe[pProject->m_field[i].nAxis];

        // Cleaning axe points
        for (j = 0; j < pProject->m_field[i].nAxis; j++)
            pProject->m_field[i].axe[j].point = NULL;

        pProject->m_field[i].nPoints = 1;
        /////////////////////////////////////////////////////////////////
        for (j = 0; j < pProject->m_field[i].nAxis; j++)	 // for every axe
        {

            // axe-name, axe-size, N-axe-points
            file >> szBuffer;  pProject->m_field[i].axe[j].name = szBuffer;
            file >> szBuffer;  pProject->m_field[i].axe[j].size = szBuffer;
            if (szBuffer == "_")pProject->m_field[i].axe[j].size = "";

            file >> k; pProject->m_field[i].axe[j].nPoints = abs(k);
            if (file.eof() != 0) return ErrorInProject(pProject);
            if (file.fail() != 0) return ErrorInProject(pProject, 3, i, j);

            if (k != 0)
            {
                pProject->m_field[i].axe[j].point =
                    new float[pProject->m_field[i].axe[j].nPoints];
                // Standart axe
                if (k > 0)
                {
                    file >> x0;  pProject->m_field[i].axe[j].point[0] = (float)x0;
                    if (file.fail() != 0) return ErrorInProject(pProject, 4, i, j);
                    file >> dx;
                    if (file.eof() != 0) return ErrorInProject(pProject);
                    if ((file.fail() != 0) || (dx <= 0))
                        return ErrorInProject(pProject, 5, i, j);

                    for (k = 1; k < (int)pProject->m_field[i].axe[j].nPoints; k++)
                        pProject->m_field[i].axe[j].point[k] =
                        pProject->m_field[i].axe[j].point[k - 1] + (float)dx;
                }
                // Unstandart axe
                else
                {
                    for (k = 0; k < (int)pProject->m_field[i].axe[j].nPoints; k++)
                    {
                        file >> x0;
                        if (file.eof() != 0) return ErrorInProject(pProject);
                        if (file.fail() != 0) return ErrorInProject(pProject, 6, i, j, k);
                        pProject->m_field[i].axe[j].point[k] = (float)x0;
                    }
                }
                pProject->m_field[i].nPoints *= pProject->m_field[i].axe[j].nPoints;
            }
        }
        pProject->m_nFieldPoints += pProject->m_field[i].nPoints;
    }
    file.close();
    //Reading CEL file
    pProject->ReadGrid("data.grd");

    // Reading data file
    /////////////////////////////////////////////////////////////////////////////
    CString ttt;
    pProject->m_iHandle = _open(datFilePath, _O_RDONLY | _O_BINARY);
    if (pProject->m_iHandle == -1)
        return ErrorInProject(pProject, -2);           //error in data file name
    __int64 len = _filelengthi64(pProject->m_iHandle);//file size

    pProject->m_nTimePoints = int(len / (pProject->m_nFieldPoints * 4));
    if (pProject->m_nTimePoints < 1)return ErrorInProject(pProject, -3);

    // Asking about working with only top of the file
    if (pProject->m_nTimePoints > TabMax)
    {
        CString ttt1, ttt2;
        ttt1.Format("%i", pProject->m_nTimePoints);
        ttt2.Format("%i", TabMax);
        ttt = "File  " + datFilePath
            + "\nconsists of too many time records - " + ttt1 + " ."
            + "\nRead only first " + ttt2 + " of them?";
        if (AfxMessageBox(ttt, MB_YESNO | MB_ICONQUESTION) == IDNO)
        {
            delete pProject;	return FALSE;
        }
    }

    //Memory operations
    BOOL bRead = TRUE;
    if (pProject->m_nTimePoints)
    {
        pProject->m_time = new float[pProject->m_nTimePoints];
        if (len < MemMax)
            for (i = 0; i < pProject->m_nFields; i++)
            {
                pProject->m_field[i].point = new float[
                    pProject->m_field[i].nPoints * pProject->m_nTimePoints];
            } else
            {
                pProject->m_bInMemory = FALSE;
                for (i = 0; i < pProject->m_nFields; i++)
                {
                    pProject->m_field[i].min = -FLT_MAX;
                    pProject->m_field[i].max = FLT_MAX;
                }
            }
    }

    if (bRead)
    {
        // Prepare progress indicator window
        CRect Rec; GetClientRect(Rec);
        Rec.top += Rec.Height() * 1 / 10;		Rec.bottom = Rec.top + 50;
        k = Rec.Width();	Rec.left += k / 3;	Rec.right -= k / 3;
        CProgressCtrl pProg;
        pProg.Create(WS_CHILD | WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION,
            Rec, this, ID_PROGRESS);
        pProg.SetWindowText("Reading file " + pProject->m_sName);
        pProg.ShowWindow(SW_SHOW);

        // Reading data
        int l;
        int nProcentOld = 0, nProcent = 0;
        for (k = 0; k < (int)pProject->m_nTimePoints; k++)
        {
            _read(pProject->m_iHandle, &pProject->m_time[k], 4);    //time reading
            if (pProject->m_bInMemory)
                for (i = 0; i < pProject->m_nFields; i++)
                    for (j = 0; j < pProject->m_field[i].nPoints; j++)
                    {
                        l = k * pProject->m_field[i].nPoints + j;
                        _read(pProject->m_iHandle, &pProject->m_field[i].point[l], 4);  //field reading
                        if (pProject->m_field[i].min > pProject->m_field[i].point[l])
                            pProject->m_field[i].min = pProject->m_field[i].point[l];
                        if (pProject->m_field[i].max < pProject->m_field[i].point[l])
                            pProject->m_field[i].max = pProject->m_field[i].point[l];
                    } else
                        _lseeki64(pProject->m_iHandle, pProject->m_nFieldPoints * 4 - 4, SEEK_CUR); //field skipping

        // Show progress
                    nProcent = 100 * k / pProject->m_nTimePoints;
                    if (nProcent != nProcentOld)
                    {
                        pProg.SetPos(nProcent);
                        pProg.UpdateWindow();
                        nProcentOld = nProcent;
                    }
        }

        pProg.DestroyWindow();
        if (pProject->m_bInMemory) _close(pProject->m_iHandle);
    }

    // Without reading
    else
    {
        _read(pProject->m_iHandle, &pProject->m_time[0], 4);//time0 reading
        _lseeki64(pProject->m_iHandle, pProject->m_nFieldPoints * 4 - 4, SEEK_CUR);
        _read(pProject->m_iHandle, &pProject->m_time[1], 4);//time1 reading
        float dt = pProject->m_time[1] - pProject->m_time[0];
        for (k = 2; k < (int)pProject->m_nTimePoints; k++)
            pProject->m_time[k] = pProject->m_time[k - 1] + dt;
    }

    // Add project to project list
    /////////////////////////////////////////////////////////////////////////////

    CWinApp* app = AfxGetApp();

    // do not add relative file to the MRU
    if (!isRelative) {
        app->AddToRecentFileList(name);
    }

    CVideoApp* pApp = (CVideoApp*)app;

    //First project
    if (pApp->First == NULL)
    {
        pApp->First = pProject;
        pApp->First->Next = pProject;
        pApp->First->Prev = pProject;
        SetMenu(&m_menuGraph);	//change menu
        MDISetMenu(NULL, m_menuGraph.GetSubMenu(3));
    }

    //Add project to the end of list
    else
    {
        pProject->Next = pApp->First;
        pProject->Prev = pApp->First->Prev;
        pApp->First->Prev->Next = pProject;
        pApp->First->Prev = pProject;
    }
    if (!silentMode) {
        OnGraphNew();           //draw new graph
    }
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Create NEW graph
//		No projects, no new graphs	
void CMainFrame::OnUpdateGraphNew(CCmdUI* pCmdUI)
{
    CVideoApp* pApp = (CVideoApp*)AfxGetApp();
    if (pApp->First == NULL) pCmdUI->Enable(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// Really create new graph
void CMainFrame::OnGraphNew()
{
    CVideoApp* pApp = (CVideoApp*)AfxGetApp();

    //Select graph type
    CGraphSel dlg;
    if (dlg.DoModal() != IDOK) return;

    switch (dlg.m_nType)
    {
        //ONE-Dim graph
        ///////////////
    case 0:
    {
        //Select graph in CField_1_Dlg
        CField_1_Dlg dlg1;
        dlg1.bNewGraph = TRUE;	//	Init value for new graph	
        if (dlg1.DoModal() != IDOK) return;
        CChild_1_Frame* pChild = new CChild_1_Frame(&dlg1);
        pChild->Create(NULL, pChild->sName, WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_MAXIMIZE);
    }
    break;

    //TWO-Dim graph
    ///////////////
    case 1:
    {
        //Select graph in CField_2_Dlg
        CField_2_Dlg dlg2;
        dlg2.bNewGraph = TRUE;	//	Init value for new graph	
        if (dlg2.DoModal() != IDOK) return;
        if (dlg2.pProject->m_field[dlg2.m_nField].nAxis == 0)
        {
            AfxMessageBox("Two-dim graph can't be build !");
            return;
        }
        CChild_2_Frame* pChild = new CChild_2_Frame(&dlg2);
        pChild->Create(NULL, pChild->sName, WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_MAXIMIZE);
    }
    break;
    }
}


std::vector<CProject*> CMainFrame::getProjects() {

    std::unordered_set<CProject*> projects;
    CVideoApp* pApp = (CVideoApp*)AfxGetApp();
    auto fProj = ((CVideoApp*)AfxGetApp())->First;
    projects.insert(fProj);

    if (fProj == NULL) {
        return std::vector<CProject*>();
    }

    for (auto pr = fProj->Next; pr != fProj; pr = pr->Next) {
        projects.insert(pr);
    }

    std::vector<CProject*> out(projects.begin(), projects.end());

    return out;
}


std::unordered_set<std::string> CMainFrame::getPathsoOfOpenedProjects() {
    std::unordered_set<std::string> out;
    auto projects = getProjects();

    for (auto& p : projects) {
        out.insert(std::string((p->projectPath + p->m_sName)));
    }

    return out;
}

void CMainFrame::OnFileLoadSession() {
    std::ifstream inp;
    VideoSaveDto dto;
    std::vector<CProject*> pr;

    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY,
        "Video session file (*.json)|*.json", this);
    if (dlg.DoModal() != IDOK)return;
    std::string sessionFilePath = dlg.GetPathName();

    inp.open(sessionFilePath, std::ios_base::in);
    if (!inp.is_open()) {
        return;
    }
    
    try
    {
        inp >> dto;
    }
    catch (const std::exception& e)
    {
        return;
    }

    auto openedProjects = getPathsoOfOpenedProjects();

    for (auto& path : dto.projects) {
        if (!openedProjects.count(path))
            OpenProject(path.c_str(), true);
    }

    auto p = getProjects();
    for (auto& a : p) {
        pr.push_back(a);
    }

    auto cur = ((CVideoApp*)AfxGetApp())->First;
    auto f = cur;
    std::unordered_map<std::string, CProject*> projectMap;
    do
    {
        projectMap[std::string(cur->projectPath + cur->m_sName)] = cur;
        cur = cur->Next;
    } while (cur != f);


    for (auto& s : dto.twoDimGraphs) {
        auto prPath = dto.projects[s.projectIndex];
        if (!projectMap.count(prPath)) {
            continue;
        }
        
        s.pProject = projectMap[prPath];
        s.ColorMin = RGB(0, 0, 255);
        s.ColorMax = RGB(255, 0, 0);

        CChild_2_Frame* pChild = new CChild_2_Frame(&s);
        pChild->Create(NULL, pChild->sName, WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_MAXIMIZE);

    }

    for (auto& s : dto.oneDimGraphs) {
        bool ok = true;
        for (auto& ent : s.graphs) {
            auto prPath = dto.projects[ent.projectIndex];
            if (!projectMap.count(prPath)) {
                ok = false;
                continue;
            }
            ent.pProject = projectMap[prPath];
        }
        if (!ok) { continue; }
        CChild_1_Frame* pChild = new CChild_1_Frame(s);
        pChild->Create(NULL, pChild->sName, WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_MAXIMIZE);
    }

}


void CMainFrame::OnFileSaveSession()
{   
    std::ofstream outfile;
    CFileDialog dlg(FALSE, "json", "VideoSession.json", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "session files (*.json)|*.json||");
    dlg.m_ofn.lpstrInitialDir = MainDir;
    if (dlg.DoModal() != IDOK)return;	//Take filename from 'SAVE AS' dialog
    std::string sessionFilePath = dlg.GetPathName();

    outfile.open(sessionFilePath, std::ios_base::out);
    if (!outfile.is_open() || sessionFilePath == "") {
        return;
    }

    VideoSaveDto dto;
    CMDIChildWnd* pChild;
    CString title;
    auto projects = getProjects();
    CMDIChildWnd* pChildIni = MDIGetActive();

    for (auto& p : projects) {
        dto.projects.push_back(std::string((p->projectPath + p->m_sName)));
    }

    pChild = pChildIni;
    while (pChild != NULL) {

        pChild->GetWindowText(title);


        if (title.Find("Two-dim graph ") == 0) { // two dim
            int projectIndex = 0;
            auto twoDimWin = (CChild_2_Frame*)MDIGetActive();
            auto it = std::find(projects.begin(), projects.end(), twoDimWin->pProject);
            if (it != projects.end()) {
                projectIndex = it - projects.begin();
            }

            auto s = twoDimWin->GetSession();
            s.projectIndex = projectIndex;
            dto.twoDimGraphs.push_back(s);
        }
        else {
            auto oneDimWin = (CChild_1_Frame*)MDIGetActive();
            dto.oneDimGraphs.push_back(oneDimWin->GetSessions(projects));
        }
        


        MDINext();
        pChild = MDIGetActive();
        if (pChild == pChildIni) break;
    }

    
    try
    {
        if (outfile.is_open()) {
            outfile << dto;
        }
    }
    catch (const std::exception&)
    {
        return;
    }

}




