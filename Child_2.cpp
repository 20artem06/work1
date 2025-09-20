/////////////////////////////////////////////////////////////////////////////
// Child_2.cpp : implementation of the CChild_2_Frame class
//

#include "stdafx.h"

#include <fstream>
#include <math.h>
#include <cfloat>

#include <vfw.h>
#include <winuser.h>

#include "windows.h"

#include "video.h"
#include "MainFrm.h"
#include "F_2_dlg.h"
#include "Child_2.h"
#include "AviDlg.h"
#include <afxext.h>
#include "Session.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAXLAYERS 20

//#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "AviUtils.h"


/////////////////////////////////////////////////////////////////////////////
// CChild_2_Frame

IMPLEMENT_DYNCREATE(CChild_2_Frame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChild_2_Frame, CMDIChildWnd)
    //{{AFX_MSG_MAP(CChild_2_Frame)
    ON_COMMAND(ID_GRAPH_MODIFY, OnGraphModify)
    ON_WM_PAINT()
    ON_WM_RBUTTONDOWN()
    ON_COMMAND(ID_FILE_SAVEDATA, OnFileSavedata)
    ON_COMMAND(ID_FILE_SAVEPICT, OnFileSavepict)
    ON_WM_KILLFOCUS()
    ON_WM_LBUTTONDOWN()
    ON_COMMAND(IDD_SAVE_WMF, OnSaveWmf)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_COMMAND(ID_PHOTO_FILM, OnPhotoFilm)
    ON_COMMAND(ID_RESIZE, OnResize)
    ON_COMMAND(IDD_TEMPMAX, OnTempmax)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChild_2_Frame constructors/destructors

////////////////////////////Bitmap saving//////////////////////////


////////////////////////////////////////////////////////////////////////////////

// CChild_2_Frame main constructor
CChild_2_Frame::CChild_2_Frame(CField_2_Dlg* pDlg)
{   
    InitLayers();
    SetDataFromDto(pDlg);
    PrepareData();
}


CChild_2_Frame::CChild_2_Frame(VideoSession2Dim* session)
{   
    InitLayers();
    SetDataFromDto(session);
    PrepareData();
}


void CChild_2_Frame::InitLayers() {
    CMainFrame* pWnd = (CMainFrame*)AfxGetMainWnd();
    MainDir = pWnd->MainDir;
    PicsTotal = 1;

    std::fstream file;
    file.open("data.col", std::ios::_Nocreate | std::ios::in);
    if (!file.fail())
    {
        int lay, red, green, blue;
        for (int i = 0; i < MAXLAYERS; i++)
        {
            file >> lay;

            file >> blue;
            file >> green;
            file >> red;

            LAYER_COL[i] = RGB(red, green, blue);
        }
        file.close();
    }
    else {
        LAYER_COL[0] = 0;//RGB(255,255,255);
        LAYER_COL[1] = 256 * 256 * (int)(200) + 256 * (int)(200) + (int)(250);//RGB(0, 0, 0);
        LAYER_COL[2] = RGB(231, 123, 5);
        LAYER_COL[3] = RGB(83, 136, 154);
        LAYER_COL[4] = RGB(45, 196, 154);
        LAYER_COL[5] = RGB(155, 31, 182);
        LAYER_COL[6] = RGB(191, 20, 68);
        LAYER_COL[7] = RGB(129, 96, 46);
        LAYER_COL[8] = RGB(226, 221, 16);
        for (int i = 9; i < MAXLAYERS; i++)
        {
            LAYER_COL[i] = RGB(0, 0, 0);
        }

    }
}



BOOL CChild_2_Frame::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);	//Enable OpenGL

    return CMDIChildWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CChild_2_Frame diagnostics
#ifdef _DEBUG
void CChild_2_Frame::AssertValid() const
{
    CMDIChildWnd::AssertValid();
}

void CChild_2_Frame::Dump(CDumpContext& dc) const
{
    CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG


void CChild_2_Frame::SetDataFromDto(CField_2_Dlg* pDlg) {
    int i;

    // Take data from CField_2_Dlg
    pProject = pDlg->pProject;
    nField = pDlg->m_nField;
    fFmin = pDlg->m_fFmin;
    fFmax = pDlg->m_fFmax;
    bPolar = pDlg->m_bPolar;
    bLog = pDlg->m_bLog;
    nArg1 = pDlg->m_nArg1;
    nX1min = pDlg->m_nX1min;
    nX1max = pDlg->m_nX1max;
    nArg2 = pDlg->m_nArg2;
    nX2min = pDlg->m_nX2min;
    nX2max = pDlg->m_nX2max;
    nGrids = pDlg->m_nGrids;
    bGridL = pDlg->m_bGridLines;
    bGridAuto = pDlg->m_bGridAuto;
    ColorMax = pDlg->ColorMax;
    ColorMin = pDlg->ColorMin;
    Palette = pDlg->m_Palette;

    layers = pDlg->m_layers;
    cloud = pDlg->m_cloud;
    lines = pDlg->m_lines;


    INDEX.resize(pProject->m_field[nField].nAxis + 1);
    for (i = 0; i <= pProject->m_field[nField].nAxis; i++) {
        if ((int)i == nArg1) {
            INDEX[i] = nX1min;
        }
        else if ((int)i == nArg2) {
            INDEX[i] = nX2min;
        }
        else {
            INDEX[i] = pDlg->INDEX[i];
        }
    }

}

void CChild_2_Frame::SetDataFromDto(VideoSession2Dim* s) {
    // Take data from CField_2_Dlg
    pProject = s->pProject;
    nField = s->m_nField;
    fFmin = s->m_fFmin;
    fFmax = s->m_fFmax;
    bPolar = s->m_bPolar;
    bLog = s->m_bLog;
    nArg1 = s->m_nArg1;
    nX1min = s->m_nX1min;
    nX1max = s->m_nX1max;
    nArg2 = s->m_nArg2;
    nX2min = s->m_nX2min;
    nX2max = s->m_nX2max;
    nGrids = s->m_nGrids;
    bGridL = s->m_bGridLines;
    bGridAuto = s->m_bGridAuto;
    ColorMax = s->ColorMax;
    ColorMin = s->ColorMin;
    Palette = s->m_Palette;

    layers = s->m_layers;
    cloud = s->m_cloud;
    lines = s->m_lines;


    INDEX.resize(pProject->m_field[nField].nAxis + 1);
    for (int i = 0; i <= pProject->m_field[nField].nAxis; i++) {
        if ((int)i == nArg1) {
            INDEX[i] = nX1min;
        }
        else if ((int)i == nArg2) {
            INDEX[i] = nX2min;
        }
        else {
            INDEX[i] = s->INDEX[i];
        }
    }

}


/////////////////////////////////////////////////////////////////////////////
// PrepareData - Main CChild_2_Frame procedure:
//  preparing all data from dialog;
//  change window name;
//  find function values;
//  recalculate function Min and Max;
//  recalculate min,max for draw axes.
void CChild_2_Frame::PrepareData()
{
    int i = pProject->m_field[nField].nAxis + 1;
    
    //Change window name
    sName = pProject->m_field[nField].name;

    if (nArg1 != (int)pProject->m_field[nField].nAxis)	//X1 is axe
        sName += '(' + pProject->m_field[nField].axe[nArg1].name + ',';
    else												//X1 is time
        sName += '(' + pProject->m_sTime + ',';

    if (nArg2 != (int)pProject->m_field[nField].nAxis)  //X2 is axe
        sName += pProject->m_field[nField].axe[nArg2].name + ')';
    else												//X2 is time
        sName += pProject->m_sTime + ')';

    sDesc = sName;
    sName = "Two-dim graph " + sName;
    //Change description
    CString ttt;
    for (i = 0; i < pProject->m_field[nField].nAxis; i++)
    {
        if ((i != nArg1) && (i != nArg2))
        {
            ttt.Format("%g", pProject->m_field[nField].axe[i].point[INDEX[i]]);
            sDesc += ", " + pProject->m_field[nField].axe[i].name + "=" + ttt + " см";
        }
    }
    if ((nArg1 != pProject->m_field[nField].nAxis) && (nArg2 != pProject->m_field[nField].nAxis))
    {
        ttt.Format("%g", 1e9 * pProject->m_time[INDEX[pProject->m_field[nField].nAxis]]);
        sDesc += ", " + pProject->m_sTime + "=" + ttt + " нс";
    }


    

    //Prepare Function values TWO-Dim Fun(X1,X2)
    //------------------------------------------
    int j;
    __int64  l, k, kst1, kst2;

    // Names
    if (nArg1 != (int)pProject->m_field[nField].nAxis)   //X1 is axe
        sX1Name = pProject->m_field[nField].axe[nArg1].name + "["
        + pProject->m_field[nField].axe[nArg1].size + "]";
    else												 //X1 is time
        sX1Name = pProject->m_sTime + "["
        + pProject->m_sTimeSize + "]";

    if (nArg2 != (int)pProject->m_field[nField].nAxis)   //X2 is axe
        sX2Name = pProject->m_field[nField].axe[nArg2].name + "["
        + pProject->m_field[nField].axe[nArg2].size + "]";
    else												 //X2 is time
        sX2Name = pProject->m_sTime + "["
        + pProject->m_sTimeSize + "]";

    sFName = pProject->m_field[nField].name + "["	//Function name
        + pProject->m_field[nField].size + "]";

    nX1 = nX1max - nX1min + 1;
    nX2 = nX2max - nX2min + 1;
    Fun.resize(nX1 * nX2);
    X1.resize(nX1 * nX2);
    X2.resize(nX1 * nX2);

    // First index
    k = INDEX[pProject->m_field[nField].nAxis - 1];
    for (l = pProject->m_field[nField].nAxis - 2; l >= 0; l--)
        k = INDEX[l] + k * pProject->m_field[nField].axe[l].nPoints;

    k += INDEX[pProject->m_field[nField].nAxis] * (__int64)pProject->m_field[nField].nPoints;

    // X1 step
    if (nArg1 == (int)pProject->m_field[nField].nAxis)   //X1 is time
        kst1 = pProject->m_field[nField].nPoints;
    else												 //X1 is axe
    {
        kst1 = 1;
        for (l = 0; l < nArg1; l++)
            kst1 *= pProject->m_field[nField].axe[l].nPoints;
    }

    // X2 step
    if (nArg2 == (int)pProject->m_field[nField].nAxis)   //X2 is time
        kst2 = pProject->m_field[nField].nPoints - (nX1 - 1) * kst1;
    else												 //X2 is axe
    {
        kst2 = 1;
        for (l = 0; l < nArg2; l++)
            kst2 *= pProject->m_field[nField].axe[l].nPoints;
        kst2 -= (nX1 - 1) * kst1;
    }


    // Fill CEL section

    if (nArg1 == 0) CEL_ind[1] = 1; else
        if (nArg2 == 0) CEL_ind[1] = 2; else
        {
            CEL_ind[1] = 3;
            Values[3] = pProject->m_field[nField].axe[0].point[INDEX[0]];
        }

    if (nArg1 == 1) CEL_ind[2] = 1; else
        if (nArg2 == 1) CEL_ind[2] = 2; else
        {
            CEL_ind[2] = 4;
            Values[4] = pProject->m_field[nField].axe[1].point[INDEX[1]];
        }

    if (nArg1 == 2) CEL_ind[3] = 1; else
        if (nArg2 == 2) CEL_ind[3] = 2; else
        {
            CEL_ind[3] = 5;
            Values[5] = pProject->m_field[nField].axe[2].point[INDEX[2]];
        }

    //Search function values 

    pProject->read_points(nField, k, kst1, nX1, kst2 - kst1, nX2, Fun, bLog);  //Function value

    for (j = nX2min; (int)j <= nX2max; j++)
    {
        for (i = nX1min; (int)i <= nX1max; i++)
        {
            l = i - nX1min + (j - nX2min) * nX1;

            if (nArg1 != (int)pProject->m_field[nField].nAxis)   //X1 is axe
                X1[l] = pProject->m_field[nField].axe[nArg1].point[i];
            else												 //X1 is time
                X1[l] = pProject->m_time[i];

            if (nArg2 != (int)pProject->m_field[nField].nAxis)   //X2 is axe
                X2[l] = pProject->m_field[nField].axe[nArg2].point[j];
            else												 //X2 is time
                X2[l] = pProject->m_time[j];

        }
    }

    if (pProject->m_field[nField].name == "N")
    {
        // podelit' na razmer
        for (i = 0; i < nX1 - 1; i++)
        {
            for (j = 0; j < nX2 - 1; j++)
            {
                Fun[j * nX1 + i] = Fun[j * nX1 + i] /
                    ((X1[i + 1] - X1[i]) * (X2[(j + 1) * nX1] - X2[j * nX1]));
            }
            Fun[j * nX1 + i] = Fun[j * nX1 + i] /
                ((X1[i] - X1[i - 1]) * (X2[(j + 1) * nX1] - X2[j * nX1]));
        }
        for (j = 0; j < nX2 - 1; j++)
            Fun[j * nX1 + i] = Fun[j * nX1 + i] /
            ((X1[i] - X1[i - 1]) * (X2[(j + 1) * nX1] - X2[(j)*nX1]));

        Fun[j * nX1 + i] = Fun[j * nX1 + i] /
            ((X1[i] - X1[i - 1]) * (X2[(j)*nX1] - X2[(j - 1) * nX1]));

    }

    //Recalculate fMin,fMax
    float fMin = FLT_MAX;
    float fMax = -FLT_MAX;
    if (bLog)
    {
        fMin = 37;
        fMax = -37;
    }
    for (j = 0; j < nX1 * nX2; j++)
    {
        if (Fun[j] > fMax) fMax = Fun[j];
        if (Fun[j] < fMin) fMin = Fun[j];
    }
    frealmax = fMax;
    frealmin = fMin;
    if (fFmax > fMax) fFmax = fMax;
    if (fFmin < fMin) fFmin = fMin;

    //Prepare Polar coordinates
    //-------------------------
    if (bPolar)
    {
        CString ttt = sX2Name + "*COS(" + sX1Name + ")";// Axes names
        sX2Name = sX2Name + "*SIN(" + sX1Name + ")";
        sX1Name = ttt;

        float   x1, x2;
        fX1min = fX2min = FLT_MAX;
        fX1max = fX2max = -FLT_MAX;

        for (i = 0; i < (int)(nX1 * nX2); i++)
        {
            x1 = X1[i] * (float)cos(X2[i]);//X1-argument
            if (x1 > fX1max) fX1max = x1;
            if (x1 < fX1min) fX1min = x1;
            x2 = X1[i] * (float)sin(X2[i]);//X2-argument
            if (x2 > fX2max) fX2max = x2;
            if (x2 < fX2min) fX2min = x2;
            X1[i] = x1;
            X2[i] = x2;
        }
    }

    else
        //Prepare decart coordinates
        //--------------------------
    {
        //Calculate X1 min,max
        if (nArg1 != (int)pProject->m_field[nField].nAxis)   //X1 is axe
        {
            fX1max = pProject->m_field[nField].axe[nArg1].point[nX1max];
            fX1min = pProject->m_field[nField].axe[nArg1].point[nX1min];
        } else												 //X1 is time
        {
            fX1max = pProject->m_time[nX1max];
            fX1min = pProject->m_time[nX1min];
        }
        //Calculate X2 min,max
        if (nArg2 != (int)pProject->m_field[nField].nAxis)   //X2 is axe
        {
            fX2max = pProject->m_field[nField].axe[nArg2].point[nX2max];
            fX2min = pProject->m_field[nField].axe[nArg2].point[nX2min];
        } else												 //X2 is time
        {
            fX2max = pProject->m_time[nX2max];
            fX2min = pProject->m_time[nX2min];
        }
    }

    //Prepare number of grid lines for "Auto" mode
    if (bGridAuto)
    {
        double dF = fFmax - fFmin;
        if (bLog)
        {
            for (int i = 1; i < 4; i++)
            {
                int inter = dF / i;
                if (inter <= 20)
                {
                    nGrids = inter;
                    break;
                }
            }
        }
        else
        {
            for (int i = 0; i < 37; i++)
            {
                for (int j = 1; j < 10; j++)
                {
                    INT64 inter = dF / (j *pow(10, i));
                    if ((inter <= 25) && (inter >= 10))
                    {
                        nGrids = inter;
                        break;
                    }
                }
            }
        }
    }

    //Recalculate X1,X2 min,max for draw
    float dx;

    if (fX1max <= fX1min) fX1max = fX1min + 1;
    dx = (float)0.1 * Roundation(fX1max - fX1min);
    fX1max = (Divide(fX1max, dx) + 1) * dx;
    fX1min = Divide(fX1min, dx) * dx;

    if (fX2max <= fX2min) fX2max = fX2min + 1;
    dx = (float)0.1 * Roundation(fX2max - fX2min);
    fX2max = (Divide(fX2max, dx) + 1) * dx;
    fX2min = Divide(fX2min, dx) * dx;

    //Recalculate grid minimun and nGrids for draw TWO-Dim graph
    fGmin = Scaling(nGrids, fFmax, fFmin, bGridAuto, bLog, &fGstep);
}


void CChild_2_Frame::PrepareData_max()
{
    int i;

    INDEX[pProject->m_field[nField].nAxis] = 0; // from the very top

    //Prepare Function values TWO-Dim Fun(X1,X2)
    //------------------------------------------
    int j;
    __int64  l, k, kst1, kst2;

    // Names
    if (nArg1 != (int)pProject->m_field[nField].nAxis)   //X1 is axe
        sX1Name = pProject->m_field[nField].axe[nArg1].name + "["
        + pProject->m_field[nField].axe[nArg1].size + "]";
    else												 //X1 is time
        sX1Name = pProject->m_sTime + "["
        + pProject->m_sTimeSize + "]";

    if (nArg2 != (int)pProject->m_field[nField].nAxis)   //X2 is axe
        sX2Name = pProject->m_field[nField].axe[nArg2].name + "["
        + pProject->m_field[nField].axe[nArg2].size + "]";
    else												 //X2 is time
        sX2Name = pProject->m_sTime + "["
        + pProject->m_sTimeSize + "]";

    sFName = pProject->m_field[nField].name + "["	//Function name
        + pProject->m_field[nField].size + "]";

    nX1 = nX1max - nX1min + 1;
    nX2 = nX2max - nX2min + 1;
    Fun.resize(nX1 * nX2);
    X1.resize(nX1 * nX2);
    X2.resize(nX1 * nX2);

    std::vector<float> tmpFun(nX1 * nX2);

    // First index
    k = INDEX[pProject->m_field[nField].nAxis - 1];
    for (l = pProject->m_field[nField].nAxis - 2; l >= 0; l--)
        k = INDEX[l] + k * pProject->m_field[nField].axe[l].nPoints;

    k += INDEX[pProject->m_field[nField].nAxis] * (__int64)pProject->m_field[nField].nPoints;

    // X1 step
    if (nArg1 == (int)pProject->m_field[nField].nAxis)   //X1 is time
        kst1 = pProject->m_field[nField].nPoints;
    else												 //X1 is axe
    {
        kst1 = 1;
        for (l = 0; l < nArg1; l++)
            kst1 *= pProject->m_field[nField].axe[l].nPoints;
    }

    // X2 step
    if (nArg2 == (int)pProject->m_field[nField].nAxis)   //X2 is time
        kst2 = pProject->m_field[nField].nPoints - (nX1 - 1) * kst1;
    else												 //X2 is axe
    {
        kst2 = 1;
        for (l = 0; l < nArg2; l++)
            kst2 *= pProject->m_field[nField].axe[l].nPoints;
        kst2 -= (nX1 - 1) * kst1;
    }

    // Fill CEL section

    if (nArg1 == 0) CEL_ind[1] = 1; else
        if (nArg2 == 0) CEL_ind[1] = 2; else
        {
            CEL_ind[1] = 3;
            Values[3] = pProject->m_field[nField].axe[0].point[INDEX[0]];
        }

    if (nArg1 == 1) CEL_ind[2] = 1; else
        if (nArg2 == 1) CEL_ind[2] = 2; else
        {
            CEL_ind[2] = 4;
            Values[4] = pProject->m_field[nField].axe[1].point[INDEX[1]];
        }

    if (nArg1 == 2) CEL_ind[3] = 1; else
        if (nArg2 == 2) CEL_ind[3] = 2; else
        {
            CEL_ind[3] = 5;
            Values[5] = pProject->m_field[nField].axe[2].point[INDEX[2]];
        }

    k = INDEX[pProject->m_field[nField].nAxis - 1];
    for (l = pProject->m_field[nField].nAxis - 2; l >= 0; l--)
        k = INDEX[l] + k * pProject->m_field[nField].axe[l].nPoints;

    k += INDEX[pProject->m_field[nField].nAxis] * (__int64)pProject->m_field[nField].nPoints;

    pProject->read_points(nField, k, kst1, nX1, kst2 - kst1, nX2, Fun, bLog);  //Function value


//Search function values 
    for (INDEX[pProject->m_field[nField].nAxis] = 0; INDEX[pProject->m_field[nField].nAxis] < pProject->m_nTimePoints; INDEX[pProject->m_field[nField].nAxis]++) {

        // First index
        k = INDEX[pProject->m_field[nField].nAxis - 1];
        for (l = pProject->m_field[nField].nAxis - 2; l >= 0; l--)
            k = INDEX[l] + k * pProject->m_field[nField].axe[l].nPoints;

        k += INDEX[pProject->m_field[nField].nAxis] * (__int64)pProject->m_field[nField].nPoints;

        pProject->read_points(nField, k, kst1, nX1, kst2 - kst1, nX2, tmpFun, bLog);  //Function value

        for (int i = 0; i < nX1 * nX2; ++i) {
            Fun[i] = (fabs(Fun[i]) > fabs(tmpFun[i])) ? Fun[i] : tmpFun[i];
        }
    }

    for (j = nX2min; (int)j <= nX2max; j++)
    {
        for (i = nX1min; (int)i <= nX1max; i++)
        {
            l = i - nX1min + (j - nX2min) * nX1;

            if (nArg1 != (int)pProject->m_field[nField].nAxis)   //X1 is axe
                X1[l] = pProject->m_field[nField].axe[nArg1].point[i];
            else												 //X1 is time
                X1[l] = pProject->m_time[i];

            if (nArg2 != (int)pProject->m_field[nField].nAxis)   //X2 is axe
                X2[l] = pProject->m_field[nField].axe[nArg2].point[j];
            else												 //X2 is time
                X2[l] = pProject->m_time[j];

        }
    }

    if (pProject->m_field[nField].name == "N")
    {
        // podelit' na razmer
        for (i = 0; i < nX1 - 1; i++)
        {
            for (j = 0; j < nX2 - 1; j++)
            {
                Fun[j * nX1 + i] = Fun[j * nX1 + i] /
                    ((X1[i + 1] - X1[i]) * (X2[(j + 1) * nX1] - X2[j * nX1]));
            }
            Fun[j * nX1 + i] = Fun[j * nX1 + i] /
                ((X1[i] - X1[i - 1]) * (X2[(j + 1) * nX1] - X2[j * nX1]));
        }
        for (j = 0; j < nX2 - 1; j++)
            Fun[j * nX1 + i] = Fun[j * nX1 + i] /
            ((X1[i] - X1[i - 1]) * (X2[(j + 1) * nX1] - X2[(j)*nX1]));

        Fun[j * nX1 + i] = Fun[j * nX1 + i] /
            ((X1[i] - X1[i - 1]) * (X2[(j)*nX1] - X2[(j - 1) * nX1]));

    }

    //Recalculate fMin,fMax
    float fMin = FLT_MAX;
    float fMax = -FLT_MAX;
    for (j = 0; j < nX1 * nX2; j++)
    {
        if (Fun[j] > fMax) fMax = Fun[j];
        if (Fun[j] < fMin) fMin = Fun[j];
    }
    frealmax = fMax;
    frealmin = fMin;
    if (fFmax > fMax) fFmax = fMax;
    if (fFmin < fMin) fFmin = fMin;

    //Prepare Polar coordinates
    //-------------------------
    if (bPolar)
    {
        CString ttt = sX2Name + "*COS(" + sX1Name + ")";// Axes names
        sX2Name = sX2Name + "*SIN(" + sX1Name + ")";
        sX1Name = ttt;

        float   x1, x2;
        fX1min = fX2min = FLT_MAX;
        fX1max = fX2max = -FLT_MAX;

        for (i = 0; i < (int)(nX1 * nX2); i++)
        {
            x1 = X1[i] * (float)cos(X2[i]);//X1-argument
            if (x1 > fX1max) fX1max = x1;
            if (x1 < fX1min) fX1min = x1;
            x2 = X1[i] * (float)sin(X2[i]);//X2-argument
            if (x2 > fX2max) fX2max = x2;
            if (x2 < fX2min) fX2min = x2;
            X1[i] = x1;
            X2[i] = x2;
        }
    }

    else
        //Prepare decart coordinates
        //--------------------------
    {
        //Calculate X1 min,max
        if (nArg1 != (int)pProject->m_field[nField].nAxis)   //X1 is axe
        {
            fX1max = pProject->m_field[nField].axe[nArg1].point[nX1max];
            fX1min = pProject->m_field[nField].axe[nArg1].point[nX1min];
        } else												 //X1 is time
        {
            fX1max = pProject->m_time[nX1max];
            fX1min = pProject->m_time[nX1min];
        }
        //Calculate X2 min,max
        if (nArg2 != (int)pProject->m_field[nField].nAxis)   //X2 is axe
        {
            fX2max = pProject->m_field[nField].axe[nArg2].point[nX2max];
            fX2min = pProject->m_field[nField].axe[nArg2].point[nX2min];
        } else												 //X2 is time
        {
            fX2max = pProject->m_time[nX2max];
            fX2min = pProject->m_time[nX2min];
        }
    }

    //Recalculate X1,X2 min,max for draw
    float dx;

    if (fX1max <= fX1min) fX1max = fX1min + 1;
    dx = (float)0.1 * Roundation(fX1max - fX1min);
    fX1max = (Divide(fX1max, dx) + 1) * dx;
    fX1min = Divide(fX1min, dx) * dx;

    if (fX2max <= fX2min) fX2max = fX2min + 1;
    dx = (float)0.1 * Roundation(fX2max - fX2min);
    fX2max = (Divide(fX2max, dx) + 1) * dx;
    fX2min = Divide(fX2min, dx) * dx;

    //Recalculate grid minimun and nGrids for draw TWO-Dim graph
    //fGmin = Scaling(nGrids, fFmax, fFmin, &fGstep);
}

void CChild_2_Frame::PrepareData_time(BOOL MinMax)
{

    INDEX[pProject->m_field[nField].nAxis] += 1;


    sName = pProject->m_field[nField].name;

    if (nArg1 != (int)pProject->m_field[nField].nAxis)	//X1 is axe
        sName += '(' + pProject->m_field[nField].axe[nArg1].name + ',';
    else												//X1 is time
        sName += '(' + pProject->m_sTime + ',';

    if (nArg2 != (int)pProject->m_field[nField].nAxis)  //X2 is axe
        sName += pProject->m_field[nField].axe[nArg2].name + ')';
    else												//X2 is time
        sName += pProject->m_sTime + ')';

    sDesc = sName;
    sName = "Two-dim graph " + sName;
    //Change description
    CString ttt;
    int i;
    for (i = 0; i < pProject->m_field[nField].nAxis; i++)
    {
        if ((i != nArg1) && (i != nArg2))
        {
            ttt.Format("%g", pProject->m_field[nField].axe[i].point[INDEX[i]]);
            sDesc += ", " + pProject->m_field[nField].axe[i].name + "=" + ttt + " см";
        }
    }
    if ((nArg1 != pProject->m_field[nField].nAxis) && (nArg2 != pProject->m_field[nField].nAxis))
    {
        ttt.Format("%g", 1e9 * pProject->m_time[INDEX[pProject->m_field[nField].nAxis]]);
        sDesc += ", " + pProject->m_sTime + "=" + ttt + " нс";
    }

    //Prepare Function values TWO-Dim Fun(X1,X2)
    //------------------------------------------
    int j;
    int  l, k, kst1, kst2;

    // Memory operations
    nX1 = nX1max - nX1min + 1;
    nX2 = nX2max - nX2min + 1;

    // First index
    k = INDEX[pProject->m_field[nField].nAxis - 1];
    for (l = pProject->m_field[nField].nAxis - 2; l >= 0; l--)
        k = INDEX[l] + k * pProject->m_field[nField].axe[l].nPoints;

    k += INDEX[pProject->m_field[nField].nAxis] * pProject->m_field[nField].nPoints;

    // X1 step
    if (nArg1 == (int)pProject->m_field[nField].nAxis)   //X1 is time
        kst1 = pProject->m_field[nField].nPoints;
    else												 //X1 is axe
    {
        kst1 = 1;
        for (l = 0; l < nArg1; l++)
            kst1 *= pProject->m_field[nField].axe[l].nPoints;
    }

    // X2 step
    if (nArg2 == (int)pProject->m_field[nField].nAxis)   //X2 is time
        kst2 = pProject->m_field[nField].nPoints - (nX1 - 1) * kst1;
    else												 //X2 is axe
    {
        kst2 = 1;
        for (l = 0; l < nArg2; l++)
            kst2 *= pProject->m_field[nField].axe[l].nPoints;
        kst2 -= (nX1 - 1) * kst1;
    }

    //Search function values 

    pProject->read_points(nField, k, kst1, nX1, kst2 - kst1, nX2, Fun, bLog);  //Function value

    if (pProject->m_field[nField].name == "N")
    {
        // Podelit' na razmer jachejki
        int i;
        for (i = 0; i < nX1 - 1; i++)
        {
            for (j = 0; j < nX2 - 1; j++)
            {
                Fun[j * nX1 + i] = Fun[j * nX1 + i] /
                    ((X1[i + 1] - X1[i]) * (X2[(j + 1) * nX1] - X2[j * nX1]));
            }
            Fun[j * nX1 + i] = Fun[j * nX1 + i] /
                ((X1[i] - X1[i - 1]) * (X2[(j + 1) * nX1] - X2[j * nX1]));
        }
        for (j = 0; j < nX2 - 1; j++)
            Fun[j * nX1 + i] = Fun[j * nX1 + i] /
            ((X1[i] - X1[i - 1]) * (X2[(j + 1) * nX1] - X2[(j)*nX1]));

        Fun[j * nX1 + i] = Fun[j * nX1 + i] /
            ((X1[i] - X1[i - 1]) * (X2[(j)*nX1] - X2[(j - 1) * nX1]));
    }


    //Recalculate fMin,fMax
    float fMin = FLT_MAX;
    float fMax = -FLT_MAX;
    for (j = 0; j < nX1 * nX2; j++)
    {
        if (Fun[j] > fMax) fMax = Fun[j];
        if (Fun[j] < fMin) fMin = Fun[j];
    }
    frealmax = fMax;
    frealmin = fMin;



    if (!MinMax)
    {
        fFmax = fMax;
        fFmin = fMin;
    }

    //fGmin = Scaling(nGrids, fFmax, fFmin, &fGstep);

}
/////////////////////////////////////////////////////////////////////////////
// CChild_2_Frame message handlers

void CChild_2_Frame::OnKillFocus(CWnd* pNewWnd)
{
    Invalidate();//redraw
    CMDIChildWnd::OnKillFocus(pNewWnd);

    // Clearning status bar
    CMainFrame* pWnd = (CMainFrame*)AfxGetMainWnd();
    pWnd->pStatusBar->SetText("", 1, 0);
    pWnd->pStatusBar->SetText("", 2, 0);
    pWnd->pStatusBar->SetText("", 3, 0);
}

/////////////////////////////////////////////////////////////////////////////
// OnGraphModify
void CChild_2_Frame::OnGraphModify()
{
    CField_2_Dlg dlg;
    dlg.bNewGraph = FALSE;
    //Restore values for CField_2_Dlg
    dlg.pProject = pProject;
    dlg.m_nField = nField;
    dlg.m_fFmin = fFmin;
    dlg.m_fFmax = fFmax;
    dlg.m_bPolar = bPolar;
    dlg.m_nArg1 = nArg1;
    dlg.m_nX1min = nX1min;
    dlg.m_nX1max = nX1max;
    dlg.m_nArg2 = nArg2;
    dlg.m_nX2min = nX2min;
    dlg.m_nX2max = nX2max;
    dlg.m_nGrids = nGrids;
    dlg.m_bGridLines = bGridL;
    dlg.ColorMax = ColorMax;
    dlg.ColorMin = ColorMin;
    dlg.m_cloud = cloud;
    dlg.m_lines = lines;
    dlg.m_bLog = bLog;
    dlg.m_bGridAuto = bGridAuto;
    //dlg.m_layers=layers;


    int i;
    dlg.INDEX.resize(pProject->m_field[nField].nAxis + 1);
    for (i = 0; i <= pProject->m_field[nField].nAxis; i++) {
        dlg.INDEX[i] = INDEX[i];
    }

    //Do CField_2_Dlg "Graph->Modify"
    if (dlg.DoModal() != IDOK)return;

    //Change current values
    SetDataFromDto(&dlg);
    PrepareData();
    SetWindowText(sName);
    Invalidate();
}

VideoSession2Dim CChild_2_Frame::GetSession() {
    VideoSession2Dim session;

    session.pProject = pProject;
    session.m_nField = nField;
    session.m_fFmin = fFmin;
    session.m_fFmax = fFmax;
    session.m_bPolar = bPolar;
    session.m_nArg1 = nArg1;
    session.m_nX1min = nX1min;
    session.m_nX1max = nX1max;
    session.m_nArg2 = nArg2;
    session.m_nX2min = nX2min;
    session.m_nX2max = nX2max;
    session.m_nGrids = nGrids;
    session.m_bGridLines = bGridL;
    session.ColorMax = ColorMax;
    session.ColorMin = ColorMin;
    session.m_bLog = bLog;
    session.m_bGridAuto = bGridAuto;

    session.m_layers = layers;
    session.m_cloud = cloud;
    session.m_lines = lines;
    session.m_Palette = Palette;



    int i;
    session.INDEX.resize(pProject->m_field[nField].nAxis + 1);
    for (i = 0; i <= pProject->m_field[nField].nAxis; i++) {
        session.INDEX[i] = INDEX[i];
    }


    return session;
}


/////////////////////////////////////////////////////////////////////////////
// Call Graph-modify dialog by Rigth mouse button
void CChild_2_Frame::OnRButtonDown(UINT nFlags, CPoint point)
{
    CChild_2_Frame::OnGraphModify();
}

/////////////////////////////////////////////////////////////////////////////
// Save data
void CChild_2_Frame::OnFileSavedata()
{

    CFileDialog dlg(FALSE, "dat", "data", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "Data files (*.dat)|*.dat||");
    dlg.m_ofn.lpstrInitialDir = MainDir;
    if (dlg.DoModal() != IDOK)return;	//Take filename from 'SAVE AS' dialog
    std::fstream file;	file.open(dlg.GetPathName(), std::ios::out);
    int i;
	PrepareData();
    file << sX1Name << "  " << sX2Name << "  " << sFName << "\n";//Title
    for (i = 0; i < nX1 * nX2; i++)
        //if ((Fun[i] >= fFmin) && (Fun[i] <= fFmax))
            file << X1[i] << "  " << X2[i] << "  " << Fun[i] << "\n";//Data
		
    file.close();
}

/////////////////////////////////////////////////////////////////////////////
void CChild_2_Frame::OnFileSavepict()
{
    // Здесь частично использован код с сайта http://www.wischik.com/lu/programmer/avi_utils.html
    // Этот код распространяется свободно:
    //(c) 2002 Lucian Wischik. This code is free, and anyone can do with it whatever they like
    //	(except sell it or claim ownership).


    CAviDlg Dlg(this);

    if ((nArg1 == (int)pProject->m_field[nField].nAxis) || (nArg2 == (int)pProject->m_field[nField].nAxis))
    {
        AfxMessageBox("Для того, чтобы сохранить видео-файл время должно быть свободно\n", MB_OK);
        return;
    }

    //Dlg.
    CMainFrame* pWnd = (CMainFrame*)AfxGetMainWnd();
    MainDir = pWnd->MainDir;
    Dlg.m_FileName = "test.avi";

    if (Dlg.DoModal() != IDOK) return;

    CDC* cdcscreen = GetDC();
    HDC hdcscreen = cdcscreen->m_hDC, hdc = CreateCompatibleDC(hdcscreen); ReleaseDC(cdcscreen);
    BITMAPINFO bi; ZeroMemory(&bi, sizeof(bi)); BITMAPINFOHEADER& bih = bi.bmiHeader;
    bih.biSize = sizeof(bih);
    bih.biWidth = 800;
    bih.biHeight = 600;
    bih.biPlanes = 1;
    bih.biBitCount = 32;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = ((bih.biWidth * bih.biBitCount / 8 + 3) & 0xFFFFFFFC) * bih.biHeight;
    bih.biXPelsPerMeter = 10000;
    bih.biYPelsPerMeter = 10000;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;
    void* bits; HBITMAP hbm = CreateDIBSection(hdc, (BITMAPINFO*)&bih, DIB_RGB_COLORS, &bits, NULL, NULL);
    //
    HGDIOBJ holdb = SelectObject(hdc, hbm);
    HPEN hp = CreatePen(PS_SOLID, 16, RGB(255, 255, 128));
    HGDIOBJ holdp = SelectObject(hdc, hp);

    if (Dlg.m_Delay == 0) Dlg.m_Delay = 100;
    //
    HAVI avi = CreateAvi(MainDir + "test.avi", Dlg.m_Delay, NULL);
    INDEX[pProject->m_field[nField].nAxis] = Dlg.start_frame - 1; // I will add 1 later

    // Trying bits
    DWORD* dbits = (DWORD*)bits;
    DWORD* object = new DWORD[bih.biHeight * bih.biWidth];

    CRect GrafRec;
    GrafRec.left = 0;
    GrafRec.right = 800;
    GrafRec.top = 0;
    GrafRec.bottom = 600;

    BOOL lines;
    lines = FALSE;

    int nX1_real, nX2_real;
    int* X1_real_ind, * X2_real_ind;
    int** MinIzo;
    int** MaxIzo;

    X1_real_ind = new int[nX1];
    X2_real_ind = new int[nX2];
    X1_real_ind[0] = 0;
    X2_real_ind[0] = 0;

    int ii, jj, ia, ja;
    int i, j, k;

    float mX1 = GrafRec.Width() / (fX1max - fX1min);
    float mX2 = GrafRec.Height() / (fX2max - fX2min);

    for (ii = 0, jj = 0; ii < nX1; ii++)
        if (mX1 * (X1[ii] - X1[X1_real_ind[jj]]) > 1)
        {
            jj++;
            X1_real_ind[jj] = ii;
        }
    nX1_real = jj + 1;
    for (ii = 0, jj = 0; ii < nX2; ii++)
        if (mX2 * (X2[ii * nX1] - X2[X2_real_ind[jj] * nX1]) > 1)
        {
            jj++;
            X2_real_ind[jj] = ii;
        }
    nX2_real = jj + 1;

    float temp_fun;

    if (lines) {
        MinIzo = new int* [nX1_real - 1];
        for (k = 0; k < nX1_real - 1; k++) {
            MinIzo[k] = new int[nX2_real - 1];
        }

        MaxIzo = new int* [nX1_real - 1];
        for (k = 0; k < nX1_real - 1; k++) {
            MaxIzo[k] = new int[nX2_real - 1];
        }
    }

    float   f[4], x1[4], x2[4];
    CPoint  P[4];
    BOOL	B[4];

    COLORREF color;
    int	n_hR, n_hG, n_hB;


    float fMin;
    float fMax;

    double X;
    float hX, X1_, X2_;
    int nX;

    for (i = 0; i < GrafRec.Height(); i++) {
        for (j = 0; j < GrafRec.Width(); j++) {
            if (layers && pProject->has_cel) {
                Values[1] = j / mX1 + fX1min;
                Values[2] = i / mX2 + fX2min;

                object[j + i * bih.biWidth] = LAYER_COL[pProject->GetLayer(Values[CEL_ind[1]], Values[CEL_ind[2]], Values[CEL_ind[3]])];
            } else {
                object[j + i * bih.biWidth] = 0;
            }
        }
    }

    for (int frame = 0; frame < Dlg.frames; frame++) {
        PrepareData_time(Dlg.m_MinMaxMode);

        for (i = 0; i < (nX1_real - 1); i++) {
            for (j = 0; j < (nX2_real - 1); j++) {
                temp_fun = 0;
                for (ii = X1_real_ind[i]; ii < X1_real_ind[i + 1]; ii++) {
                    for (jj = X2_real_ind[j]; jj < X2_real_ind[j + 1]; jj++) {
                        temp_fun += Fun[jj * nX1 + ii] * (X1[jj * nX1 + ii + 1] - X1[jj * nX1 + ii]) * (X2[(jj + 1) * nX1 + ii] - X2[jj * nX1 + ii]);
                    }
                }

                Fun[X2_real_ind[j] * nX1 + X1_real_ind[i]] = temp_fun / ((X1[X1_real_ind[i + 1]] - X1[X1_real_ind[i]]) * (X2[X2_real_ind[j + 1] * nX1] - X2[X2_real_ind[j] * nX1]));
            }
        }

        if (lines) {
            HPEN mpen1 = CreatePen(PS_SOLID, 1, 2000);
            SelectObject(hdc, mpen1);

            Rectangle(hdc, 0, 0, 800, 600);

            HPEN mpen2 = CreatePen(PS_SOLID, 1, 0x000000);
            SelectObject(hdc, mpen2);

            //Scale calculation

            //Draw axis lines
            MoveToEx(hdc, GrafRec.left, GrafRec.top, NULL);
            LineTo(hdc, GrafRec.left, GrafRec.bottom);
            LineTo(hdc, GrafRec.right, GrafRec.bottom);
            CString ttt;
            CSize sSize;

            ///////////////////////////////////////////////////////////////////////
            //Draw TWO-Dim graph

            color = ColorMin;

            if (nGrids > 0) {
                n_hR = (GetRValue(ColorMax) - GetRValue(ColorMin)) / nGrids;
                n_hG = (GetGValue(ColorMax) - GetGValue(ColorMin)) / nGrids;
                n_hB = (GetBValue(ColorMax) - GetBValue(ColorMin)) / nGrids;
            }


            X = fGmin;

            for (i = 0; i < (nX2_real - 1); i++) {
                for (j = 0; j < (nX1_real - 1); j++) {
                    fMin = FLT_MAX;
                    fMax = -FLT_MAX;
                    f[0] = Fun[X2_real_ind[i] * nX1 + X1_real_ind[j]];              //      P[2]
                    f[1] = Fun[X2_real_ind[i] * nX1 + X1_real_ind[j + 1]];          //  !           !
                    f[2] = Fun[X2_real_ind[(i + 1)] * nX1 + X1_real_ind[j]];        //  !           !
                    f[3] = Fun[X2_real_ind[(i + 1)] * nX1 + X1_real_ind[j + 1]];    // f[0]---*---f[1]

                    for (k = 0; k < 4; k++) {
                        if (f[k] < fMin) fMin = f[k];
                        if (f[k] > fMax) fMax = f[k];
                    }

                    MinIzo[j][i] = (int)((fMin - fGmin) / fGstep - 2);
                    MaxIzo[j][i] = (int)((fMax - fGmin) / fGstep + 2);
                }
            }

            for (nX = 0; (int)nX < nGrids; nX++) {
                //Prepare pen
                k = nX - (int)((nX) / 5) * 5;
                HPEN mpen3 = CreatePen(PS_SOLID, 1, color);
                SelectObject(hdc, mpen3);

                color = RGB(GetRValue(color) + n_hR, GetGValue(color) + n_hG, GetBValue(color) + n_hB); //change color

                for (i = 0; i < (nX2_real - 1); i++) {
                    for (j = 0; j < (nX1_real - 1); j++) {
                        //Find izoline points
                        for (k = 0; k < 4; k++) {
                            B[k] = FALSE;
                        }

                        ii = i;
                        jj = j;
                        i = X2_real_ind[ii];
                        j = X1_real_ind[jj];
                        ia = X2_real_ind[ii + 1];
                        ja = X1_real_ind[jj + 1];

                        f[0] = Fun[i * nX1 + j] - X;    //            P[2]
                        x1[0] = X1[i * nX1 + j];        //      f[2]---*---f[3]
                        x2[0] = X2[i * nX1 + j];        //       !          !
                        f[1] = Fun[i * nX1 + ja] - X;   //       !          !
                        x1[1] = X1[i * nX1 + ja];       //       !          !
                        x2[1] = X2[i * nX1 + ja];       //  P[1] *          * P[3]
                        f[2] = Fun[(ia)*nX1 + j] - X;   //       !          !
                        x1[2] = X1[(ia)*nX1 + j];       //       !          !
                        x2[2] = X2[(ia)*nX1 + j];       //       !          !
                        f[3] = Fun[(ia)*nX1 + ja] - X;  //      f[0]---*---f[1]
                        x1[3] = X1[(ia)*nX1 + ja];      //            P[0]
                        x2[3] = X2[(ia)*nX1 + ja];

                        i = ii;
                        j = jj;


                        if ((f[0] * f[1] <= 0) && (f[1] != f[0])) {
                            //find P[0] between x1,x2[0] and x1,x2[1]
                            hX = (float)(fabs(f[0] / (f[1] - f[0])));
                            P[0].x = GrafRec.left + (int)(mX1 * (x1[0] + hX * (x1[1] - x1[0]) - fX1min));
                            P[0].y = GrafRec.bottom - (int)(mX2 * (x2[0] + hX * (x2[1] - x2[0]) - fX2min));
                            B[0] = TRUE;
                        }

                        if ((f[0] * f[2] <= 0) && (f[0] != f[2])) {
                            //find P[1] between x1,x2[0] and x1,x2[2]
                            hX = (float)(fabs(f[0] / (f[2] - f[0])));
                            P[1].x = GrafRec.left + (int)(mX1 * (x1[0] + hX * (x1[2] - x1[0]) - fX1min));
                            P[1].y = GrafRec.bottom - (int)(mX2 * (x2[0] + hX * (x2[2] - x2[0]) - fX2min));
                            B[1] = TRUE;
                        }

                        if ((f[2] * f[3] <= 0) && (f[3] != f[2])) {
                            //find P[2] between x1,x2[2] and x1,x2[3]
                            hX = (float)(fabs(f[2] / (f[3] - f[2])));
                            P[2].x = GrafRec.left + (int)(mX1 * (x1[2] + hX * (x1[3] - x1[2]) - fX1min));
                            P[2].y = GrafRec.bottom - (int)(mX2 * (x2[2] + hX * (x2[3] - x2[2]) - fX2min));
                            B[2] = TRUE;
                        }

                        if ((f[1] * f[3] <= 0) && (f[1] != f[3])) {
                            //find P[3] between x1,x2[1] and x1,x2[3]
                            hX = (float)(fabs(f[1] / (f[3] - f[1])));
                            P[3].x = GrafRec.left + (int)(mX1 * (x1[1] + hX * (x1[3] - x1[1]) - fX1min));
                            P[3].y = GrafRec.bottom - (int)(mX2 * (x2[1] + hX * (x2[3] - x2[1]) - fX2min));
                            B[3] = TRUE;
                        }

                        //Draw izolines:
                        /* P[0] - P[2] */   if (B[0] && B[2] && !B[1]) { MoveToEx(hdc, P[0].x, P[0].y, NULL); LineTo(hdc, P[2].x, P[2].y); continue; }
                        /* P[1] - P[3] */   if (B[1] && B[3] && !B[0]) { MoveToEx(hdc, P[1].x, P[1].y, NULL); LineTo(hdc, P[3].x, P[3].y); continue; }
                        /* P[0] - P[1] */   if (B[0] && B[1]) { MoveToEx(hdc, P[0].x, P[0].y, NULL); LineTo(hdc, P[1].x, P[1].y); }
                        /* P[1] - P[2] */   if (B[1] && B[2]) { MoveToEx(hdc, P[1].x, P[1].y, NULL); LineTo(hdc, P[2].x, P[2].y); }
                        /* P[2] - P[3] */   if (B[2] && B[3]) { MoveToEx(hdc, P[2].x, P[2].y, NULL); LineTo(hdc, P[3].x, P[3].y); }
                        /* P[0] - P[3] */   if (B[0] && B[3]) { MoveToEx(hdc, P[0].x, P[0].y, NULL); LineTo(hdc, P[3].x, P[3].y); }
                    }
                }

                X += fGstep;

                if (fabs(X) < 0.01 * fGstep) {
                    X = 0.0;
                }

                DeleteObject(mpen3);
            }

            DeleteObject(mpen1);
            DeleteObject(mpen2);
        }

        // Try dbits
        if (cloud) {
            jj = 1;
            for (i = 0; i < bih.biHeight; i++) {
                ii = 1;
                for (j = 0; j < bih.biWidth; j++) {

                    X1_ = j / mX1 + fX1min;
                    X2_ = i / mX2 + fX2min;

                    for (; ii < nX1_real - 1; ii++) {
                        if (X1_ < X1[X1_real_ind[ii]]) {
                            break;
                        }
                    }

                    for (; jj < nX2_real - 1; jj++) {
                        if (X2_ < X2[X2_real_ind[jj] * nX1]) {
                            break;
                        }
                    }

                    X = ((-X2[X2_real_ind[jj - 1] * nX1] + X2_) * (X1_ - X1[X1_real_ind[ii - 1]]) * Fun[X2_real_ind[jj] * nX1 + X1_real_ind[ii]] +
                        (X2[X2_real_ind[jj] * nX1] - X2_) * (X1_ - X1[X1_real_ind[ii - 1]]) * Fun[X2_real_ind[jj - 1] * nX1 + X1_real_ind[ii]] +
                        (X2[X2_real_ind[jj] * nX1] - X2_) * (-X1_ + X1[X1_real_ind[ii]]) * Fun[X2_real_ind[jj - 1] * nX1 + X1_real_ind[ii - 1]] +
                        (-X2[X2_real_ind[jj - 1] * nX1] + X2_) * (-X1_ + X1[X1_real_ind[ii]]) * Fun[X2_real_ind[jj] * nX1 + X1_real_ind[ii - 1]]) /
                        ((X2[X2_real_ind[jj] * nX1] - X2[X2_real_ind[jj - 1] * nX1]) * (X1[X1_real_ind[ii]] - X1[X1_real_ind[ii - 1]]));

                    hX = (X - fFmin) / (fFmax - fFmin);

                    if (hX > 1) {
                        hX = 1.0f;
                    }

                    if (hX < 0) {
                        hX = 0.0f;
                    }

                    dbits[j + i * bih.biWidth] = MakeColor(hX, Palette, 0) | object[j + i * bih.biWidth];
                }
            }
        }


        AddAviFrame(avi, hbm);
    }

    CloseAvi(avi);

    if (lines) {
        for (k = 0; k < nX1 - 1; k++) {
            delete[] MinIzo[k];
        }
        delete[] MinIzo;

        for (k = 0; k < nX1 - 1; k++) {
            delete[] MaxIzo[k];
        }
        delete[] MaxIzo;
    }


    delete[] X1_real_ind;
    delete[] X2_real_ind;

    SelectObject(hdc, holdb); SelectObject(hdc, holdp);
    DeleteDC(hdc); DeleteObject(hbm); DeleteObject(hp);
}

/////////////////////////////////////////////////////////////////////////////
// Draw the graph
/////////////////////////////////////////////////////////////////////////////
void CChild_2_Frame::OnPaint()
{
    CPaintDC dc(this);  // device context for painting
    Paint(&dc);
}

void CChild_2_Frame::Paint(CDC* pDC)
{
    const int wMin = 80, hMin = 40;
    CRect WinRec;
    GetClientRect(WinRec);  // window size

    //Calculate graph rectangle
    CRect GrafRec;
    GrafRec.left = 90;
    GrafRec.right = WinRec.right - 150;
    if (GrafRec.Width() < wMin) {
        return;
    }
    GrafRec.top = 60;
    GrafRec.bottom = WinRec.bottom - 30;
    if (GrafRec.Height() < hMin) {
        return;
    }

    pDC->TextOut(GrafRec.left + GrafRec.Width() / 2, 30, sDesc);    //X2-labels


    //Scale calculation
    float mX1 = GrafRec.Width() / (fX1max - fX1min);
    float mX2 = GrafRec.Height() / (fX2max - fX2min);

    float X;

    X = fGmin;

    int i, j, k, l;
    int ii, jj, ia, ja;
    double X1_, X2_;

    float   f[4], x1[4], x2[4];
    CPoint  P[4];
    BOOL    B[4];

    int nX1_real, nX2_real;
    int* X1_real_ind, * X2_real_ind;
    X1_real_ind = new int[nX1 + 1];
    X2_real_ind = new int[nX2 + 1];
    X1_real_ind[0] = 0;
    X2_real_ind[0] = 0;

    for (ii = 0; ii < nX1 + 1; ii++) {
        X1_real_ind[ii] = 0;
    }

    for (ii = 0; ii < nX2 + 1; ii++) {
        X2_real_ind[ii] = 0;
    }

    //Draw axis lines
    pDC->MoveTo(GrafRec.left, GrafRec.top);
    pDC->LineTo(GrafRec.left, GrafRec.bottom);
    pDC->LineTo(GrafRec.right, GrafRec.bottom);
    CString ttt;
    CSize sSize;

    int** MinIzo;
    int** MaxIzo;

    if (lines) {
        MinIzo = new int* [nX1 - 1];
        for (k = 0; k < nX1 - 1; k++) MinIzo[k] = new int[nX2 - 1];
        MaxIzo = new int* [nX1 - 1];
        for (k = 0; k < nX1 - 1; k++) MaxIzo[k] = new int[nX2 - 1];
    }

    float fMin;
    float fMax;
    float hX;
    float temp_fun;


    //Draw X2-axe 
    pDC->TextOut(5, 0, sX2Name);    //X2-axe name
    int  nX = GrafRec.Height() / hMin;
    hX = (fX2max - fX2min) / nX;
    float dd = 10 * Roundation(hX);
    hX = (Divide(hX, dd) + 1) * dd;

    X = Divide(fX2min, hX) * hX;
    hX /= 2;
    for (; X <= fX2max;) {
        if (X >= fX2min) {
            ttt.Format("%g", X);	sSize = pDC->GetTextExtent(ttt);
            j = GrafRec.bottom - (int)((X - fX2min) * mX2);
            pDC->TextOut(GrafRec.left - 15 - sSize.cx, j - 8, ttt);     //X2-labels
            pDC->MoveTo(GrafRec.left - 10, j);
            pDC->LineTo(GrafRec.left, j);                               //X2-main marks
            if (bGridL) {
                pDC->LineTo(GrafRec.right, j);                          //X2-grid lines
            }
        }
        X += hX;
        if ((X >= fX2min) && (X <= fX2max)) {
            j = GrafRec.bottom - (int)((X - fX2min) * mX2);
            pDC->MoveTo(GrafRec.left - 5, j);
            pDC->LineTo(GrafRec.left, j);                               //X2-short marks
        }
        X += hX;

        if (fabs(X) < 0.01 * hX) X = (float)0;
    }

    //Draw X1-axe 
    sSize = pDC->GetTextExtent(sX1Name);    //X1-axe name
    pDC->TextOut(WinRec.right - sSize.cx - 20, GrafRec.bottom, sX1Name);
    nX = GrafRec.Width() / wMin;
    hX = (fX1max - fX1min) / nX;
    dd = 10 * Roundation(hX);
    hX = (Divide(hX, dd) + 1) * dd;
    X = Divide(fX1min, hX) * hX;
    hX /= 2;
    for (; X <= fX1max; ) {
        if (X >= fX1min) {
            ttt.Format("%g", X);
            sSize = pDC->GetTextExtent(ttt);
            j = GrafRec.left + (int)((X - fX1min) * mX1);
            pDC->TextOut(j - sSize.cx / 2, GrafRec.bottom + 10, ttt);   //X1-labels
            pDC->MoveTo(j, GrafRec.bottom + 10);
            pDC->LineTo(j, GrafRec.bottom);                             //X1-main marks
            if (bGridL) {
                pDC->LineTo(j, GrafRec.top);                            //X1-grid lines
            }
        }
        X += hX;
        if ((X >= fX1min) && (X <= fX1max)) {
            j = GrafRec.left + (int)((X - fX1min) * mX1);
            pDC->MoveTo(j, GrafRec.bottom + 5);
            pDC->LineTo(j, GrafRec.bottom);                             //X1-short marks
        }
        X += hX;

        if (fabs(X) < 0.01 * hX) X = (float)0;
    }


    ///////////////////////////////////////////////////////////////////////
    //Draw TWO-Dim graph



    for (ii = 0, jj = 0; ii < nX1; ii++) {
        if (mX1 * (X1[ii] - X1[X1_real_ind[jj]]) > 0.5) {
            jj++;
            X1_real_ind[jj] = ii;
        }
    }
    nX1_real = jj + 1;

    for (ii = 0, jj = 0; ii < nX2; ii++) {
        if (mX2 * (X2[ii * nX1] - X2[X2_real_ind[jj] * nX1]) > 0.5) {
            jj++;
            X2_real_ind[jj] = ii;
        }
    }
    nX2_real = jj + 1;

    for (i = 0; i < (nX1_real - 1); i++) {
        for (j = 0; j < (nX2_real - 1); j++) {
            temp_fun = 0;

            for (ii = X1_real_ind[i]; ii < X1_real_ind[i + 1]; ii++) {
                for (jj = X2_real_ind[j]; jj < X2_real_ind[j + 1]; jj++) {
                    temp_fun += Fun[jj * nX1 + ii] * (X1[jj * nX1 + ii + 1] - X1[jj * nX1 + ii]) * (X2[(jj + 1) * nX1 + ii] - X2[jj * nX1 + ii]);
                }

                Fun[X2_real_ind[j] * nX1 + X1_real_ind[i]] = temp_fun / ((X1[X1_real_ind[i + 1]] - X1[X1_real_ind[i]]) * (X2[X2_real_ind[j + 1] * nX1] - X2[X2_real_ind[j] * nX1]));
            }
        }
    }

    if (lines) {
        for (i = 0; i < (nX2_real - 1); i++) {
            for (j = 0; j < (nX1_real - 1); j++) {
                fMin = FLT_MAX;
                fMax = -FLT_MAX;

                f[0] = Fun[X2_real_ind[i] * nX1 + X1_real_ind[j]];              //      P[2]
                f[1] = Fun[X2_real_ind[i] * nX1 + X1_real_ind[j + 1]];          //  !           !
                f[2] = Fun[X2_real_ind[(i + 1)] * nX1 + X1_real_ind[j]];        //  !           !
                f[3] = Fun[X2_real_ind[(i + 1)] * nX1 + X1_real_ind[j + 1]];    // f[0]---*---f[1]
                for (k = 0; k < 4; k++) {
                    if (f[k] < fMin) fMin = f[k];
                    if (f[k] > fMax) fMax = f[k];
                }
                MinIzo[j][i] = (int)((fMin - fGmin) / fGstep - 2);
                MaxIzo[j][i] = (int)((fMax - fGmin) / fGstep + 2);
            }
        }
    }

    ///////////////////////////////
    // Make a DI Bitmap for memory painting
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

    DWORD* dbits = (DWORD*)bits;

    CDC memDC;
    CBitmap MemBit;

    memDC.CreateCompatibleDC(pDC);

    memDC.SelectObject(hbm);

    COLORREF color = ColorMin;
    int	n_hR, n_hG, n_hB;
    if (nGrids > 0) {
        n_hR = 5 * (GetRValue(ColorMax) - GetRValue(ColorMin)) / nGrids;
        n_hG = 5 * (GetGValue(ColorMax) - GetGValue(ColorMin)) / nGrids;
        n_hB = 5 * (GetBValue(ColorMax) - GetBValue(ColorMin)) / nGrids;
        if (GrafRec.Height() / nGrids >= hMin / 2) {
            l = 1;                                                  //legend for every grid
        } else {
            l = (nGrids / (10 * GrafRec.Height() / hMin) + 1) * 5;  //legend for several grids
        }
    }

    //////////////////
    if (cloud) {
        jj = 1;
        for (i = 0; i < GrafRec.Height(); i++) {
            ii = 1;
            for (j = 0; j < GrafRec.Width(); j++) {
                X1_ = j / mX1 + fX1min;
                X2_ = i / mX2 + fX2min;

                for (; ii < nX1_real - 1; ii++) {
                    if (X1_ < X1[X1_real_ind[ii]]) {
                        break;
                    }
                }

                for (; jj < nX2_real - 1; jj++) {
                    if (X2_ < X2[X2_real_ind[jj] * nX1]) {
                        break;
                    }
                }

                X = ((-X2[X2_real_ind[jj - 1] * nX1] + X2_) * (X1_ - X1[X1_real_ind[ii - 1]]) * Fun[X2_real_ind[jj] * nX1 + X1_real_ind[ii]] +
                    (X2[X2_real_ind[jj] * nX1] - X2_) * (X1_ - X1[X1_real_ind[ii - 1]]) * Fun[X2_real_ind[jj - 1] * nX1 + X1_real_ind[ii]] +
                    (X2[X2_real_ind[jj] * nX1] - X2_) * (-X1_ + X1[X1_real_ind[ii]]) * Fun[X2_real_ind[jj - 1] * nX1 + X1_real_ind[ii - 1]] +
                    (-X2[X2_real_ind[jj - 1] * nX1] + X2_) * (-X1_ + X1[X1_real_ind[ii]]) * Fun[X2_real_ind[jj] * nX1 + X1_real_ind[ii - 1]]) /
                    ((X2[X2_real_ind[jj] * nX1] - X2[X2_real_ind[jj - 1] * nX1]) * (X1[X1_real_ind[ii]] - X1[X1_real_ind[ii - 1]]));
                hX = (X - fFmin) / (fFmax - fFmin);

                if (hX > 1.0f) {
                    hX = 1.0f;
                }

                if (hX < 0.0f) {
                    hX = 0.0f;
                }

                dbits[j + i * bih.biWidth] = dbits[j + i * bih.biWidth] | MakeColor(hX, Palette, 0);
            }

            hX = static_cast<float>(i) / GrafRec.Height();

            for (j = GrafRec.Width(); j < GrafRec.Width() + 5; j++) {
                dbits[j + i * bih.biWidth] = 256 * 256 * 255 + 255 * 256 + 255;

            }

            for (j = GrafRec.Width() + 5; j < GrafRec.Width() + 20; j++) {
                X1_ = j / mX1 + fX1min;
                X2_ = i / mX2 + fX2min;
                for (; ii < nX1_real - 1; ii++) {
                    if (X1_ < X1[X1_real_ind[ii]]) {
                        break;
                    }
                }

                for (; jj < nX2_real - 1; jj++) {
                    if (X2_ < X2[X2_real_ind[jj] * nX1]) {
                        break;
                    }
                }

                dbits[j + i * bih.biWidth] = dbits[j + i * bih.biWidth] | MakeColor(hX, Palette, 0);
            }
        }

        pDC->BitBlt(GrafRec.left + 1, GrafRec.top, GrafRec.Width() + 19, GrafRec.Height(),
            &memDC, 1, WinRec.Height() - GrafRec.Height(), SRCCOPY);

        pDC->MoveTo(GrafRec.left + GrafRec.Width() + 5, GrafRec.top);
        pDC->LineTo(GrafRec.left + GrafRec.Width() + 5, GrafRec.bottom);
        pDC->LineTo(GrafRec.left + GrafRec.Width() + 20, GrafRec.bottom);
        pDC->LineTo(GrafRec.left + GrafRec.Width() + 20, GrafRec.top);
        pDC->LineTo(GrafRec.left + GrafRec.Width() + 5, GrafRec.top);

        X = fGmin;
        for (nX = 0; nX < nGrids; nX++) {
            //Draw legend
            if ((nX != 0) && ((nX / l) * l == nX)) {
                k = GrafRec.bottom - (int)((nX + 0.5) * GrafRec.Height() / nGrids);
                ttt.Format("%g", X);
                pDC->TextOut(GrafRec.right + 21, k, ttt);
            }
            X += fGstep; if (fabs(X) < 0.01 * fGstep) X = (float)0;
        }

        ttt.Format("MIN: %7.2g", frealmin);
        pDC->TextOut(GrafRec.right + 21, GrafRec.bottom - 20, ttt);
        ttt.Format("MAX: %7.2g", frealmax);
        pDC->TextOut(GrafRec.right + 21, GrafRec.top, ttt);
    }

    CPen myPen(PS_SOLID, 1, ColorMin);          //change pen to ColorMin-pen
    CPen* oldPen = pDC->SelectObject(&myPen);

    if (lines) {
        X = fGmin;
        for (nX = 0; (int)nX < nGrids; nX++) {
            //Prepare pen
            k = nX - (int)((nX) / 5) * 5;
            pDC->SelectObject(oldPen);
            myPen.DeleteObject();
            myPen.CreatePen(iStyle[k], 1, color);
            pDC->SelectObject(&myPen);
            if (k == 4) {
                color = RGB(GetRValue(color) + n_hR, GetGValue(color) + n_hG, GetBValue(color) + n_hB); //change color
            }

            //Draw legend
            if ((nX != 0) && ((nX / l) * l == nX)) {
                k = GrafRec.bottom - (int)((nX + 0.5) * GrafRec.Height() / nGrids);
                pDC->MoveTo(WinRec.right, k);
                pDC->LineTo(GrafRec.right, k);
                ttt.Format("%g", X);
                pDC->TextOut(GrafRec.right + 10, k + 5, ttt);
            }
            ttt.Format("MIN: %7.2g", frealmin);
            pDC->TextOut(GrafRec.right + 10, GrafRec.bottom - 20, ttt);
            ttt.Format("MAX: %7.2g", frealmax);
            pDC->TextOut(GrafRec.right + 10, GrafRec.top, ttt);

            for (i = 0; i < (nX2_real - 1); i++) {
                for (j = 0; j < (nX1_real - 1); j++) {
                    //Find izoline points
                    if (!((nX >= MinIzo[j][i]) && (nX <= MaxIzo[j][i]))) {
                        continue;
                    }
                    for (k = 0; k < 4; k++) B[k] = FALSE;
                    ii = i;
                    jj = j;
                    i = X2_real_ind[ii];
                    j = X1_real_ind[jj];
                    ia = X2_real_ind[ii + 1];
                    ja = X1_real_ind[jj + 1];

                    f[0] = Fun[i * nX1 + j] - X;    //            P[2]
                    x1[0] = X1[i * nX1 + j];        //      f[2]---*---f[3]
                    x2[0] = X2[i * nX1 + j];        //       !           !
                    f[1] = Fun[i * nX1 + ja] - X;   //       !           !
                    x1[1] = X1[i * nX1 + ja];       //       !           !
                    x2[1] = X2[i * nX1 + ja];       //  P[1] *           * P[3]
                    f[2] = Fun[(ia)*nX1 + j] - X;   //       !           !
                    x1[2] = X1[(ia)*nX1 + j];       //       !           !
                    x2[2] = X2[(ia)*nX1 + j];       //       !           !
                    f[3] = Fun[(ia)*nX1 + ja] - X;  //      f[0]---*---f[1]
                    x1[3] = X1[(ia)*nX1 + ja];      //            P[0]
                    x2[3] = X2[(ia)*nX1 + ja];

                    i = ii;
                    j = jj;

                    if ((f[0] * f[1] <= 0) && (f[1] != f[0])) {
                        //find P[0] between x1,x2[0] and x1,x2[1]
                        hX = (float)(fabs(f[0] / (f[1] - f[0])));
                        P[0].x = GrafRec.left + (int)(mX1 * (x1[0] + hX * (x1[1] - x1[0]) - fX1min));
                        P[0].y = GrafRec.bottom - (int)(mX2 * (x2[0] + hX * (x2[1] - x2[0]) - fX2min));
                        B[0] = TRUE;
                    }

                    if ((f[0] * f[2] <= 0) && (f[0] != f[2])) {
                        //find P[1] between x1,x2[0] and x1,x2[2]
                        hX = (float)(fabs(f[0] / (f[2] - f[0])));
                        P[1].x = GrafRec.left + (int)(mX1 * (x1[0] + hX * (x1[2] - x1[0]) - fX1min));
                        P[1].y = GrafRec.bottom - (int)(mX2 * (x2[0] + hX * (x2[2] - x2[0]) - fX2min));
                        B[1] = TRUE;
                    }

                    if ((f[2] * f[3] <= 0) && (f[3] != f[2])) {
                        //find P[2] between x1,x2[2] and x1,x2[3]
                        hX = (float)(fabs(f[2] / (f[3] - f[2])));
                        P[2].x = GrafRec.left + (int)(mX1 * (x1[2] + hX * (x1[3] - x1[2]) - fX1min));
                        P[2].y = GrafRec.bottom - (int)(mX2 * (x2[2] + hX * (x2[3] - x2[2]) - fX2min));
                        B[2] = TRUE;
                    }

                    if ((f[1] * f[3] <= 0) && (f[1] != f[3])) {
                        //find P[3] between x1,x2[1] and x1,x2[3]
                        hX = (float)(fabs(f[1] / (f[3] - f[1])));
                        P[3].x = GrafRec.left + (int)(mX1 * (x1[1] + hX * (x1[3] - x1[1]) - fX1min));
                        P[3].y = GrafRec.bottom - (int)(mX2 * (x2[1] + hX * (x2[3] - x2[1]) - fX2min));
                        B[3] = TRUE;
                    }
                    //Draw izolines:
                    /* P[0] - P[2] */   if (B[0] && B[2] && !B[1]) { pDC->MoveTo(P[0]); pDC->LineTo(P[2]); continue; }
                    /* P[1] - P[3] */   if (B[1] && B[3] && !B[0]) { pDC->MoveTo(P[1]); pDC->LineTo(P[3]); continue; }
                    /* P[0] - P[1] */   if (B[0] && B[1]) { pDC->MoveTo(P[0]); pDC->LineTo(P[1]); }
                    /* P[1] - P[2] */   if (B[1] && B[2]) { pDC->MoveTo(P[1]); pDC->LineTo(P[2]); }
                    /* P[2] - P[3] */   if (B[2] && B[3]) { pDC->MoveTo(P[2]); pDC->LineTo(P[3]); }
                    /* P[0] - P[3] */   if (B[0] && B[3]) { pDC->MoveTo(P[0]); pDC->LineTo(P[3]); }
                }
            }

            X += fGstep;

            if (fabs(X) < 0.01 * fGstep) {
                X = 0.0;
            }
        }
        pDC->SelectObject(oldPen);
        myPen.DeleteObject();

        for (k = 0; k < nX1 - 1; k++) {
            delete[] MinIzo[k];
        }
        delete[] MinIzo;

        for (k = 0; k < nX1 - 1; k++) {
            delete[] MaxIzo[k];
        }
        delete[] MaxIzo;
    } // end lines*/

    delete[] X1_real_ind;
    delete[] X2_real_ind;
}


/////////////////////////////////////////////////////////////////////////////
// Draw nearest graph point on Left mouse button down
void CChild_2_Frame::OnLButtonDown(UINT nFlags, CPoint point)
{
    Invalidate();		// redraw
    const int wMin = 80, hMin = 40;
    CPaintDC dc(this);	// device context for painting -not used

//Clear status bar
    CMainFrame* pWnd = (CMainFrame*)AfxGetMainWnd();
    pWnd->pStatusBar->SetText("", 1, 0);
    pWnd->pStatusBar->SetText("", 2, 0);
    pWnd->pStatusBar->SetText("", 3, 0);

    //Calculate graph rectangle
    CRect       WinRec;	GetClientRect(WinRec);	// window size	
    CRect 		GrafRec;


    GrafRec.left = 90; GrafRec.right = WinRec.right - 150;
    if (GrafRec.Width() < wMin) return;
    GrafRec.top = 60; GrafRec.bottom = WinRec.bottom - 30;
    if (GrafRec.Height() < hMin) return;



    //////////////////////////////////////////////////////
    //Find and drow nearest point 
    if (GrafRec.PtInRect(point))
    {
        float x1, x2;
        int i, j;
        CString ttt;
        //Scale calculation
        float mX1 = GrafRec.Width() / (fX1max - fX1min);
        float mX2 = GrafRec.Height() / (fX2max - fX2min);
        //Find point x1,x2 (in phisical coorginates)
        x1 = fX1min + (point.x - GrafRec.left) / mX1;
        x2 = fX2max - (point.y - GrafRec.top) / mX2;

        if (!bPolar) // for decart coordinates	
        {
            //Find nearest X1[i] point
            for (i = 0; i < nX1; i++) if (X1[i] > x1) break; i--;
            if (i < 0)   i = 0;
            else if (((i + 1) < nX1) && ((X1[i + 1] - x1) < (x1 - X1[i]))) i++;
            //Find nearest X2[j] point
            for (j = 0; j < nX2; j++) if (X2[j * nX1] > x2) break; j--;
            if (j < 0)   j = 0;
            else if (((j + 1) < nX2) && ((X2[(j + 1) * nX1] - x2) < (x2 - X2[j * nX1]))) j++;
            //Global index
            i = j * nX1 + i;
        } else					 // for polar coordinates
        {
            //Find nearest X1[i],X2[i] point
            i = 0;
            double delta = sqrt((X1[i] - x1) * (X1[i] - x1) + (X2[i] - x2) * (X2[i] - x2));
            for (j = 1; j < nX1 * nX2; j++)
            {
                double delta_n = sqrt((X1[j] - x1) * (X1[j] - x1) + (X2[j] - x2) * (X2[j] - x2));
                if (delta_n < delta) { i = j; delta = delta_n; }
            }
        }

        //Prepare X1 status bar sell (1)
        ttt.Format("%g", X1[i]);
        if (sX1Name.GetLength() <= 8)
            pWnd->pStatusBar->SetText(sX1Name + ": " + ttt, 1, 0);
        else
            pWnd->pStatusBar->SetText(ttt, 1, 0);
        //Prepare X2 status bar sell (2) ttt is ready
        ttt.Format("%g", X2[i]);
        if (sX2Name.GetLength() <= 8)
            pWnd->pStatusBar->SetText(sX2Name + ": " + ttt, 2, 0);
        else
            pWnd->pStatusBar->SetText(ttt, 2, 0);
        //Prepare Fun status bar sell (3)
        ttt.Format("%g", Fun[i]);
        if (sFName.GetLength() <= 8)
            pWnd->pStatusBar->SetText(sFName + ": " + ttt, 3, 0);
        else
            pWnd->pStatusBar->SetText(ttt, 3, 0);

        //Find nearest graph point (i,j)
        j = GrafRec.bottom - (int)(mX2 * (X2[i] - fX2min));
        i = GrafRec.left + (int)(mX1 * (X1[i] - fX1min));
        //Draw nearest point
        dc.Ellipse(i - 5, j - 5, i + 5, j + 5);
        dc.MoveTo(GrafRec.left, j);
        dc.LineTo(i, j);
        dc.LineTo(i, GrafRec.bottom);
    }
    Invalidate(FALSE);	//Don't redraw
}

void CChild_2_Frame::OnSaveWmf()
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

    memDC.SelectObject(hbm);

    memDC.BitBlt(0, 0, WinRec.Width(), WinRec.Height(), pDC, 0, 0, SRCCOPY);

    PBITMAPINFO info = CreateBitmapInfoStruct(NULL, hbm);

    CString name, nm;
    nm = sDesc;
    nm.Replace('|', ' ');
    nm.Replace(',', ' ');
    nm.Replace('.', ' ');

    CMainFrame* pWnd = (CMainFrame*)AfxGetMainWnd();
    MainDir = pWnd->MainDir;

    name.Format("%spictures\\%d - %s.bmp", MainDir, PicsTotal, nm);
    PicsTotal++;

    CreateDirectory(MainDir + "pictures", NULL);
    CreateBMPFile(NULL, name.GetBuffer(0), info, hbm, pDC->m_hDC);
}

int CChild_2_Frame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

BOOL CChild_2_Frame::SetPixelFormat(HDC hdc)
{
    // Заполняем поля структуры
    static PIXELFORMATDESCRIPTOR pfd = {
      sizeof(PIXELFORMATDESCRIPTOR),  // размер структуры
      1,                              // номер версии
      PFD_DRAW_TO_WINDOW |          // поддержка вывода в окно
      PFD_SUPPORT_OPENGL |          // поддержка OpenGL
      PFD_DOUBLEBUFFER |           // двойная буферизация
      PFD_TYPE_RGBA,         // цвета в режиме RGBA
      24,                    // 24-разряда на цвет
      0, 0, 0, 0, 0, 0,      // биты цвета игнорируются
      0,                     // не используется альфа параметр
      0,                     // смещение цветов игнорируются
      0,                     // буфер аккумулятора не используется
      0, 0, 0, 0,            // биты аккумулятора игнорируются
      32,                    // 32-разрядный буфер глубины
      0,                     // буфер трафарета не используется
      0,                     // вспомогательный буфер не используется
      PFD_MAIN_PLANE,        // основной слой
      0,                     // зарезервирован
      0, 0, 0                // маски слоя игнорируются
    };

    int pixelFormat;

    // Поддерживает ли система необходимый формат пикселей?
    if ((pixelFormat = ::ChoosePixelFormat(hdc, &pfd)) == 0) {
        MessageBox("С заданным форматом пикселей работать нельзя");
        return FALSE;
    }

    if (::SetPixelFormat(hdc, pixelFormat, &pfd) == FALSE)
    {
        MessageBox("Ошибка при выполнении SetPixelFormat");
        return FALSE;
    }

    return TRUE;
}

void CChild_2_Frame::OnSize(UINT nType, int cx, int cy)
{
    CMDIChildWnd::OnSize(nType, cx, cy);
}

void CChild_2_Frame::OnPhotoFilm()
{
    PicsTotal = 100;
    int ind;
    while (1 == 1)
    {
        ind = INDEX[pProject->m_field[nField].nAxis] += 9;
        if (ind >= pProject->m_nTimePoints - 1) return;
        PrepareData_time(1);
        OnPaint();
        Invalidate();
        OnSaveWmf();
    }
}

void CChild_2_Frame::OnResize()
{
    SetWindowPos(NULL, 0, 0, 640, 480, SWP_SHOWWINDOW | SWP_DRAWFRAME | SWP_NOOWNERZORDER | SWP_NOMOVE);
}

void CChild_2_Frame::OnTempmax()
{
    PrepareData_max();
}
