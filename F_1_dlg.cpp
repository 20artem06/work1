/////////////////////////////////////////////////////////////////////////////
// Field_1.cpp : implementation file
//

#include "stdafx.h"
#include "video.h"
#include "MainFrm.h"
#include "F_1_dlg.h"

#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CdlgGraph contructors/destructors

CdlgGraph::CdlgGraph()
{
    Next = NULL;
    Prev = NULL;
    bPolar = FALSE;
    bFtoX = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CField_1_Dlg dialog constructor
CField_1_Dlg::CField_1_Dlg(CWnd* pParent /*=NULL*/)
    : CDialog(CField_1_Dlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CField_1_Dlg)
    m_nGraph = 0;
    m_nProject = 0;
    m_nField = -1;
    m_fFmin = 0.0f;
    m_fFmax = 0.0f;
    m_bPolar = FALSE;
    m_bFtoX = FALSE;
    m_nArg1 = -1;
    m_nX1min = -1;
    m_nX1max = -1;
    m_nArgument = -1;
    m_nParam = -1;
    m_bGridLines = FALSE;
    m_bLog = FALSE;
    m_nStyle = 0;
    m_nAxeStatus = 2;
    m_nAxeGraph = -1;
    m_fXmin = 0.0f;
    m_fYmin = 0.0f;
    m_fXmax = 0.0f;
    m_fYmax = 0.0f;
    //}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////
// CField_1_Dlg dialog destructor
CField_1_Dlg::~CField_1_Dlg()
{
    if (First)
    {
        CdlgGraph* pDlgGraph = First->Next;

        while (pDlgGraph != First)
        {
            pDlgGraph = pDlgGraph->Next;
            delete   pDlgGraph->Prev;
        }
        delete   pDlgGraph;
    }
}

/////////////////////////////////////////////////////////////////////////////
void CField_1_Dlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CField_1_Dlg)
    DDX_LBIndex(pDX, IDC_GRAPH, m_nGraph);
    DDX_CBIndex(pDX, IDC_PROJECT, m_nProject);
    DDX_CBIndex(pDX, IDC_FIELD, m_nField);
    DDX_Text(pDX, IDC_FMIN, m_fFmin);
    DDX_Text(pDX, IDC_FMAX, m_fFmax);
    //DDX_Check   (pDX, IDC_POLAR,	m_bPolar);
    //DDX_Check   (pDX, IDC_POLAR2,	m_bFtoX);
    DDX_CBIndex(pDX, IDC_ARG1, m_nArg1);
    DDX_CBIndex(pDX, IDC_ARG1MIN, m_nX1min);
    DDX_CBIndex(pDX, IDC_ARG1MAX, m_nX1max);
    DDX_CBIndex(pDX, IDC_ARGUMENT, m_nArgument);
    DDX_LBIndex(pDX, IDC_PARAM, m_nParam);
    DDX_Text(pDX, IDC_ARGNAME, m_sArgName);
    DDX_Check(pDX, IDC_GRIDLINES, m_bGridLines);
    DDX_CBIndex(pDX, IDC_STYLE, m_nStyle);
    DDX_Radio(pDX, IDC_AXE, m_nAxeStatus);
    DDX_CBIndex(pDX, IDC_AXEGRAPH, m_nAxeGraph);
    DDX_Text(pDX, IDC_XMIN, m_fXmin);
    DDX_Text(pDX, IDC_XMAX, m_fXmax);
    DDX_Text(pDX, IDC_YMAX, m_fYmax);
    DDX_Text(pDX, IDC_YMIN, m_fYmin);
    //}}AFX_DATA_MAP
    DDX_Check(pDX, IDC_LOG, m_bLog);
}

BEGIN_MESSAGE_MAP(CField_1_Dlg, CDialog)
    //{{AFX_MSG_MAP(CField_1_Dlg)
    ON_BN_CLICKED(IDC_OK, OnClickedOk)
    ON_BN_CLICKED(IDC_AXE, OnAxe0)
    ON_BN_CLICKED(IDC_AXE1, OnAxe1)
    ON_BN_CLICKED(IDC_AXE2, OnAxe2)
    ON_CBN_SELCHANGE(IDC_AXEGRAPH, OnSelectAxeGraph)
    ON_BN_CLICKED(IDC_ADD, OnAddGraph)
    ON_BN_CLICKED(IDC_DEL, OnDelGraph)
    ON_CBN_SELCHANGE(IDC_GRAPH, OnSelectGraph)
    ON_CBN_SELCHANGE(IDC_PROJECT, OnSelectProject)
    ON_CBN_SELCHANGE(IDC_FIELD, OnFieldModify)
    ON_CBN_SELCHANGE(IDC_ARG1, OnChangeArg1)
    ON_LBN_SELCHANGE(IDC_PARAM, OnSelectParam)
    ON_CBN_SELCHANGE(IDC_ARGUMENT, OnSelectNewArgument)
    ON_EN_KILLFOCUS(IDC_FMIN, OnChangeFmin)
    ON_EN_KILLFOCUS(IDC_FMAX, OnChangeFmax)
    ON_CBN_SELCHANGE(IDC_ARG1MAX, OnSelectArg1max)
    ON_CBN_SELCHANGE(IDC_ARG1MIN, OnSelectArg1min)
    ON_EN_KILLFOCUS(IDC_XMAX, OnChangeXmax)
    ON_EN_KILLFOCUS(IDC_XMIN, OnChangeXmin)
    ON_EN_KILLFOCUS(IDC_YMAX, OnChangeYmax)
    ON_EN_KILLFOCUS(IDC_YMIN, OnChangeYmin)
    ON_WM_PAINT()
    ON_BN_CLICKED(IDC_COLOR, OnColour)
    ON_CBN_SELCHANGE(IDC_STYLE, OnChangeStyle)
    ON_BN_CLICKED(IDC_LOADPRESET, OnBnClickedLoadpreset)
    ON_BN_CLICKED(IDC_SAVEPRESET, OnBnClickedSavepreset)

    //ON_BN_CLICKED   (IDC_POLAR,     OnPolar)
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_LOG, &CField_1_Dlg::OnBnClickedLog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CField_1_Dlg message handlers
void CField_1_Dlg::OnOK()
{
    NextDlgCtrl();
}

void CField_1_Dlg::OnClickedOk()
{
    UpdateData();
   /* if (m_bLog)
    {
        if (m_fFmin <= 0)
        {
            AfxMessageBox("Минимальное значение функции меньше или равно нулю! Выбрана логарифмическая шакала!");
            return;
        }
    }*/
    ReadDataToCurrent();
    CDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// Axe status functions	
void CField_1_Dlg::OnAxe0()		// Fixed axes selected
{
    OnSelectAxeGraph();
    ChangeAxeStatus();
}

void CField_1_Dlg::OnAxe1()		// Unfixed axes selected
{
    ChangeAxeStatus();
}

void CField_1_Dlg::OnAxe2()		// "Axes from:" selected 
{
    OnSelectAxeGraph();
    ChangeAxeStatus();
}

/////////////////////////////////////////////////////////////////////////////
// Change axe status	
void CField_1_Dlg::ChangeAxeStatus()
{
    CComboBox* pABox = (CComboBox*)GetDlgItem(IDC_AXEGRAPH);
    CButton* pAxe0 = (CButton*)GetDlgItem(IDC_AXE);
    CButton* pAxe1 = (CButton*)GetDlgItem(IDC_AXE1);
    CEdit* pXmin = (CEdit*)GetDlgItem(IDC_XMIN);
    CEdit* pXmax = (CEdit*)GetDlgItem(IDC_XMAX);
    CEdit* pYmin = (CEdit*)GetDlgItem(IDC_YMIN);
    CEdit* pYmax = (CEdit*)GetDlgItem(IDC_YMAX);

    if (bNewGraph)	UpdateData();

    // Check new axe status
    switch (m_nAxeStatus)
    {
    case 0:		// Fixed axes
    {
        pXmin->EnableWindow();			//open X/Y  min/max windows
        pXmax->EnableWindow();
        pYmin->EnableWindow();
        pYmax->EnableWindow();
        pABox->EnableWindow(FALSE);		//close AxesGraph window
    } break;
    case 1:		// Unfixed axes
    {
        pXmin->EnableWindow(FALSE);		//close all common graph properties
        pXmax->EnableWindow(FALSE);
        pYmin->EnableWindow(FALSE);
        pYmax->EnableWindow(FALSE);
        pABox->EnableWindow(FALSE);
    } break;
    case 2:		// Axes from graph
    {
        pXmin->EnableWindow(FALSE);		//close X/Y  min/max windows
        pXmax->EnableWindow(FALSE);
        pYmin->EnableWindow(FALSE);
        pYmax->EnableWindow(FALSE);
        pABox->EnableWindow();			//open AxesGraph window
        pABox->SetCurSel(m_nAxeGraph);	//select current axes graph
    } break;
    }
    UpdateData(FALSE);
}

////////////////////////////////////////////////////////////////////////
// Change graph with axes
void CField_1_Dlg::OnSelectAxeGraph()
{
    int i, n;

    //Keep old graph number
    int	nGraphOld = m_nAxeGraph;
    if (bNewGraph) { UpdateData(); ReadDataToCurrent(); }

    //Find new graph
    n = abs(m_nAxeGraph - nGraphOld);

    if (nGraphOld < m_nAxeGraph)
        for (i = 0; i < n; i++) AxesGraph = AxesGraph->Next;
    else
        for (i = 0; i < n; i++) AxesGraph = AxesGraph->Prev;

    //Update data from new graph
    m_fYmin = AxesGraph->fFmin;
    m_fYmax = AxesGraph->fFmax;
    if (AxesGraph->nArg1 < (int)AxesGraph->pProject->m_field[AxesGraph->nField].nAxis)//axe
    {
        m_fXmin = AxesGraph->pProject->m_field[AxesGraph->nField].axe[AxesGraph->nArg1].point[AxesGraph->nX1min];
        m_fXmax = AxesGraph->pProject->m_field[AxesGraph->nField].axe[AxesGraph->nArg1].point[AxesGraph->nX1max];
    } else																			 //time
    {
        m_fXmin = AxesGraph->pProject->m_time[AxesGraph->nX1min];
        m_fXmax = AxesGraph->pProject->m_time[AxesGraph->nX1max];
    }

    UpdateData(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// Read current data to CdlgGraph
void CField_1_Dlg::ReadDataToCurrent()
{
    UpdateData();
    Current->nField = m_nField;
    Current->bPolar = m_bPolar;
    Current->bFtoX = m_bFtoX;
    Current->fFmin = m_fFmin;
    Current->fFmax = m_fFmax;
    Current->nArg1 = m_nArg1;
    Current->nX1min = m_nX1min,
    Current->nX1max = m_nX1max;
    Current->Style = m_nStyle;
    Current->bGridL = m_bGridLines;
    Current->bLog = m_bLog;
}

/////////////////////////////////////////////////////////////////////////////
// Read data from CdlgGraph
void CField_1_Dlg::ReadDataFromCurrent()
{
    m_nField = Current->nField;
    m_bPolar = Current->bPolar;
    m_bFtoX = Current->bFtoX;
    m_fFmin = Current->fFmin;
    m_fFmax = Current->fFmax;
    m_nArg1 = Current->nArg1;
    m_nX1min = Current->nX1min,
    m_nX1max = Current->nX1max;
    m_nStyle = Current->Style;
    m_bGridLines = Current->bGridL;
    m_bLog = Current->bLog;
    UpdateData(FALSE);
    ChangeAxeStatus();
    OnPaint();			//Redraw
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Beginning
BOOL CField_1_Dlg::OnInitDialog()
{
    CDialog::OnInitDialog();			// standart call

    CVideoApp* pApp = (CVideoApp*)AfxGetApp();
    CComboBox* pPBox = (CComboBox*)GetDlgItem(IDC_PROJECT);
    CListBox* pGBox = (CListBox*)GetDlgItem(IDC_GRAPH);
    CComboBox* pABox = (CComboBox*)GetDlgItem(IDC_AXEGRAPH);

    // Clear for NEW Graph
    if (bNewGraph)
    {
        First = new CdlgGraph; 		//Create first graph
        First->Next = First->Prev = First;
        Current = AxesGraph = First;
        Current->sName = "";
        SetWindowText("Build new ONE-DIM graph");
        Current->pProject = pApp->First;// Current project
        Current->Color = RGB(0, 0, 255);	// Blue color
        m_nAxeStatus = 2;
        ChangeAxeStatus();
    } else
    {
        SetWindowText("Modify ONE-DIM graph");
        ReadDataFromCurrent();			//Read current graph
    }

    // Project - all items
    CProject* pTmp = pApp->First;
    do
    {
        pPBox->AddString(pTmp->m_sName);
        pTmp = pTmp->Next;
    } while (pTmp != pApp->First);

    // Graphs, AxesGraph - all items for beginning
    int i = 0;
    CString ttt;
    CdlgGraph* pGraph = First;
    do
    {
        ttt.Format("%i", i + 1);	ttt += ". " + pGraph->sName;
        pGBox->AddString(ttt);
        pABox->AddString(ttt);
        if (pGraph == Current)	m_nGraph = (int)i;
        if (pGraph == AxesGraph) m_nAxeGraph = (int)i;
        pGraph = pGraph->Next;	i++;
    } while (pGraph != First);

    // Graphs -  current item
    pGBox->SetCurSel(m_nGraph);
    UpdateData(FALSE);
    OnSelectGraph();
    //OnPolar();
    return TRUE; //standart return
}

/////////////////////////////////////////////////////////////////////////////
// Add new graph
void CField_1_Dlg::OnAddGraph()
{
    CVideoApp* pApp = (CVideoApp*)AfxGetApp();
    CListBox* pGBox = (CListBox*)GetDlgItem(IDC_GRAPH);
    CComboBox* pABox = (CComboBox*)GetDlgItem(IDC_AXEGRAPH);
    CComboBox* pPBox = (CComboBox*)GetDlgItem(IDC_PROJECT);

    ReadDataToCurrent();			//save old graph data
    bNewGraph = TRUE;

    // Create new graph
    Current = new CdlgGraph;		//add new graph before first 
    Current->Next = First;
    Current->Prev = First->Prev;
    First->Prev->Next = Current;
    First->Prev = Current;

    Current->sName = "";
    Current->pProject = pApp->First;// Current project

    Current->Color = First->Prev->Prev->Color;	//Old color
    m_nStyle += 1;								//New style
    if (m_nStyle == 5) m_nStyle = 0;
    UpdateData(FALSE);
    ChangeAxeStatus();
    OnPaint();						//Redraw

// Graph - current item
    CString ttt;
    CdlgGraph* pGraph = First;
    for (m_nGraph = 0;; m_nGraph++)		//find m_nGraph
    {
        if (pGraph == Current) break;
        pGraph = pGraph->Next;
    }
    ttt.Format("%i", m_nGraph + 1);	ttt += ". " + pGraph->sName;
    pGBox->AddString(ttt);			//add item to Graphs and AxesGraph
    pABox->AddString(ttt);
    pGBox->SetCurSel(m_nGraph);

    // Project - current item
    m_nProject = 0;					//project[0]
    pPBox->SetCurSel(m_nProject);
    UpdateData(FALSE);
    OnSelectProject();
    m_bPolar = FALSE;
    m_bFtoX = FALSE;
    UpdateData(FALSE);
    //OnPolar();
}

/////////////////////////////////////////////////////////////////////////////
// Delete current graph
void CField_1_Dlg::OnDelGraph()
{
    //Close last graph
    if (First->Next == First)
    {
        if (AfxMessageBox("Are you sure to close the last graph ?", MB_YESNO) != IDYES)
            return;
        delete First; First = NULL;	CDialog::OnOK(); //finish
    }

    //Close current graph
    else
    {
        if (Current == First) First = Current->Next;
        if (Current == AxesGraph) AxesGraph = First;

        Current->Prev->Next = Current->Next;
        Current->Next->Prev = Current->Prev;
        CdlgGraph* pGraph = Current->Next;

        delete Current;
        Current = pGraph;		//current is next

// Graphs,AxesGraph - renumbering all items
        CListBox* pGBox = (CListBox*)GetDlgItem(IDC_GRAPH);
        CComboBox* pABox = (CComboBox*)GetDlgItem(IDC_AXEGRAPH);
        pGBox->ResetContent();	//clear boxes					    
        pABox->ResetContent();

        CString ttt;	int i = 0;

        pGraph = First;
        do
        {
            ttt.Format("%i", i + 1);	ttt += ". " + pGraph->sName;
            pGBox->AddString(ttt);
            pABox->AddString(ttt);
            if (pGraph == Current)	m_nGraph = (int)i;
            if (pGraph == AxesGraph) m_nAxeGraph = (int)i;
            pGraph = pGraph->Next;	i++;
        } while (pGraph != First);

        // Graphs -  current item
        pGBox->SetCurSel(m_nGraph);
        bNewGraph = FALSE;
        ReadDataFromCurrent();	//read graph data 

// Project - current item
        CVideoApp* pApp = (CVideoApp*)AfxGetApp();
        CComboBox* pPBox = (CComboBox*)GetDlgItem(IDC_PROJECT);

        m_nProject = 0;
        CProject* pTmp = pApp->First;
        do
        {
            if (pTmp == Current->pProject) break;
            pTmp = pTmp->Next;	m_nProject++;
        } while (pTmp != pApp->First);

        pPBox->SetCurSel(m_nProject);
        UpdateData(FALSE);
        OnSelectProject();
        //OnPolar();
    }
}

/////////////////////////////////////////////////////////////////////////////
// Select new graph
void CField_1_Dlg::OnSelectGraph()
{
    CVideoApp* pApp = (CVideoApp*)AfxGetApp();
    CComboBox* pPBox = (CComboBox*)GetDlgItem(IDC_PROJECT);
    CListBox* pGBox = (CListBox*)GetDlgItem(IDC_GRAPH);
    int i, n;

    //Keep old graph number
    int	nGraphOld = m_nGraph;

    //Clear for NEW Graph
    if (bNewGraph) UpdateData();
    pGBox->SetCurSel(m_nGraph);

    //Find new graph
    n = abs(m_nGraph - nGraphOld);
    if (n != 0)
    {
        ReadDataToCurrent();            //save old graph data
        bNewGraph = FALSE;
        if (nGraphOld < m_nGraph)
            for (i = 0; i < n; i++) Current = Current->Next;
        else
            for (i = 0; i < n; i++) Current = Current->Prev;

        ReadDataFromCurrent();			//read new graph data 
    }

    // Project - current item
    m_nProject = 0;
    CProject* pTmp = pApp->First;
    do
    {
        if (pTmp == Current->pProject) break;
        pTmp = pTmp->Next;	m_nProject++;
    } while (pTmp != pApp->First);

    pPBox->SetCurSel(m_nProject);
    UpdateData(FALSE);
    OnSelectProject();
    //OnPolar();
}

/////////////////////////////////////////////////////////////////////////////
// Change Project
void CField_1_Dlg::OnSelectProject()
{
    CComboBox* pFBox = (CComboBox*)GetDlgItem(IDC_FIELD);
    int i, n;

    //Keep old project number
    int	nProjectOld = m_nProject;

    // Clear for NEW Graph
    if (bNewGraph)
    {
        UpdateData();
        m_nField = 0;					// Field [0]
    }

    //Find project
    n = abs(m_nProject - nProjectOld);
    if (nProjectOld < m_nProject)
        for (i = 0; i < n; i++) Current->pProject = Current->pProject->Next;
    else
        for (i = 0; i < n; i++) Current->pProject = Current->pProject->Prev;

    // Fields - all items
    pFBox->ResetContent();				// Clear fields
    for (i = 0; i < Current->pProject->m_nFields; i++)
        pFBox->AddString(Current->pProject->m_field[i].name + " ["
            + Current->pProject->m_field[i].size + "] ");

    // Fields - current	item
    pFBox->SetCurSel(m_nField);
    UpdateData(FALSE);
    OnFieldModify();
}

////////////////////////////////////////////////////////////////////////
// Change Field
VOID CField_1_Dlg::OnFieldModify()
{
    CComboBox* pX1Box = (CComboBox*)GetDlgItem(IDC_ARG1);
    int i;

    // Clear for NEW Graph
    if (bNewGraph)
    {
        UpdateData();
        m_nArg1 = Current->pProject->m_field[m_nField].nAxis;// F(t)

        Current->INDEX.resize(Current->pProject->m_field[m_nField].nAxis + 1);
        for (i = 0; i <= Current->pProject->m_field[m_nField].nAxis; i++)
            Current->INDEX[i] = 0;
    }

    // Arg1 - all items
    CString ttt;
    pX1Box->ResetContent();					    // Clear Arg1
    for (i = 0; i < Current->pProject->m_field[m_nField].nAxis; i++)
    {
        ttt = Current->pProject->m_field[m_nField].axe[i].name + " ["
            + Current->pProject->m_field[m_nField].axe[i].size + "] ";
        pX1Box->AddString(ttt);					// Add axes for	Arg1
    }
    ttt = Current->pProject->m_sTime + " ["
        + Current->pProject->m_sTimeSize + "] ";
    pX1Box->AddString(ttt);		  				// Add time for Arg1

//	Arg1 - current item
    pX1Box->SetCurSel(m_nArg1);
    UpdateData(FALSE);
    OnChangeArg1();
    return;
}

////////////////////////////////////////////////////////////////////////
// Change Argument 1
void CField_1_Dlg::OnChangeArg1()
{
    CComboBox* pX1Min = (CComboBox*)GetDlgItem(IDC_ARG1MIN);
    CComboBox* pX1Max = (CComboBox*)GetDlgItem(IDC_ARG1MAX);
    CListBox* pParam = (CListBox*)GetDlgItem(IDC_PARAM);

    int i;
    CString ttt1, ttt2, ttt3;

    // Clear for NEW Graph
    if (bNewGraph)
    {
        UpdateData();
        // Arg1 - current min,max
        m_nX1min = 0;		  // Min is [0] everywhere
        if (m_nArg1 < (int)Current->pProject->m_field[m_nField].nAxis)//Max for axe
            m_nX1max = Current->pProject->m_field[m_nField].axe[m_nArg1].nPoints - 1;
        else														 //Max for time
            m_nX1max = Current->pProject->m_nTimePoints - 1;
        if (m_nX1max > TabMax)m_nX1max = TabMax;
    }

    // Arg1 min,max - all items
    pX1Min->ResetContent();				// Clear Arg1Min, Arg1Max
    pX1Max->ResetContent();
    if (m_nArg1 < (int)Current->pProject->m_field[m_nField].nAxis)
        for (i = 0; i < Current->pProject->m_field[m_nField].axe[m_nArg1].nPoints; i++)
        {
            ttt1.Format("%g", Current->pProject->m_field[m_nField].axe[m_nArg1].point[i]);
            pX1Min->AddString(ttt1);	//Min,max for axe
            pX1Max->AddString(ttt1);
        } else
            for (i = 0; i < Current->pProject->m_nTimePoints; i++)
            {
                ttt1.Format("%g", Current->pProject->m_time[i]);
                pX1Min->AddString(ttt1);	//Min,max for time
                pX1Max->AddString(ttt1);
            };

        //  Parameters - all items
        pParam->ResetContent();							   //Clear Parameters
        for (i = 0; i < Current->pProject->m_field[m_nField].nAxis; i++) //Axes  Parameters
        {
            ttt1 = Current->pProject->m_field[m_nField].axe[i].name;
            ttt2 = " [" + Current->pProject->m_field[m_nField].axe[i].size + "]";
            if ((int)i == m_nArg1)				   //Axe is Arg1 
                pParam->AddString(ttt1 + ttt2 + " - AGRUMENT 1");
            else										   //Axe isn't Argument
            {
                ttt1 = ttt1 + " = ";
                ttt3.Format("%g", Current->pProject->m_field[m_nField].axe[i].point
                    [Current->INDEX[i]]);
                pParam->AddString(ttt1 + ttt3 + ttt2);
            }
        }
        ttt1 = Current->pProject->m_sTime;							   //Time Parameter
        ttt2 = " [" + Current->pProject->m_sTimeSize + "]";
        if (int(i) == m_nArg1)						   //Time is Arg1 
            pParam->AddString(ttt1 + ttt2 + " - AGRUMENT 1");
        else											   //Time isn't Argument
        {
            ttt1 = ttt1 + " = ";
            ttt3.Format("%g", Current->pProject->m_time[Current->INDEX[i]]);
            pParam->AddString(ttt1 + ttt3 + ttt2);
        }

        // Change current graph name
        if (bNewGraph)
        {
            if (m_nArg1 < (int)Current->pProject->m_field[m_nField].nAxis)
                ttt1 = Current->pProject->m_field[m_nField].name + "("
                + Current->pProject->m_field[m_nField].axe[m_nArg1].name + ")";
            else
                ttt1 = Current->pProject->m_field[m_nField].name + "("
                + Current->pProject->m_sTime + ")";

            UpdateData(FALSE);
            OnChangeGraphName(ttt1);
        }

        //	Paramert - current
        m_nParam = i;						//Parameter=time
        if (m_nArg1 == (int)i) m_nParam = 0;	//Time is Arg1; Parametr=[0]
        UpdateData(FALSE);
        OnSelectParam();
}

////////////////////////////////////////////////////////////////////////
// Change current graph name
void CField_1_Dlg::OnChangeGraphName(CString ttt)
{
    CListBox* pGBox = (CListBox*)GetDlgItem(IDC_GRAPH);
    CComboBox* pABox = (CComboBox*)GetDlgItem(IDC_AXEGRAPH);

    Current->sName = ttt;
    CString ttt1;
    ttt1.Format("%i", m_nGraph + 1);	ttt1 += ". " + ttt;

    pGBox->DeleteString(m_nGraph);
    pABox->DeleteString(m_nGraph);
    pGBox->InsertString(m_nGraph, ttt1);
    pABox->InsertString(m_nGraph, ttt1);
    UpdateData(FALSE);
}

////////////////////////////////////////////////////////////////////////
// Change parameter number												
void CField_1_Dlg::OnSelectParam()
{
    CListBox* pParam = (CListBox*)GetDlgItem(IDC_PARAM);
    CComboBox* pArg = (CComboBox*)GetDlgItem(IDC_ARGUMENT);

    int i;
    CString ttt;

    // NEW Graph
    if (bNewGraph)		UpdateData();

    // Field is 0-Dim (time only)
    if (Current->pProject->m_field[m_nField].nAxis == 0)
    {
        pArg->EnableWindow(FALSE); //Close all parameters window
        pParam->EnableWindow(FALSE);
        m_sArgName = "";
        if (bNewGraph)
        {
            m_fFmin = Current->pProject->m_field[m_nField].min;
            m_fFmax = Current->pProject->m_field[m_nField].max;
            if (m_bLog)
            {
                if (m_fFmin > 0)
                    m_fFmin = log10(m_fFmin);
                else
                    m_fFmin = -37;

                if (m_fFmax > 0)
                    m_fFmax = log10(m_fFmax);
                else
                    m_fFmax = 37;
            }
            
        }
        bNewGraph = TRUE;
        UpdateData(FALSE);
        return;
    }

    // Field isn't 0-Dim
    pArg->EnableWindow();			//Open parameters window
    pParam->EnableWindow();

    // Change Parametr if it equals to Arg1
    if (m_nParam == m_nArg1) m_nParam = (int)Current->pProject->m_field[m_nField].nAxis;
    if (m_nParam == m_nArg1) m_nParam = 0;
    pParam->SetCurSel(m_nParam);

    // Current Parameter - all items
    pArg->ResetContent();							 //Clear CurrentParameter
    if (m_nParam < (int)Current->pProject->m_field[m_nField].nAxis) //CurrentParameter - axe
    {
        for (i = 0; i < Current->pProject->m_field[m_nField].axe[m_nParam].nPoints; i++)
        {
            ttt.Format("%g", Current->pProject->m_field[m_nField].axe[m_nParam].point[i]);
            pArg->AddString(ttt);
        }
        m_sArgName = Current->pProject->m_field[m_nField].axe[m_nParam].name + " ["
            + Current->pProject->m_field[m_nField].axe[m_nParam].size + "] ";
    } else												//CurrentParameter - time
    {
        for (i = 0; i < Current->pProject->m_nTimePoints; i++)
        {
            ttt.Format("%g", Current->pProject->m_time[i]);
            pArg->AddString(ttt);
        }
        m_sArgName = Current->pProject->m_sTime + " ["
            + Current->pProject->m_sTimeSize + "] ";
    }

    //	Current Parameter - current 
    m_nArgument = Current->INDEX[m_nParam];
    pArg->SetCurSel(m_nArgument);
    UpdateData(FALSE);
    OnSelectNewArgument();
    return;
}

////////////////////////////////////////////////////////////////////////
// Change Current Parameter
void CField_1_Dlg::OnSelectNewArgument()
{
    CListBox* pParam = (CListBox*)GetDlgItem(IDC_PARAM);

    CString ttt1, ttt2, ttt3;

    // NEW Graph
    if (bNewGraph)
    {
        UpdateData();

        m_fFmin = Current->pProject->m_field[m_nField].min;
        m_fFmax = Current->pProject->m_field[m_nField].max;
        if (m_bLog)
        {
            if (m_fFmin > 0)
                m_fFmin = log10(m_fFmin);
            else
                m_fFmin = -37;

            if (m_fFmax > 0)
                m_fFmax = log10(m_fFmax);
            else
                m_fFmax = 37;
        }
    }

    bNewGraph = TRUE;
    Current->INDEX[m_nParam] = m_nArgument;

    //Prepare Parameter string
    pParam->DeleteString(m_nParam);
    if (m_nParam < (int)Current->pProject->m_field[m_nField].nAxis) //Current Parameter - axe
    {
        ttt1 = Current->pProject->m_field[m_nField].axe[m_nParam].name + " = ";
        ttt2 = " [" + Current->pProject->m_field[m_nField].axe[m_nParam].size + "]";
        ttt3.Format("%g", Current->pProject->m_field[m_nField].axe[m_nParam].point[m_nArgument]);
    } else
    {													 //Current Parameter - time
        ttt1 = Current->pProject->m_sTime + " = ";
        ttt2 = " [" + Current->pProject->m_sTimeSize + "]";
        ttt3.Format("%g", Current->pProject->m_time[m_nArgument]);
    }
    pParam->InsertString(m_nParam, ttt1 + ttt3 + ttt2);

    pParam->SetCurSel(m_nParam);
    UpdateData(FALSE);
    return;
}

////////////////////////////////////////////////////////////////////////
//	 Change any Min's and Max's. Prepare  Min < Max .
void CField_1_Dlg::OnChangeFmin()
{
    UpdateData();
    if (m_bLog)
    {
        float FminCurr = Current->pProject->m_field[m_nField].min;
        float FmaxCurr = Current->pProject->m_field[m_nField].max;
        if (FminCurr > 0)
            FminCurr = log10(FminCurr);
        else
            FminCurr = -37;

        if (FmaxCurr > 0)
            FmaxCurr = log10(FmaxCurr);
        else
            FmaxCurr = 37;

        if (m_fFmin < FminCurr)
            m_fFmin = FminCurr;
        if (m_fFmin > m_fFmax)
            m_fFmin = m_fFmax;
    }
    else
    {
        if (m_fFmin < Current->pProject->m_field[m_nField].min)
            m_fFmin = Current->pProject->m_field[m_nField].min;
        if (m_fFmin > m_fFmax)
            m_fFmin = m_fFmax;
    }
    UpdateData(FALSE);
}

void CField_1_Dlg::OnChangeFmax()
{
    UpdateData();
    if (m_bLog)
    {
        float FminCurr = Current->pProject->m_field[m_nField].min;
        float FmaxCurr = Current->pProject->m_field[m_nField].max;
        if (FminCurr > 0)
            FminCurr = log10(FminCurr);
        else
            FminCurr = -37;

        if (FmaxCurr > 0)
            FmaxCurr = log10(FmaxCurr);
        else
            FmaxCurr = 37;

        if (m_fFmax < m_fFmin)
            m_fFmax = m_fFmin;
        if (m_fFmax > FmaxCurr)
            m_fFmax = FmaxCurr;
    }
    else
    {
        if (m_fFmax < m_fFmin)
            m_fFmax = m_fFmin;
        if (m_fFmax > Current->pProject->m_field[m_nField].max)
            m_fFmax = Current->pProject->m_field[m_nField].max;
    }
    UpdateData(FALSE);
}

void CField_1_Dlg::OnSelectArg1max()
{
    UpdateData();
    if (m_nX1max <= m_nX1min) m_nX1max = m_nX1min + 1;
    m_fFmin = Current->pProject->m_field[m_nField].min;
    m_fFmax = Current->pProject->m_field[m_nField].max;
    if (m_bLog)
    {
        if (m_fFmin > 0)
            m_fFmin = log10(m_fFmin);
        else
            m_fFmin = -37;

        if (m_fFmax > 0)
            m_fFmax = log10(m_fFmax);
        else
            m_fFmax = 37;
    }
    UpdateData(FALSE);
}

void CField_1_Dlg::OnSelectArg1min()
{
    UpdateData();
    if (m_nX1min >= m_nX1max) m_nX1min = m_nX1max - 1;
    m_fFmin = Current->pProject->m_field[m_nField].min;
    m_fFmax = Current->pProject->m_field[m_nField].max;
    if (m_bLog)
    {
        if (m_fFmin > 0)
            m_fFmin = log10(m_fFmin);
        else
            m_fFmin = -37;

        if (m_fFmax > 0)
            m_fFmax = log10(m_fFmax);
        else
            m_fFmax = 37;
    }
    UpdateData(FALSE);
}

void CField_1_Dlg::OnChangeXmax()
{
    float z = m_fXmax;
    UpdateData();
    if (m_fXmax <= m_fXmin) { m_fXmax = z; UpdateData(FALSE); }
}

void CField_1_Dlg::OnChangeXmin()
{
    float z = m_fXmin;
    UpdateData();
    if (m_fXmax <= m_fXmin) { m_fXmin = z; UpdateData(FALSE); }
}

void CField_1_Dlg::OnChangeYmax()
{
    float z = m_fYmax;
    UpdateData();
    if (m_fYmax <= m_fYmin) { m_fYmax = z; UpdateData(FALSE); }
}

void CField_1_Dlg::OnChangeYmin()
{
    float z = m_fYmin;
    UpdateData();
    if (m_fYmax <= m_fYmin) { m_fYmin = z; UpdateData(FALSE); }
}

////////////////////////////////////////////////////////////////////////
// REDRAW Color rectangle
void CField_1_Dlg::OnPaint()
{
    if (bNewGraph) UpdateData();
    CPaintDC dc(this); // device context for painting	

    CStatic* pWnd = (CStatic*)GetDlgItem(IDC_COL);
    CDC* pControlDC = pWnd->GetDC();
    CRect	 pW;          pWnd->GetClientRect(&pW); //Take window size

    pWnd->Invalidate();
    pWnd->UpdateWindow();

    pControlDC->FillSolidRect(pW, RGB(255, 255, 255)); //Fill window by white color
    CPen  myPen(iStyle[m_nStyle], 1, Current->Color);	//By current style and color			
    CPen* oldPen = pControlDC->SelectObject(&myPen);

    pControlDC->MoveTo(pW.left + 2, pW.bottom - pW.Height() / 2);
    pControlDC->LineTo(pW.right - 2, pW.bottom - pW.Height() / 2);//draw line

    pControlDC->SelectObject(oldPen);
    myPen.DeleteObject();
    pWnd->ReleaseDC(pControlDC);
}

////////////////////////////////////////////////////////////////////////
// Call Color dialog
void CField_1_Dlg::OnColour()
{
    CColorDialog dlg;
    if (dlg.DoModal() == IDOK)
        Current->Color = dlg.GetColor();	//Change Color after dialog
    if (Current->Color == RGB(255, 255, 255))Current->Color = RGB(0, 0, 0);
    OnPaint();						//Redraw	
}

////////////////////////////////////////////////////////////////////////
// Change Style
void CField_1_Dlg::OnChangeStyle()
{
    OnPaint();						//Redraw	
}

////////////////////////////////////////////////////////////////////////
// Enable/Disable F<->X button (used only with polar coordinates)
/*void CField_1_Dlg::OnPolar()
{
    CButton	  *pFtoX  = (CButton   *)GetDlgItem(IDC_POLAR2);
    UpdateData();

    if(m_bPolar)
        pFtoX->EnableWindow();		//enable   F<->X button
    else
        pFtoX->EnableWindow(FALSE);	//disable  F<->X button
}
*/


void CField_1_Dlg::OnBnClickedLoadpreset()
{
    CString fName = "VideoPreset.json";
    std::ifstream source(Current->pProject->projectPath + fName);

    json root;

    if (source.is_open())
    {
        try {
            source >> root;
        }
        catch (std::exception e) {
            return;
        }
    } else {
        return;
    }
    source.close();

    CComboBox* pX1Box = (CComboBox*)GetDlgItem(IDC_ARG1);
    CComboBox* pX1Min = (CComboBox*)GetDlgItem(IDC_ARG1MIN);
    CComboBox* pX1Max = (CComboBox*)GetDlgItem(IDC_ARG1MAX);

    CListBox* pParam = (CListBox*)GetDlgItem(IDC_PARAM);
    CComboBox* pArg = (CComboBox*)GetDlgItem(IDC_ARGUMENT);


    m_nArg1 = root["OneDim"]["Arg1"]["Value"];
    pX1Box->SetCurSel(root["OneDim"]["Arg1"]["Value"].get<int>());
    OnChangeArg1();
    
    int arg1_points;
    if (m_nArg1 < (int)Current->pProject->m_field[m_nField].nAxis) {
        arg1_points = Current->pProject->m_field[m_nField].axe[m_nArg1].nPoints - 1;
    }
    else {
        arg1_points = Current->pProject->m_nTimePoints - 1;
    }

    int arg1LoadMax = root["OneDim"]["Arg1"]["Max"].get<int>();
    int arg1LoadMin = root["OneDim"]["Arg1"]["Min"].get<int>();

    int arg1_max = (arg1LoadMax <= arg1_points) ? arg1LoadMax : arg1_points;
    int arg1_min = (arg1LoadMin < arg1_points)  ? arg1LoadMin : 0;

    pX1Min->SetCurSel(arg1_min);
    pX1Max->SetCurSel(arg1_max);
    UpdateData();

    //INDEX массив, где хранятся индексы, полученные из Меню параметров (правый листбокс)
    // нагружаю в этот массив данные из файла
    for (int i = 0; i < Current->pProject->m_field[m_nField].nAxis + 1; i++) {
        Current->INDEX[i] = root["OneDim"]["MainArg"].value(std::to_string(i), 0);
    }

    //цикл, вызывающий обновление интерфейса с данными из массива INDEX
    for (int i = 0; i < Current->pProject->m_field[m_nField].nAxis + 1; i++) {
        if (i != m_nArg1) {
            pParam->SetCurSel(i);
            OnSelectParam();
        }
    }
}


void CField_1_Dlg::OnBnClickedSavepreset()
{
    json root;

    root["OneDim"]["MainArg"]["Min"] = m_fFmin;
    root["OneDim"]["MainArg"]["Max"] = m_fFmax;

    root["OneDim"]["Arg1"]["Value"] = m_nArg1;
    root["OneDim"]["Arg1"]["Min"] = m_nX1min;
    root["OneDim"]["Arg1"]["Max"] = m_nX1max;


    for (int i = 0; i < Current->pProject->m_field[m_nField].nAxis + 1; i++) {
        root["OneDim"]["MainArg"][std::to_string(i)] = Current->INDEX[i];
    }

    std::ofstream out;
    CString fName = "VideoPreset.json";


    //подкачка из файла значений для двумерных графиков
    std::ifstream source(Current->pProject->projectPath + fName);
    json tmpRoot;
    if (source.is_open())
    {
        try {
            source >> tmpRoot;
        }
        catch (std::exception e) {
            tmpRoot["TwoDim"];
        }
    } else {
        return;
    }
    source.close();

    root["TwoDim"] = tmpRoot["TwoDim"];


    //сохранение
    out.open(Current->pProject->projectPath + fName);
    if (out.is_open())
    {
        out << root << std::endl;
    }
    out.close();
}



void CField_1_Dlg::OnBnClickedLog()
{
    UpdateData();

    if (m_bLog)
    {
        if (m_fFmax > 0)
            m_fFmax = log10(m_fFmax);
        else
            m_fFmax = -37;

        if (m_fFmin > 0)
            m_fFmin = log10(m_fFmin);
        else
            m_fFmin = -37;

    }
    else
    {
        m_fFmax = pow(10, m_fFmax);
        m_fFmin = pow(10, m_fFmin);
    }
    UpdateData(FALSE);
    // TODO: добавьте свой код обработчика уведомлений
}
