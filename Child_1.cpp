/////////////////////////////////////////////////////////////////////////////
// Child_1.cpp : implementation of the CChild_1_Frame class
//

#include "stdafx.h"

#include <fstream>
#include <math.h>
#include <cfloat>

#include "video.h"
#include "MainFrm.h"
#include "F_1_dlg.h"
#include "Child_1.h"

#include "windows.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChild_1_Frame
IMPLEMENT_DYNCREATE(CChild_1_Frame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChild_1_Frame, CMDIChildWnd)
    //{{AFX_MSG_MAP(CChild_1_Frame)
    ON_COMMAND(ID_GRAPH_MODIFY, OnGraphModify)
    ON_COMMAND(ID_FILE_SAVEDATA, OnFileSavedata)
    //ON_COMMAND(ID_FILE_SAVEPICT, OnFileSavepict)
    ON_WM_KILLFOCUS()
    ON_WM_RBUTTONDOWN()
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_COMMAND(IDD_SAVE_WMF, OnSaveWmf)
    ON_COMMAND(ID_RESIZE, OnResize)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChild_1_Frame construction/destruction
CChild_1_Frame::CChild_1_Frame() //standart constructor - not used
{
    PicsTotal = 0;
}

// CChild_1_Frame main constructor
CChild_1_Frame::CChild_1_Frame(CField_1_Dlg* pDlg)
{
    AxesGraph = Current = First = NULL;
    CMainFrame* pWnd = (CMainFrame*)AfxGetMainWnd();
    MainDir = pWnd->MainDir;
    SetData(pDlg);
    PrepareData();
}

CGraph* sessionToCGraph(VideoSession1Dim& s) {

    auto g = new CGraph;

    g->pProject = s.pProject;
    g->nField = s.nField;
    g->bPolar = s.bPolar;
    g->fFmin = s.fFmin;
    g->fFmax = s.fFmax;
    g->nArg1 = s.nArg1;
    g->nX1min = s.nX1min;
    g->nX1max = s.nX1max;
    g->Color = s.Color;
    g->Style = s.Style;
    g->bGridL = s.bGridL;
    g->sName = s.sName.c_str();
    g->bLog = s.bLog;
    g->Color = s.Color;

    g->INDEX.resize(s.pProject->m_field[s.nField].nAxis + 1);
    for (int i = 0; i <= s.pProject->m_field[s.nField].nAxis; i++) {
        g->INDEX[i] = s.INDEX[i];
    }

    return g;
}


CChild_1_Frame::CChild_1_Frame(OneDimWindowDto& sessions) {
    

    // Из VideoSession1Dim считываем в текущий класс CGraph и метаданные из окна (параметры для осей 6 штук), их нужно сохранить в json.
    // AxesGraph нужен для выбора значений осей из одного из графиков
    // вызываем основную логику из prepare data 250

    First = sessionToCGraph(sessions.graphs[0]);
    Current = First;
    AxesGraph = First;

    for (int i = 1; i < sessions.graphs.size(); i++) {

        auto tmp = sessionToCGraph(sessions.graphs[i]);
        Current->Next = tmp;
        tmp->Prev = Current;
        Current = tmp;
    }

    Current->Next = First;
    First->Prev = Current;
    Current = First;

    AxeStatus = sessions.AxeStatus;
    fX1min = sessions.fX1min;
    fX1max = sessions.fX1max;
    fX2min = sessions.fX2min;
    fX2max = sessions.fX2max;

    PrepareData();

}

CChild_1_Frame::~CChild_1_Frame()
{
    if (First)
    {
        Current = First;
        while (Current->Next != First)
        {
            Current = Current->Next;
            delete Current->Prev;
        }
        delete Current;
    }
}

BOOL CChild_1_Frame::PreCreateWindow(CREATESTRUCT& cs)
{
    return CMDIChildWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CChild_1_Frame diagnostics
#ifdef _DEBUG
void CChild_1_Frame::AssertValid() const
{
    CMDIChildWnd::AssertValid();
}

void CChild_1_Frame::Dump(CDumpContext& dc) const
{
    CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG


void CChild_1_Frame::SetData(CField_1_Dlg* pDlg) {
    // Take data from CField_1_Dlg	
    CdlgGraph* pDlgGraph = pDlg->First;
    CGraph* pGraph, * pTmp = NULL;
    do
    {
        pGraph = new CGraph;
        pGraph->Prev = pTmp;
        if (pTmp) pTmp->Next = pGraph;

        if (pDlg->First == pDlgGraph) First = pGraph;
        if (pDlg->Current == pDlgGraph) Current = pGraph;
        if (pDlg->AxesGraph == pDlgGraph) AxesGraph = pGraph;

        pGraph->pProject = pDlgGraph->pProject;
        pGraph->nField = pDlgGraph->nField;
        pGraph->bPolar = pDlgGraph->bPolar;
        pGraph->bFtoX = pDlgGraph->bFtoX;
        pGraph->fFmin = pDlgGraph->fFmin;
        pGraph->fFmax = pDlgGraph->fFmax;
        pGraph->nArg1 = pDlgGraph->nArg1;
        pGraph->nX1min = pDlgGraph->nX1min;
        pGraph->nX1max = pDlgGraph->nX1max;
        pGraph->Color = pDlgGraph->Color;
        pGraph->Style = pDlgGraph->Style;
        pGraph->bGridL = pDlgGraph->bGridL;
        pGraph->sName = pDlgGraph->sName;
        pGraph->bLog = pDlgGraph->bLog;

        pGraph->INDEX.resize(pGraph->pProject->m_field[pGraph->nField].nAxis + 1);

        for (int i = 0; i <= pGraph->pProject->m_field[pGraph->nField].nAxis; i++) {
            if ((int)i == pGraph->nArg1) {
                pGraph->INDEX[i] = pGraph->nX1min;
            }
            else {
                pGraph->INDEX[i] = pDlgGraph->INDEX[i];
            }
        }

        pTmp = pGraph;
        pDlgGraph = pDlgGraph->Next;
    } while (pDlgGraph != pDlg->First);

    First->Prev = pTmp;
    pTmp->Next = First;

    //Prepare standart axes
    AxeStatus = pDlg->m_nAxeStatus;

    if (AxeStatus == 0) {
        fX1min = pDlg->m_fXmin;
        fX1max = pDlg->m_fXmax;
        fX2min = pDlg->m_fYmin;
        fX2max = pDlg->m_fYmax;
    }

}


/////////////////////////////////////////////////////////////////////////////
// PrepareData - Main CChild_1_Frame procedure:
//		preparing all data from dialog;
//		change window name;
//		find function values;
//		recalculate function Min and Max;
//		recalculate min,max for draw axes.
void CChild_1_Frame::PrepareData()
{
    int i;
    CGraph* pGraph = NULL;
    

    //Window name (begin)
    if (First == First->Next)	sName = "Graph  ";
    else						sName = "Graphs  ";

    //For every graph
    /////////////////////////////////////////////////////////////////////////////
    pGraph = First;
    do
    {
        //Window name (continue)
        sName += pGraph->sName + ", ";

        //Prepare Function values
        int j, jst;		int l;

        pGraph->nX1 = pGraph->nX1max - pGraph->nX1min + 1;
        pGraph->Fun.resize(pGraph->nX1);
        pGraph->X1.resize(pGraph->nX1);

        pGraph->sFName = pGraph->pProject->m_field[pGraph->nField].name + "["
            + pGraph->pProject->m_field[pGraph->nField].size + "]";

        //Fun(t)
        if (pGraph->nArg1 == (int)pGraph->pProject->m_field[pGraph->nField].nAxis)
        {
            pGraph->sX1Name = pGraph->pProject->m_sTime + "["
                + pGraph->pProject->m_sTimeSize + "]";
            if (pGraph->pProject->m_field[pGraph->nField].nAxis != 0)
            {
                j = pGraph->INDEX[pGraph->pProject->m_field[pGraph->nField].nAxis - 1];
                for (l = pGraph->pProject->m_field[pGraph->nField].nAxis - 2; l >= 0; l--)
                    j = pGraph->INDEX[l] + j * pGraph->pProject->m_field[pGraph->nField].axe[l].nPoints;
            } else
                j = 0;

            j += pGraph->nX1min * pGraph->pProject->m_field[pGraph->nField].nPoints;

            pGraph->pProject->read_point(
                pGraph->nField,
                j,
                pGraph->pProject->m_field[pGraph->nField].nPoints,
                pGraph->nX1,
                pGraph->Fun,
                pGraph->bLog
            );

            for (l = pGraph->nX1min; l <= pGraph->nX1max; l++)
                pGraph->X1[l - pGraph->nX1min] = pGraph->pProject->m_time[l];
        }
        //Fun(x)
        else
        {
            pGraph->sX1Name = pGraph->pProject->m_field[pGraph->nField].axe[pGraph->nArg1].name + "["
                + pGraph->pProject->m_field[pGraph->nField].axe[pGraph->nArg1].size + "]";

            j = pGraph->INDEX[pGraph->pProject->m_field[pGraph->nField].nAxis - 1];
            for (l = pGraph->pProject->m_field[pGraph->nField].nAxis - 2; l >= 0; l--)
                j = pGraph->INDEX[l] + j * pGraph->pProject->m_field[pGraph->nField].axe[l].nPoints;

            j += pGraph->INDEX[pGraph->pProject->m_field[pGraph->nField].nAxis] *
                pGraph->pProject->m_field[pGraph->nField].nPoints;

            jst = 1;
            for (l = 0; l < pGraph->nArg1; l++)
                jst *= pGraph->pProject->m_field[pGraph->nField].axe[l].nPoints;

            pGraph->pProject->read_point(
                pGraph->nField,
                j,
                jst,
                pGraph->nX1,
                pGraph->Fun,
                pGraph->bLog
            );
            for (l = pGraph->nX1min; l <= pGraph->nX1max; l++)
                pGraph->X1[l - pGraph->nX1min] = pGraph->pProject->m_field[pGraph->nField].axe[pGraph->nArg1].point[l];
        }

        //Recalculate fMin,fMax
        float fMin = FLT_MAX;
        float fMax = -FLT_MAX;
        if (pGraph->bLog)
        {
            fMin = 37;
            fMax = -37;
        }
        for (j = 0; j < pGraph->nX1; j++)
        {
            if (pGraph->Fun[j] > fMax) fMax = pGraph->Fun[j];
            if (pGraph->Fun[j] < fMin) fMin = pGraph->Fun[j];
        }
        if (pGraph->fFmax > fMax) pGraph->fFmax = fMax;
        if (pGraph->fFmin < fMin) pGraph->fFmin = fMin;

        //Prepare Polar coordinates
        //-------------------------
        if (pGraph->bPolar)
        {
            CString ttt;
            if (pGraph->bFtoX)		//Change X and F
            {
                auto Temp = pGraph->Fun;
                pGraph->Fun = pGraph->X1;
                pGraph->X1 = Temp;
                ttt = pGraph->sX1Name;
                pGraph->sX1Name = pGraph->sFName;
                pGraph->sFName = ttt;
            }

            ttt = pGraph->sFName + "*COS(" + pGraph->sX1Name + ")";
            pGraph->sFName = pGraph->sFName + "*SIN(" + pGraph->sX1Name + ")";
            pGraph->sX1Name = ttt;   // Change axes names

            float   x1, x2;
            pGraph->fX1min = pGraph->fX2min = FLT_MAX;
            pGraph->fX1max = pGraph->fX2max = -FLT_MAX;

            j = 0;
            for (i = 0; i < pGraph->nX1; i++)
                if ((
                    (!pGraph->bFtoX) &&
                    (pGraph->Fun[i] >= pGraph->fFmin) &&
                    (pGraph->Fun[i] <= pGraph->fFmax)
                    )
                    ||
                    (
                        (pGraph->bFtoX) &&
                        (pGraph->X1[i] >= pGraph->fFmin) &&
                        (pGraph->X1[i] <= pGraph->fFmax)
                        )

                    )
                {
                    x1 = pGraph->Fun[i] * (float)cos(pGraph->X1[i]);//X1-argument
                    if (x1 > pGraph->fX1max) pGraph->fX1max = x1;
                    if (x1 < pGraph->fX1min) pGraph->fX1min = x1;
                    x2 = pGraph->Fun[i] * (float)sin(pGraph->X1[i]);//Fun-value
                    if (x2 > pGraph->fX2max) pGraph->fX2max = x2;
                    if (x2 < pGraph->fX2min) pGraph->fX2min = x2;
                    pGraph->X1[j] = x1;
                    pGraph->Fun[j] = x2;
                    j++;
                }
            pGraph->nX1 = j;
        }

        //Prepare decart coordinates
        //--------------------------
        else
        {
            //Calculate X1 min,max
            if (pGraph->nArg1 == (int)pGraph->pProject->m_field[pGraph->nField].nAxis)	//Fun(t)
            {
                pGraph->fX1max = pGraph->pProject->m_time[pGraph->nX1max];
                pGraph->fX1min = pGraph->pProject->m_time[pGraph->nX1min];
            } else																	//Fun(x)
            {
                pGraph->fX1max = pGraph->pProject->m_field[pGraph->nField].axe[pGraph->nArg1].point[pGraph->nX1max];
                pGraph->fX1min = pGraph->pProject->m_field[pGraph->nField].axe[pGraph->nArg1].point[pGraph->nX1min];
            }

            //Calculate X2 min,max
            pGraph->fX2max = pGraph->fFmax;
            pGraph->fX2min = pGraph->fFmin;
        }

        //Recalculate X1,X2 min,max for draw
        float dx;
        if (pGraph->fX1max <= pGraph->fX1min) pGraph->fX1max = pGraph->fX1min + 1;
        dx = (float)0.1 * Roundation(pGraph->fX1max - pGraph->fX1min);
        pGraph->fX1max = (Divide(pGraph->fX1max, dx) + 1) * dx;
        pGraph->fX1min = Divide(pGraph->fX1min, dx) * dx;

        if (pGraph->fX2max <= pGraph->fX2min) pGraph->fX2max = pGraph->fX2min + 1;
        if (pGraph->fX2max - pGraph->fX2min == 0) dx = (float)0.1 * Roundation(pGraph->fX2max);
        else dx = (float)0.1 * Roundation(pGraph->fX2max - pGraph->fX2min);
        pGraph->fX2max = (Divide(pGraph->fX2max, dx) + 1) * dx;
        if (!pGraph->bLog)
        {
            pGraph->fX2min = Divide(pGraph->fX2min, dx) * dx;
        }

        //End graph circle
        /////////////////////////////////////////////////////////////////////////////
        pGraph = pGraph->Next;
    } while (pGraph != First);

    //Window name (end)
    sName.SetAt(sName.GetLength() - 2, '.');
    
    switch (AxeStatus)
    {
    case 0:		//Fixed axes
    {
        /*fX1min = pDlg->m_fXmin;
        fX1max = pDlg->m_fXmax;
        fX2min = pDlg->m_fYmin;
        fX2max = pDlg->m_fYmax;*/
    } break;
    case 1:		//Unfixed axes
        break;
    case 2:		//Axes from AxesGraph
    {
        fX1min = AxesGraph->fX1min;
        fX1max = AxesGraph->fX1max;
        fX2min = AxesGraph->fX2min;
        fX2max = AxesGraph->fX2max;
    } break;
    }

}

/////////////////////////////////////////////////////////////////////////////
// CChild_1_Frame message handlers
void CChild_1_Frame::OnKillFocus(CWnd* pNewWnd)
{
    Invalidate();//redraw
    CMDIChildWnd::OnKillFocus(pNewWnd);

    // Clearning status bar
    CMainFrame* pWnd = (CMainFrame*)AfxGetMainWnd();
    pWnd->pStatusBar->SetText("", 1, 0);
    pWnd->pStatusBar->SetText("", 2, 0);
    pWnd->pStatusBar->SetText("", 3, 0);
}


VideoSession1Dim* CGraph::getSession(std::vector<CProject*> &projects) {
    auto s = new VideoSession1Dim;

    s->nField = this->nField;
    s->bPolar = this->bPolar;
    s->fFmin = this->fFmin;
    s->fFmax = this->fFmax;
    s->nArg1 = this->nArg1;
    s->nX1min = this->nX1min;
    s->nX1max = this->nX1max;
    s->Color = this->Color;
    s->Style = this->Style;
    s->bGridL = this->bGridL;
    s->bLog = this->bLog;
    s->sName = this->sName;
    s->INDEX = this->INDEX;
    s->pProject = this->pProject;

    s->projectIndex = std::find(projects.begin(), projects.end(), this->pProject) - projects.begin();

    return s;
}

OneDimWindowDto CChild_1_Frame::GetSessions(std::vector<CProject*> &projects) {


    OneDimWindowDto sessions;

    // Take data from CField_1_Dlg	
    CGraph* first = this->First;
    CGraph* current = first;
    do
    {
        auto session = current->getSession(projects);
        sessions.graphs.push_back(*session);
        current = current->Next;

    } while (current != first);

    sessions.fX1min = fX1min;
    sessions.fX1max = fX1max;
    sessions.fX2min = fX2min;
    sessions.fX2max = fX2max;
    sessions.AxeStatus = AxeStatus;

    return sessions;
}

/////////////////////////////////////////////////////////////////////////////
// OnGraphModify
void CChild_1_Frame::OnGraphModify()
{
    CField_1_Dlg dlg1;
    dlg1.bNewGraph = FALSE;

    //Restore values for CField_1_Dlg
    CdlgGraph* pDlgGraph, * pTmp = NULL;
    CGraph* pGraph = First;
    int i;
    do
    {
        pDlgGraph = new CdlgGraph;
        pDlgGraph->Prev = pTmp;
        if (pTmp) pTmp->Next = pDlgGraph;

        if (First == pGraph) dlg1.First = pDlgGraph;
        if (Current == pGraph) dlg1.Current = pDlgGraph;
        if (AxesGraph == pGraph) dlg1.AxesGraph = pDlgGraph;

        pDlgGraph->pProject = pGraph->pProject;
        pDlgGraph->nField = pGraph->nField;
        pDlgGraph->bPolar = pGraph->bPolar;
        pDlgGraph->bFtoX = pGraph->bFtoX;
        pDlgGraph->fFmin = pGraph->fFmin;
        pDlgGraph->fFmax = pGraph->fFmax;
        pDlgGraph->nArg1 = pGraph->nArg1;
        pDlgGraph->nX1min = pGraph->nX1min;
        pDlgGraph->nX1max = pGraph->nX1max;
        pDlgGraph->Color = pGraph->Color;
        pDlgGraph->Style = pGraph->Style;
        pDlgGraph->bGridL = pGraph->bGridL;
        pDlgGraph->sName = pGraph->sName;
        pDlgGraph->bLog = pGraph->bLog;

        pDlgGraph->INDEX.resize(pGraph->pProject->m_field[pGraph->nField].nAxis + 1);
        for (i = 0; i <= pGraph->pProject->m_field[pGraph->nField].nAxis; i++) {
            pDlgGraph->INDEX[i] = pGraph->INDEX[i];
        }

        pTmp = pDlgGraph;
        pGraph = pGraph->Next;
    } while (pGraph != First);

    dlg1.First->Prev = pTmp;
    pTmp->Next = dlg1.First;

    //Prepare standart axes
    dlg1.m_nAxeStatus = AxeStatus;
    if (AxeStatus == 0)	//fixed axes
    {
        dlg1.m_fXmin = fX1min;
        dlg1.m_fXmax = fX1max;
        dlg1.m_fYmin = fX2min;
        dlg1.m_fYmax = fX2max;
    }

    //Do dialog
    if (dlg1.DoModal() == IDCANCEL) return;	//no changes 
    if (dlg1.First == NULL) { this->SendMessage(WM_CLOSE); return; }	//finish

//Change current values
    if (First)			//delete old values
    {
        Current = First;
        while (Current->Next != First)
        {
            Current = Current->Next;
            delete Current->Prev;
        }
        delete Current;
    }
    SetData(&dlg1);
    PrepareData(); //take new values
    SetWindowText(sName);
    Invalidate();		//redraw
}

/////////////////////////////////////////////////////////////////////////////
// Call Graph-modify dialog by Rigth mouse button
void CChild_1_Frame::OnRButtonDown(UINT nFlags, CPoint point)
{
    CChild_1_Frame::OnGraphModify();
}

/////////////////////////////////////////////////////////////////////////////
// Save data
void CChild_1_Frame::OnFileSavedata()
{
    CFileDialog dlg(FALSE, "dat", "E", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "Data files (*.dat)|*.dat||", this);
    dlg.m_ofn.lpstrInitialDir = MainDir;
    if (dlg.DoModal() != IDOK)return;	//SAVE AS dialog

    std::fstream file;	file.open(dlg.GetPathName(), std::ios::out);
    int i;
    CGraph* pGraph = First;
    do
    {
        file << pGraph->sX1Name << "  " << pGraph->sFName << "\n";	//title
        for (i = 0; i < pGraph->nX1; i++)
            //if ((pGraph->Fun[i] >= fX2min) && (pGraph->Fun[i] <= fX2max)
            //    && (pGraph->X1[i] >= fX1min) && (pGraph->X1[i] <= fX1max))
                file << pGraph->X1[i] << "  " << pGraph->Fun[i] << "\n";//data
        file << "\n";
        pGraph = pGraph->Next;
    } while (pGraph != First);

    file.close();
}

/////////////////////////////////////////////////////////////////////////////
// Save Picture - NOT READY
void CChild_1_Frame::OnFileSavepict()
{
    CFileDialog dlg(FALSE, "bmp", sName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "Pictures (*.bmp)|*.bmp||", this);
    dlg.m_ofn.lpstrInitialDir = MainDir;
    AfxMessageBox(MainDir);
    if (dlg.DoModal() != IDOK)return;	//SAVE AS dialog

    CFile file;
    file.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite);
    //	file.Write(..................);
    file.Close();
}

/////////////////////////////////////////////////////////////////////////////
// Draw the graph
/////////////////////////////////////////////////////////////////////////////
void CChild_1_Frame::OnPaint()
{
    const int wMin = 80, hMin = 40;
    CPaintDC dc(this);							// device context for painting
    CRect       WinRec;	GetClientRect(WinRec);	// window size	

//Calculate graph rectangle
    CRect 		GrafRec;
    GrafRec.left = 90; GrafRec.right = WinRec.right - 100;
    if (GrafRec.Width() < wMin) return;
    GrafRec.top = 30; GrafRec.bottom = WinRec.bottom - 30;
    if (GrafRec.Height() < hMin) return;

    //Prepare for unfixed axes
    if (AxeStatus == 1)		  //Unfixed axes
    {
        fX1max = Current->fX1max;
        fX1min = Current->fX1min;
        fX2max = Current->fX2max;
        fX2min = Current->fX2min;
    }

    //Scale calculation
    float mX1, mX2;
    
    mX1 = GrafRec.Width() / (fX1max - fX1min);
    mX2 = GrafRec.Height() / (fX2max - fX2min);
    

    //Draw axis lines
    dc.MoveTo(GrafRec.left, GrafRec.top);
    dc.LineTo(GrafRec.left, GrafRec.bottom);
    dc.LineTo(GrafRec.right, GrafRec.bottom);
    CString ttt;
    int j;
    CSize sSize;

    float hX;
    int nX;
    float X;
    float dd;
    //Draw X2-axe 
    if(Current->bLog)
        dc.TextOut(5, 0,"lg" + Current->sFName);						//X2-axe name
    else
        dc.TextOut(5, 0,Current->sFName);
    nX = GrafRec.Height() / hMin;
    hX;
    hX = (fX2max - fX2min) / nX;
    dd = 10 * Roundation(hX);
    hX = (Divide(hX, dd) + 1) * dd;
    X = Divide(Current->fX2min, hX) * hX;
    hX /= 2;
    for (; X <= Current->fX2max; )
    {
        if ((X >= fX2min) && (X <= fX2max))
        {
            ttt.Format("%g", X);	sSize = dc.GetTextExtent(ttt);
            j = GrafRec.bottom - (int)((X - fX2min) * mX2);
            dc.TextOut(GrafRec.left - 15 - sSize.cx, j - 8, ttt);	//X2-labels
            dc.MoveTo(GrafRec.left - 10, j);
            dc.LineTo(GrafRec.left, j);					//X2-main marks
            if (Current->bGridL) dc.LineTo(GrafRec.right, j);	//X2-grid lines
        }
        X += hX;
        if ((X >= fX2min) && (X <= fX2max))
        {
            j = GrafRec.bottom - (int)((X - fX2min) * mX2);
            dc.MoveTo(GrafRec.left - 5, j);
            dc.LineTo(GrafRec.left, j);					//X2-short marks
        }
        X += hX;  if (fabs(X) < 0.01 * hX) X = (float)0;
    }
    //Draw X1-axe 
    sSize = dc.GetTextExtent(Current->sX1Name);				//X1-axe name
    dc.TextOut(WinRec.right - sSize.cx - 20, GrafRec.bottom, Current->sX1Name);
    nX = GrafRec.Width() / wMin;
    hX = (fX1max - fX1min) / nX;
    dd = 10 * Roundation(hX);
    hX = (Divide(hX, dd) + 1) * dd;
    X = Divide(Current->fX1min, hX) * hX;
    hX /= 2;
    for (; X <= Current->fX1max; )
    {
        if ((X >= fX1min) && (X <= fX1max))
        {
            ttt.Format("%g", X);		sSize = dc.GetTextExtent(ttt);
            j = GrafRec.left + (int)((X - fX1min) * mX1);
            dc.TextOut(j - sSize.cx / 2, GrafRec.bottom + 10, ttt);	//X1-labels
            dc.MoveTo(j, GrafRec.bottom + 10);
            dc.LineTo(j, GrafRec.bottom);					//X1-main marks
            if (Current->bGridL) dc.LineTo(j, GrafRec.top);	//X1-grid lines				
        }
        X += hX;
        if ((X >= fX1min) && (X <= fX1max))
        {
            j = GrafRec.left + (int)((X - fX1min) * mX1);
            dc.MoveTo(j, GrafRec.bottom + 5);
            dc.LineTo(j, GrafRec.bottom);					//X1-short marks
        }
        X += hX; if (fabs(X) < 0.01 * hX) X = (float)0;
    }

    ///////////////////////////////////////////////////////////////////////
    //Draw ONE-Dim graph
    CPen myPen(PS_SOLID, 1, First->Color);				//change pen to Color-pen
    CPen* oldPen = dc.SelectObject(&myPen);

    CGraph* pGraph = First;
    CPoint	Point;
    int i, k = 0;
    do
    {
        //Prepare pen
        dc.SelectObject(oldPen);
        myPen.DeleteObject();
        myPen.CreatePen(iStyle[pGraph->Style], 1, pGraph->Color);
        dc.SelectObject(&myPen);

        //Draw legend
        if (First != First->Next)  //more then one graph exist
        {
            i = GrafRec.top + (int)((k + 0.5) * hMin);
            if ((int)i <= GrafRec.bottom - hMin)
            {
                ttt.Format("%i", k + 1);	ttt += ". " + pGraph->sName;
                if (pGraph == Current)
                {
                    sSize = dc.GetTextExtent(ttt);
                    dc.FillSolidRect(GrafRec.right + 5, i + 4,
                        WinRec.right - (GrafRec.right + 5), sSize.cy + 10, RGB(192, 192, 192));
                    //draw gray rectangle for current graph
                    dc.TextOut(GrafRec.right + 10, i + 10, ttt);	//current graph name
                    dc.SetBkColor(RGB(255, 255, 255));		//restore background color
                } else
                    dc.TextOut(GrafRec.right + 10, i + 10, ttt);	//graph name

                dc.MoveTo(WinRec.right, i);
                dc.LineTo(GrafRec.right, i);					//graph line
            }
            k++;
        }

        //Prepare for unfixed axes
        if (AxeStatus == 1)		  //Unfixed axes
        {
            fX1max = pGraph->fX1max;
            fX1min = pGraph->fX1min;
            fX2max = pGraph->fX2max;
            fX2min = pGraph->fX2min;
            mX1 = GrafRec.Width() / (fX1max - fX1min);	//change scale
            mX2 = GrafRec.Height() / (fX2max - fX2min);
        }

        //Draw all points
            //	pPoint = new CPoint[pGraph->nX1];
        BOOL bRead = FALSE;
        //	int j=0;
            for (i = 0; i < pGraph->nX1; i++)
                if ((pGraph->Fun[i] >= fX2min) && (pGraph->Fun[i] <= fX2max)
                    && (pGraph->X1[i] >= fX1min) && (pGraph->X1[i] <= fX1max))
                {
                    Point.x = GrafRec.left + (int)(mX1 * (pGraph->X1[i] - fX1min));
                    Point.y = GrafRec.bottom - (int)(mX2 * (pGraph->Fun[i] - fX2min));
                    if (bRead)    dc.LineTo(Point);				//draw  line
                    else { dc.MoveTo(Point); bRead = TRUE; }//first point

        //				j++;			//add points to line
                }
        //		dc.Polyline(pPoint,j);	//draw line
        //		delete [] pPoint;

        pGraph = pGraph->Next;
    } while (pGraph != First);

    dc.SelectObject(oldPen);
    myPen.DeleteObject();
}

/////////////////////////////////////////////////////////////////////////////
// Draw nearest graph point or change current graph on Left mouse button down
void CChild_1_Frame::OnLButtonDown(UINT nFlags, CPoint point)
{
    Invalidate();		//Redraw
    const int wMin = 80, hMin = 40;
    CPaintDC dc(this);	//device context for painting

//Clear status bar
    CMainFrame* pWnd = (CMainFrame*)AfxGetMainWnd();
    pWnd->pStatusBar->SetText("", 1, 0);
    pWnd->pStatusBar->SetText("", 2, 0);
    pWnd->pStatusBar->SetText("", 3, 0);

    //Calculate graph rectangle
    CRect       WinRec;	GetClientRect(WinRec);	// window size	
    CRect 		GrafRec;
    GrafRec.left = 90; GrafRec.right = WinRec.right - 100;
    if (GrafRec.Width() < wMin) return;
    GrafRec.top = 30; GrafRec.bottom = WinRec.bottom - 30;
    if (GrafRec.Height() < hMin) return;

    //Prepare for unfixed axes
    if (AxeStatus == 1)		  //Unfixed axes
    {
        fX1max = Current->fX1max;
        fX1min = Current->fX1min;
        fX2max = Current->fX2max;
        fX2min = Current->fX2min;
    }
    //Scale calculation
    float mX1 = GrafRec.Width() / (fX1max - fX1min);
    float mX2 = GrafRec.Height() / (fX2max - fX2min);

    //////////////////////////////////////////////////////
    //Find and drow nearest point 
    int i, j;
    if (GrafRec.PtInRect(point))
    {
        float x1;
        CString ttt;
        //Find point x1 (in phisical coorginates)
        x1 = fX1min + (point.x - GrafRec.left) / mX1;

        if (!Current->bPolar) // for decart coordinates	
        {
            //Find nearest X1[i] point
            for (i = 0; i < Current->nX1; i++) if (Current->X1[i] > x1) break; i--;
            if (i < 0)   i = 0;
            else if (((i + 1) < Current->nX1) && ((Current->X1[i + 1] - x1) < (x1 - Current->X1[i]))) i++;
        } else					 // for polar coordinates
        {
            //Find point x2 (in phisical coorginates)
            float x2 = fX2min + (GrafRec.bottom - point.y) / mX2;
            //Find nearest X1[i],Fun[i] point
            i = 0;
            double delta = sqrt((Current->X1[i] - x1) * (Current->X1[i] - x1)
                + (Current->Fun[i] - x2) * (Current->Fun[i] - x2));
            for (j = 1; j < Current->nX1; j++)
            {
                double delta_n = sqrt((Current->X1[j] - x1) * (Current->X1[j] - x1)
                    + (Current->Fun[j] - x2) * (Current->Fun[j] - x2));
                if (delta_n < delta) { i = j; delta = delta_n; }
            }
        }

        //Prepare X1 status bar sell (1)
        ttt.Format("%g", Current->X1[i]);
        if (Current->sX1Name.GetLength() <= 8)
            pWnd->pStatusBar->SetText(Current->sX1Name + ": " + ttt, 1, 0);
        else
            pWnd->pStatusBar->SetText(ttt, 1, 0);
        //Prepare Fun status bar sell (2)
        ttt.Format("%g", Current->Fun[i]);
        if (Current->sFName.GetLength() <= 8)
            pWnd->pStatusBar->SetText(Current->sFName + ": " + ttt, 2, 0);
        else
            pWnd->pStatusBar->SetText(ttt, 2, 0);

        //Find nearest graph point (i,j)
        j = GrafRec.bottom - (int)(mX2 * (Current->Fun[i] - fX2min));
        i = GrafRec.left + (int)(mX1 * (Current->X1[i] - fX1min));
        //Draw nearest point
        dc.Ellipse(i - 5, j - 5, i + 5, j + 5);
        dc.MoveTo(GrafRec.left, j);
        dc.LineTo(i, j);
        dc.LineTo(i, GrafRec.bottom);
        Invalidate(FALSE);	//don't redraw
        return;
    }

    //////////////////////////////////////////////////////
    //Change current graph
    if ((point.x > GrafRec.right) && (point.y < GrafRec.bottom - hMin))
    {
        Current = First;
        for (j = 0; ; j++)
        {
            i = GrafRec.top + (int)((j + 0.5) * hMin);
            if ((int)i > point.y) break;
            Current = Current->Next;
            if (Current == First)  break;
        }
        if (j != 0) Current = Current->Prev;
    }
    Invalidate();//redraw
}

void CChild_1_Frame::OnSaveWmf()
{
    CRect       WinRec;	GetClientRect(WinRec);	// window size	
    CDC* pDC = GetDC();


    CDC* cdcscreen = GetDC();
    HDC hdcscreen = cdcscreen->m_hDC, hdc = CreateCompatibleDC(hdcscreen); ReleaseDC(cdcscreen);
    BITMAPINFO bi; ZeroMemory(&bi, sizeof(bi)); BITMAPINFOHEADER& bih = bi.bmiHeader;
    bih.biSize = sizeof(bih);
    bih.biWidth = WinRec.Width();
    bih.biHeight = WinRec.Height();
    bih.biPlanes = 1;
    bih.biBitCount = 32;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = ((bih.biWidth * bih.biBitCount / 8 + 3) & 0xFFFFFFFC) * bih.biHeight;
    bih.biXPelsPerMeter = 10000;
    bih.biYPelsPerMeter = 10000;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;
    void* bits; HBITMAP hbm = CreateDIBSection(hdc, (BITMAPINFO*)&bih, DIB_RGB_COLORS, &bits, NULL, NULL);


    CDC memDC;
    CBitmap MemBit;
    memDC.CreateCompatibleDC(pDC);

    //MemBit.CreateCompatibleBitmap(pDC, GrafRec.Width(), GrafRec.Height());
    //oldB = memDC.SelectObject(&MemBit);
    memDC.SelectObject(hbm);

    memDC.BitBlt(0, 0, WinRec.Width(), WinRec.Height(),
        pDC, 0, 0, SRCCOPY);


    PBITMAPINFO info = CreateBitmapInfoStruct(NULL, hbm);

    CString name, nm;
    nm = "1-dim";
    nm.Replace('|', ' ');
    nm.Replace(',', ' ');
    nm.Replace('.', ' ');


    CMainFrame* pWnd = (CMainFrame*)AfxGetMainWnd();
    MainDir = pWnd->MainDir;

    name.Format("%spictures\\%d - %s.bmp", MainDir, PicsTotal, nm);
    PicsTotal++;


    CreateDirectory(MainDir + "pictures", NULL);
    CreateBMPFile(NULL, name.GetBuffer(0), info,
        hbm, pDC->m_hDC);

}

void CChild_1_Frame::OnResize()
{
    SetWindowPos(NULL, 0, 0, 640, 480, SWP_SHOWWINDOW | SWP_DRAWFRAME | SWP_NOOWNERZORDER | SWP_NOMOVE);
}
