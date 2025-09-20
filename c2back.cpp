/////////////////////////////////////////////////////////////////////////////
// Child_2.cpp : implementation of the CChild_2_Frame class
//

#include "stdafx.h"
#include <fstream>
#include <math.h>

#include <vfw.h>
#include <winuser.h>

#include "windows.h"

#include "video.h"
#include "MainFrm.h"
#include "F_2_dlg.h"
#include "Child_2.h"
#include "AviDlg.h"
#include <afxext.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "avi_utils.h"




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
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChild_2_Frame constructors/destructors

CChild_2_Frame::CChild_2_Frame() //standart constructor - not used
{
    PicsTotal = 0;
}

// CChild_2_Frame main constructor
CChild_2_Frame::CChild_2_Frame(CField_2_Dlg *pDlg)
{
    CMainFrame *pWnd = (CMainFrame *)AfxGetMainWnd();
    MainDir = pWnd->MainDir;
    INDEX = NULL;
    Fun = NULL;
    X1 = NULL;
    X2 = NULL;
    LAYER_COL[0] = 0;//RGB(255,255,255);
    LAYER_COL[1] = 256 * 256 * (int)(200) + 256 * (int)(200) + (int)(250);//RGB(0, 0, 0);
    LAYER_COL[2] = RGB(231, 123, 5);
    LAYER_COL[3] = RGB(83, 136, 154);
    LAYER_COL[4] = RGB(45, 196, 154);
    LAYER_COL[5] = RGB(155, 31, 182);
    LAYER_COL[6] = RGB(191, 20, 68);
    LAYER_COL[7] = RGB(129, 96, 46);
    LAYER_COL[8] = RGB(226, 221, 16);


    //	AfxMessageBox("Эта версия делит массив N на объем ячейки\n");

    PrepareData(pDlg);

}

CChild_2_Frame::~CChild_2_Frame()
{
    if (Fun)   delete[] Fun;
    if (X1)    delete[] X1;
    if (X2)    delete[] X2;
    if (INDEX) delete[] INDEX;
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

/////////////////////////////////////////////////////////////////////////////
// PrepareData - Main CChild_2_Frame procedure:
//		preparing all data from dialog;
//		change window name;
//		find function values;
//		recalculate function Min and Max;
//		recalculate min,max for draw axes.
void CChild_2_Frame::PrepareData(CField_2_Dlg *pDlg)
{
    int i;

    // Take data from CField_2_Dlg
    pProject = pDlg->pProject;
    nField = pDlg->m_nField;
    fFmin = pDlg->m_fFmin;
    fFmax = pDlg->m_fFmax;
    bPolar = pDlg->m_bPolar;
    nArg1 = pDlg->m_nArg1;
    nX1min = pDlg->m_nX1min;
    nX1max = pDlg->m_nX1max;
    nArg2 = pDlg->m_nArg2;
    nX2min = pDlg->m_nX2min;
    nX2max = pDlg->m_nX2max;
    nGrids = pDlg->m_nGrids;
    bGridL = pDlg->m_bGridLines;
    ColorMax = pDlg->ColorMax;
    ColorMin = pDlg->ColorMin;

    layers = pDlg->m_layers;
    cloud = pDlg->m_cloud;
    lines = pDlg->m_lines;

    if (INDEX) delete[] INDEX;
    INDEX = new int[pProject->m_field[nField].nAxis + 1];
    for (i = 0; i <= pProject->m_field[nField].nAxis; i++)
    {
        if ((int)i == nArg1)	INDEX[i] = nX1min;
        else if ((int)i == nArg2)	INDEX[i] = nX2min;
        else						INDEX[i] = pDlg->INDEX[i];
    }

    //Change window name
    sName = "Two-dim graph " + pProject->m_field[nField].name;

    if (nArg1 != (int)pProject->m_field[nField].nAxis)	//X1 is axe
        sName += '(' + pProject->m_field[nField].axe[nArg1].name + ',';
    else												//X1 is time
        sName += '(' + pProject->m_sTime + ',';

    if (nArg2 != (int)pProject->m_field[nField].nAxis)  //X2 is axe
        sName += pProject->m_field[nField].axe[nArg2].name + ')';
    else												//X2 is time
        sName += pProject->m_sTime + ')';

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

    // Memory operations
    if (Fun) delete[] Fun;
    if (X1)  delete[] X1;
    if (X2)  delete[] X2;
    nX1 = nX1max - nX1min + 1;
    nX2 = nX2max - nX2min + 1;
    Fun = new float[nX1*nX2];
    X1 = new float[nX1*nX2];
    X2 = new float[nX1*nX2];

    // First index
    k = INDEX[pProject->m_field[nField].nAxis - 1];
    for (l = pProject->m_field[nField].nAxis - 2; l >= 0; l--)
        k = INDEX[l] + k*pProject->m_field[nField].axe[l].nPoints;

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
        kst2 = pProject->m_field[nField].nPoints - (nX1 - 1)*kst1;
    else												 //X2 is axe
    {
        kst2 = 1;
        for (l = 0; l < nArg2; l++)
            kst2 *= pProject->m_field[nField].axe[l].nPoints;
        kst2 -= (nX1 - 1)*kst1;
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

    pProject->read_points(nField, k, kst1, nX1, kst2 - kst1, nX2, Fun);  //Function value

    for (j = nX2min; (int)j <= nX2max; j++)
    {
        for (i = nX1min; (int)i <= nX1max; i++)
        {
            l = i - nX1min + (j - nX2min)*nX1;

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
                Fun[j* nX1 + i] = Fun[j* nX1 + i] /
                    ((X1[i + 1] - X1[i])*(X2[(j + 1)*nX1] - X2[j*nX1]));
            }
            Fun[j* nX1 + i] = Fun[j* nX1 + i] /
                ((X1[i] - X1[i - 1])*(X2[(j + 1)*nX1] - X2[j*nX1]));
        }
        for (j = 0; j < nX2 - 1; j++)
            Fun[j* nX1 + i] = Fun[j* nX1 + i] /
            ((X1[i] - X1[i - 1])*(X2[(j + 1)*nX1] - X2[(j)*nX1]));

        Fun[j* nX1 + i] = Fun[j* nX1 + i] /
            ((X1[i] - X1[i - 1])*(X2[(j)*nX1] - X2[(j - 1)*nX1]));

    }

    //Recalculate fMin,fMax
    float fMin = 3.402823466e+38F;
    float fMax = -3.402823466e+38F;
    for (j = 0; j < nX1*nX2; j++)
    {
        if (Fun[j] > fMax) fMax = Fun[j];
        if (Fun[j] < fMin) fMin = Fun[j];
    }
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
        fX1min = fX2min = 3.402823466e+38F;
        fX1max = fX2max = -3.402823466e+38F;

        for (i = 0; i < (int)(nX1*nX2); i++)
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
    dx = (float)0.1*Roundation(fX1max - fX1min);
    fX1max = (Divide(fX1max, dx) + 1) * dx;
    fX1min = Divide(fX1min, dx)    * dx;

    if (fX2max <= fX2min) fX2max = fX2min + 1;
    dx = (float)0.1*Roundation(fX2max - fX2min);
    fX2max = (Divide(fX2max, dx) + 1) * dx;
    fX2min = Divide(fX2min, dx)    * dx;

    //Recalculate grid minimun and nGrids for draw TWO-Dim graph
        /*
        dx=10*Roundation((fFmax-fFmin)/nGrids);
        fGstep=(Divide(((fFmax-fFmin)/nGrids),dx)+1) * dx;
        fGmin = Divide((fFmin+5*fGstep),10*fGstep)*10*fGstep;
        nGrids= Divide((fFmax-fGmin),fGstep)+1;
        if(	fFmax==fFmin ) nGrids=0;
        */
    fGmin = Scaling(nGrids, fFmax, fFmin, &fGstep);

}
void CChild_2_Frame::PrepareData_time(BOOL MinMax)
{

    INDEX[pProject->m_field[nField].nAxis] += 1;


    //Prepare Function values TWO-Dim Fun(X1,X2)
    //------------------------------------------
    int j;
    int  l, k, kst1, kst2;

    // Memory operations
    //	if (Fun) delete [] Fun;
    //	if (X1)  delete [] X1;
    //	if (X2)  delete [] X2;
    nX1 = nX1max - nX1min + 1;
    nX2 = nX2max - nX2min + 1;
    //	Fun = new float[nX1*nX2];
    //	X1  = new float[nX1*nX2];
    //	X2  = new float[nX1*nX2];

    // First index
    k = INDEX[pProject->m_field[nField].nAxis - 1];
    for (l = pProject->m_field[nField].nAxis - 2; l >= 0; l--)
        k = INDEX[l] + k*pProject->m_field[nField].axe[l].nPoints;

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
        kst2 = pProject->m_field[nField].nPoints - (nX1 - 1)*kst1;
    else												 //X2 is axe
    {
        kst2 = 1;
        for (l = 0; l < nArg2; l++)
            kst2 *= pProject->m_field[nField].axe[l].nPoints;
        kst2 -= (nX1 - 1)*kst1;
    }

    //Search function values 

    pProject->read_points(nField, k, kst1, nX1, kst2 - kst1, nX2, Fun);  //Function value
// I don't need this, since it doesn't change 
/*
    for(j=nX2min; (int)j<=nX2max; j++)
    {
        for(i=nX1min; (int)i<=nX1max; i++)
        {
            l=i-nX1min+(j-nX2min)*nX1;

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

    */
    if (pProject->m_field[nField].name == "N")
    {
        // Podelit' na razmer jachejki
        int i;
        for (i = 0; i < nX1 - 1; i++)
        {
            for (j = 0; j < nX2 - 1; j++)
            {
                Fun[j* nX1 + i] = Fun[j* nX1 + i] /
                    ((X1[i + 1] - X1[i])*(X2[(j + 1)*nX1] - X2[j*nX1]));
            }
            Fun[j* nX1 + i] = Fun[j* nX1 + i] /
                ((X1[i] - X1[i - 1])*(X2[(j + 1)*nX1] - X2[j*nX1]));
        }
        for (j = 0; j < nX2 - 1; j++)
            Fun[j* nX1 + i] = Fun[j* nX1 + i] /
            ((X1[i] - X1[i - 1])*(X2[(j + 1)*nX1] - X2[(j)*nX1]));

        Fun[j* nX1 + i] = Fun[j* nX1 + i] /
            ((X1[i] - X1[i - 1])*(X2[(j)*nX1] - X2[(j - 1)*nX1]));
    }
    if (!MinMax)
    {
        //Recalculate fMin,fMax
        float fMin = 3.402823466e+38F;
        float fMax = -3.402823466e+38F;
        for (j = 0; j < nX1*nX2; j++)
        {
            if (Fun[j] > fMax) fMax = Fun[j];
            if (Fun[j] < fMin) fMin = Fun[j];
        }

        fFmax = fMax;
        fFmin = fMin;
    }
    //I do not need this either 
        /*
    //Prepare Polar coordinates
    //-------------------------
        if(bPolar)
        {
            CString ttt     = sX2Name+"*COS("+sX1Name+")";// Axes names
                    sX2Name = sX2Name+"*SIN("+sX1Name+")";
                    sX1Name = ttt;

            float   x1,x2;
            fX1min = fX2min =  3.402823466e+38F;
            fX1max = fX2max = -3.402823466e+38F;

            for( i=0; i<(int)(nX1*nX2); i++ )
            {
                x1 = X1[i]*(float)cos(X2[i]);//X1-argument
                if( x1 > fX1max ) fX1max=x1;
                if( x1 < fX1min ) fX1min=x1;
                x2 = X1[i]*(float)sin(X2[i]);//X2-argument
                if( x2 > fX2max ) fX2max=x2;
                if( x2 < fX2min ) fX2min=x2;
                X1 [i] = x1;
                X2 [i] = x2;
            }
        }

        else
    //Prepare decart coordinates
    //--------------------------
        {
    //Calculate X1 min,max
            if (nArg1 != (int)pProject->m_field[nField].nAxis)   //X1 is axe
            {
                fX1max=pProject->m_field[nField].axe[nArg1].point[nX1max];
                fX1min=pProject->m_field[nField].axe[nArg1].point[nX1min];
            }
            else												 //X1 is time
            {
                fX1max=pProject->m_time[nX1max];
                fX1min=pProject->m_time[nX1min];
            }
    //Calculate X2 min,max
            if (nArg2 != (int)pProject->m_field[nField].nAxis)   //X2 is axe
            {
                fX2max=pProject->m_field[nField].axe[nArg2].point[nX2max];
                fX2min=pProject->m_field[nField].axe[nArg2].point[nX2min];
            }
            else												 //X2 is time
            {
                fX2max=pProject->m_time[nX2max];
                fX2min=pProject->m_time[nX2min];
            }
        }

    //Recalculate X1,X2 min,max for draw
        float dx;

        if(	fX1max<=fX1min ) fX1max=fX1min+1;
        dx=(float)0.1*Roundation(fX1max-fX1min);
        fX1max=(Divide(fX1max,dx)+1) * dx;
        fX1min= Divide(fX1min,dx)    * dx;

        if(	fX2max<=fX2min ) fX2max=fX2min+1;
        dx=(float)0.1*Roundation(fX2max-fX2min);
        fX2max=(Divide(fX2max,dx)+1) * dx;
        fX2min= Divide(fX2min,dx)    * dx;

        */

        //Recalculate grid minimun and nGrids for draw TWO-Dim graph
            /*
            dx=10*Roundation((fFmax-fFmin)/nGrids);
            fGstep=(Divide(((fFmax-fFmin)/nGrids),dx)+1) * dx;
            fGmin = Divide((fFmin+5*fGstep),10*fGstep)*10*fGstep;
            nGrids= Divide((fFmax-fGmin),fGstep)+1;
            if(	fFmax==fFmin ) nGrids=0;
            */
    fGmin = Scaling(nGrids, fFmax, fFmin, &fGstep);

}
/////////////////////////////////////////////////////////////////////////////
// CChild_2_Frame message handlers

void CChild_2_Frame::OnKillFocus(CWnd* pNewWnd)
{
    Invalidate();//redraw
    CMDIChildWnd::OnKillFocus(pNewWnd);

    // Clearning status bar
    CMainFrame *pWnd = (CMainFrame *)AfxGetMainWnd();
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
    dlg.m_layers = layers;


    int i;
    dlg.INDEX = new int[pProject->m_field[nField].nAxis + 1];
    for (i = 0; i <= pProject->m_field[nField].nAxis; i++)
        dlg.INDEX[i] = INDEX[i];

    //Do CField_2_Dlg "Graph->Modify"
    if (dlg.DoModal() != IDOK)return;

    //Change current values
    PrepareData(&dlg);
    SetWindowText(sName);
    Invalidate();
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

    CFileDialog dlg(FALSE, "dat", sName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "Data files (*.dat)|*.dat||");
    dlg.m_ofn.lpstrInitialDir = MainDir;
    if (dlg.DoModal() != IDOK)return;	//Take filename from 'SAVE AS' dialog
    std::fstream file;
    file.open(dlg.GetPathName(), std::ios::out);
    int i;

    file << sX1Name << "  " << sX2Name << "  " << sFName << "\n";//Title
    for (i = 0; i < nX1*nX2; i++)
        if ((Fun[i] >= fFmin) && (Fun[i] <= fFmax))
            file << X1[i] << "  " << X2[i] << "  " << Fun[i] << "\n";//Data
    file.close();


}

/////////////////////////////////////////////////////////////////////////////
void CChild_2_Frame::OnFileSavepict()
{

    // Здесь частично использован код с сайта http://www.wischik.com/lu/programmer/avi_utils.html
    // Этот код распространяется свободно:
    //(c) 2002 Lucian Wischik. This code is free, and anyone can do with it whatever they like
    //(except sell it or claim ownership).


    CAviDlg Dlg(this);

    if ((nArg1 == (int)pProject->m_field[nField].nAxis) || (nArg2 == (int)pProject->m_field[nField].nAxis))
    {
        AfxMessageBox("Для того, чтобы сохранить видео-файл время должно быть свободно\n", MB_OK);
        return;
    }

    //Dlg.
    Dlg.m_FileName = "test.avi";
    if (Dlg.DoModal() != IDOK) return;



    CDC *cdcscreen = GetDC();
    HDC hdcscreen = cdcscreen->m_hDC, hdc = CreateCompatibleDC(hdcscreen); ReleaseDC(cdcscreen);
    BITMAPINFO bi; ZeroMemory(&bi, sizeof(bi)); BITMAPINFOHEADER &bih = bi.bmiHeader;
    bih.biSize = sizeof(bih);
    bih.biWidth = 800;
    bih.biHeight = 600;
    bih.biPlanes = 1;
    bih.biBitCount = 32;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = ((bih.biWidth*bih.biBitCount / 8 + 3) & 0xFFFFFFFC)*bih.biHeight;
    bih.biXPelsPerMeter = 10000;
    bih.biYPelsPerMeter = 10000;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;
    void *bits; HBITMAP hbm = CreateDIBSection(hdc, (BITMAPINFO*)&bih, DIB_RGB_COLORS, &bits, NULL, NULL);
    //
    HGDIOBJ holdb = SelectObject(hdc, hbm);
    HPEN hp = CreatePen(PS_SOLID, 16, RGB(255, 255, 128));
    HGDIOBJ holdp = SelectObject(hdc, hp);


    //
    HAVI avi = CreateAvi(Dlg.m_FileName, 100, NULL);
    INDEX[pProject->m_field[nField].nAxis] = Dlg.start_frame - 1; // I will add 1 later

    // Trying bits
    DWORD *dbits = (DWORD*)bits;
    DWORD *object = new DWORD[bih.biHeight*bih.biWidth];

    // 



    CRect 		GrafRec;
    GrafRec.left = 0; GrafRec.right = 800;
    //if(GrafRec.Width()  < wMin) return;
    GrafRec.top = 0; GrafRec.bottom = 600;
    //if(GrafRec.Height() < hMin) return;

    BOOL lines;
    lines = FALSE;

    int nX1_real, nX2_real;
    int *X1_real_ind, *X2_real_ind;
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
        if (mX1*(X1[ii] - X1[X1_real_ind[jj]]) > 1)
        {
            jj++;
            X1_real_ind[jj] = ii;
        }
    nX1_real = jj + 1;
    for (ii = 0, jj = 0; ii < nX2; ii++)
        if (mX2*(X2[ii    * nX1] - X2[X2_real_ind[jj] * nX1]) > 1)
        {
            jj++;
            X2_real_ind[jj] = ii;
        }
    nX2_real = jj + 1;

    float temp_fun;




    if (lines)
    {
        MinIzo = new int*[nX1_real - 1];
        for (k = 0; k < nX1_real - 1; k++) MinIzo[k] = new int[nX2_real - 1];
        MaxIzo = new int*[nX1_real - 1];
        for (k = 0; k < nX1_real - 1; k++) MaxIzo[k] = new int[nX2_real - 1];
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



    for (i = 0; i < GrafRec.Height(); i++)
    {

        for (j = 0; j < GrafRec.Width(); j++)
        {
            if (layers && pProject->has_cel)
            {
                Values[1] = j / mX1 + fX1min;
                Values[2] = i / mX2 + fX2min;

                object[j + i*bih.biWidth] = LAYER_COL[pProject->GetLayer(Values[CEL_ind[1]], Values[CEL_ind[2]], Values[CEL_ind[3]])];
            } else object[j + i*bih.biWidth] = 0;
        }
    }





    for (int frame = 0; frame < Dlg.frames; frame++)
    {

        PrepareData_time(Dlg.m_MinMaxMode);
        for (i = 0; i < (nX1_real - 1); i++)
            for (j = 0; j < (nX2_real - 1); j++)
            {
                temp_fun = 0;
                for (ii = X1_real_ind[i]; ii < X1_real_ind[i + 1]; ii++)
                    for (jj = X2_real_ind[j]; jj < X2_real_ind[j + 1]; jj++)
                        temp_fun +=
                        Fun[jj    * nX1 + ii] *
                        (X1[jj    * nX1 + ii + 1] - X1[jj    * nX1 + ii])*
                        (X2[(jj + 1)* nX1 + ii] - X2[jj    * nX1 + ii]);
                Fun[X2_real_ind[j] * nX1 + X1_real_ind[i]] =
                    temp_fun / ((X1[X1_real_ind[i + 1]] - X1[X1_real_ind[i]])*
                    (X2[X2_real_ind[j + 1] * nX1] - X2[X2_real_ind[j] * nX1]));
            }

        if (lines)
        {
            //AviPaint(hdc);
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
            //	CPen myPen(PS_SOLID,1,ColorMin);			//change pen to ColorMin-pen
            //	CPen* oldPen=pDC->SelectObject(&myPen);    

            color = ColorMin;

            if (nGrids > 0)
            {
                n_hR = (GetRValue(ColorMax) - GetRValue(ColorMin)) / nGrids;
                n_hG = (GetGValue(ColorMax) - GetGValue(ColorMin)) / nGrids;
                n_hB = (GetBValue(ColorMax) - GetBValue(ColorMin)) / nGrids;
            }


            X = fGmin;


            for (i = 0; i < (nX2_real - 1); i++)
                for (j = 0; j < (nX1_real - 1); j++)
                {
                    fMin = 3.402823466e+38F;
                    fMax = -3.402823466e+38F;
                    f[0] = Fun[X2_real_ind[i] * nX1 + X1_real_ind[j]];	//			  P[2]
                    f[1] = Fun[X2_real_ind[i] * nX1 + X1_real_ind[j + 1]];	//		 !		    !
                    f[2] = Fun[X2_real_ind[(i + 1)] * nX1 + X1_real_ind[j]];	//		 !		    !
                    f[3] = Fun[X2_real_ind[(i + 1)] * nX1 + X1_real_ind[j + 1]];	//		f[0]---*---f[1]
                    for (k = 0; k < 4; k++)
                    {
                        if (f[k] < fMin) fMin = f[k];
                        if (f[k] > fMax) fMax = f[k];
                    }
                    MinIzo[j][i] = (int)((fMin - fGmin) / fGstep - 2);
                    MaxIzo[j][i] = (int)((fMax - fGmin) / fGstep + 2);

                }

            for (nX = 0; (int)nX < nGrids; nX++)
            {
                //Prepare pen
                k = nX - (int)((nX) / 5) * 5;
                //		pDC->SelectObject(oldPen);    
                //		myPen.DeleteObject();  
                // 		myPen.CreatePen(iStyle[k],1,color);	
                HPEN mpen3 = CreatePen(PS_SOLID, 1, color);
                SelectObject(hdc, mpen3);


                //		pDC->SelectObject(&myPen);    
                        //if(k==4)
                color = RGB(GetRValue(color) + n_hR,
                    GetGValue(color) + n_hG,
                    GetBValue(color) + n_hB); //change color
/*//Draw legend
        if((nX!=0)&&((nX/l)*l==nX))
        {
            k=GrafRec.bottom-(int)((nX+0.5)*GrafRec.Height()/nGrids);
            pDC->MoveTo(WinRec.right,k);
            pDC->LineTo(GrafRec.right,k);
            ttt.Format("%g",X);	pDC->TextOut(GrafRec.right+10,k+5,ttt);
        }*/

                for (i = 0; i < (nX2_real - 1); i++)
                    for (j = 0; j < (nX1_real - 1); j++)
                    {
                        //	if (!((nX>=MinIzo[j][i]) && (nX<=MaxIzo[j][i]))) continue;
            //Find izoline points
                        for (k = 0; k < 4; k++) B[k] = FALSE;

                        ii = i;
                        jj = j;
                        i = X2_real_ind[ii];
                        j = X1_real_ind[jj];
                        ia = X2_real_ind[ii + 1];
                        ja = X1_real_ind[jj + 1];


                        f[0] = Fun[i   * nX1 + j] - X;//			  P[2]
                        x1[0] = X1[i   * nX1 + j];		//		f[2]---*---f[3]
                        x2[0] = X2[i   * nX1 + j];		//		 !		    !
                        f[1] = Fun[i   * nX1 + ja] - X;//		 !		    !
                        x1[1] = X1[i   * nX1 + ja];		//		 !		    !
                        x2[1] = X2[i   * nX1 + ja];		//  P[1] *		    * P[3]
                        f[2] = Fun[(ia)* nX1 + j] - X;//		 !		    !
                        x1[2] = X1[(ia)* nX1 + j];		//		 !		    !
                        x2[2] = X2[(ia)* nX1 + j];		//		 !		    !					
                        f[3] = Fun[(ia)* nX1 + ja] - X;//		f[0]---*---f[1]
                        x1[3] = X1[(ia)* nX1 + ja];		//			  P[0]	
                        x2[3] = X2[(ia)* nX1 + ja];

                        i = ii;
                        j = jj;


                        if ((f[0] * f[1] <= 0) && (f[1] != f[0]))
                            //find P[0] between x1,x2[0] and x1,x2[1]
                        {
                            hX = (float)(fabs(f[0] / (f[1] - f[0])));
                            P[0].x = GrafRec.left + (int)(mX1*(x1[0] + hX*(x1[1] - x1[0]) - fX1min));
                            P[0].y = GrafRec.bottom - (int)(mX2*(x2[0] + hX*(x2[1] - x2[0]) - fX2min));
                            B[0] = TRUE;
                        }
                        if ((f[0] * f[2] <= 0) && (f[0] != f[2]))
                            //find P[1] between x1,x2[0] and x1,x2[2]
                        {
                            hX = (float)(fabs(f[0] / (f[2] - f[0])));
                            P[1].x = GrafRec.left + (int)(mX1*(x1[0] + hX*(x1[2] - x1[0]) - fX1min));
                            P[1].y = GrafRec.bottom - (int)(mX2*(x2[0] + hX*(x2[2] - x2[0]) - fX2min));
                            B[1] = TRUE;
                        }
                        if ((f[2] * f[3] <= 0) && (f[3] != f[2]))
                            //find P[2] between x1,x2[2] and x1,x2[3]
                        {
                            hX = (float)(fabs(f[2] / (f[3] - f[2])));
                            P[2].x = GrafRec.left + (int)(mX1*(x1[2] + hX*(x1[3] - x1[2]) - fX1min));
                            P[2].y = GrafRec.bottom - (int)(mX2*(x2[2] + hX*(x2[3] - x2[2]) - fX2min));
                            B[2] = TRUE;
                        }
                        if ((f[1] * f[3] <= 0) && (f[1] != f[3]))
                            //find P[3] between x1,x2[1] and x1,x2[3]
                        {
                            hX = (float)(fabs(f[1] / (f[3] - f[1])));
                            P[3].x = GrafRec.left + (int)(mX1*(x1[1] + hX*(x1[3] - x1[1]) - fX1min));
                            P[3].y = GrafRec.bottom - (int)(mX2*(x2[1] + hX*(x2[3] - x2[1]) - fX2min));
                            B[3] = TRUE;
                        }
                        //Draw izolines:
                        /* P[0] - P[2] */   if (B[0] && B[2] && !B[1]) { MoveToEx(hdc, P[0].x, P[0].y, NULL); LineTo(hdc, P[2].x, P[2].y); continue; }
                        /* P[1] - P[3] */  	if (B[1] && B[3] && !B[0]) { MoveToEx(hdc, P[1].x, P[1].y, NULL); LineTo(hdc, P[3].x, P[3].y); continue; }
                        /* P[0] - P[1] */  	if (B[0] && B[1]) { MoveToEx(hdc, P[0].x, P[0].y, NULL); LineTo(hdc, P[1].x, P[1].y); }
                        /* P[1] - P[2] */  	if (B[1] && B[2]) { MoveToEx(hdc, P[1].x, P[1].y, NULL); LineTo(hdc, P[2].x, P[2].y); }
                        /* P[2] - P[3] */  	if (B[2] && B[3]) { MoveToEx(hdc, P[2].x, P[2].y, NULL); LineTo(hdc, P[3].x, P[3].y); }
                        /* P[0] - P[3] */  	if (B[0] && B[3]) { MoveToEx(hdc, P[0].x, P[0].y, NULL); LineTo(hdc, P[3].x, P[3].y); }
                    }
                X += fGstep; if (fabs(X) < 0.01*fGstep) X = (float)0;
                DeleteObject(mpen3);
            }
            //	pDC->SelectObject(oldPen);    
            //	myPen.DeleteObject();  
            DeleteObject(mpen1);
            DeleteObject(mpen2);

        }



        // Try dbits


        if (cloud)
        {
            jj = 1;
            for (i = 0; i < bih.biHeight; i++)
            {
                ii = 1;
                for (j = 0; j < bih.biWidth; j++)
                {

                    X1_ = j / mX1 + fX1min;
                    X2_ = i / mX2 + fX2min;
                    for (; ii < nX1_real - 1; ii++) if (X1_ < X1[X1_real_ind[ii]]) break;
                    for (; jj < nX2_real - 1; jj++) if (X2_ < X2[X2_real_ind[jj] * nX1]) break;
                    X = ((-X2[X2_real_ind[jj - 1] * nX1] + X2_)*(X1_ - X1[X1_real_ind[ii - 1]])*Fun[X2_real_ind[jj] * nX1 + X1_real_ind[ii]] +
                        (X2[X2_real_ind[jj] * nX1] - X2_)*(X1_ - X1[X1_real_ind[ii - 1]])*Fun[X2_real_ind[jj - 1] * nX1 + X1_real_ind[ii]] +
                        (X2[X2_real_ind[jj] * nX1] - X2_)*(-X1_ + X1[X1_real_ind[ii]])*Fun[X2_real_ind[jj - 1] * nX1 + X1_real_ind[ii - 1]] +
                        (-X2[X2_real_ind[jj - 1] * nX1] + X2_)*(-X1_ + X1[X1_real_ind[ii]])*Fun[X2_real_ind[jj] * nX1 + X1_real_ind[ii - 1]]) /
                        ((X2[X2_real_ind[jj] * nX1] - X2[X2_real_ind[jj - 1] * nX1])*(X1[X1_real_ind[ii]] - X1[X1_real_ind[ii - 1]]));
                    hX = (X - fFmin) / (fFmax - fFmin);
                    if (hX > 1) hX = 1;
                    if (hX < 0) hX = 0;
                    dbits[j + i*bih.biWidth] = MakeColor(hX, 0) | object[j + i*bih.biWidth];
                }

            }

        }


        AddAviFrame(avi, hbm);

    }
    CloseAvi(avi);
    //
    if (lines)
    {
        for (k = 0; k < nX1 - 1; k++) delete[] MinIzo[k];
        delete[] MinIzo;

        for (k = 0; k < nX1 - 1; k++) delete[] MaxIzo[k];
        delete[] MaxIzo;

        delete[] X1_real_ind;
        delete[] X2_real_ind;
    }
    SelectObject(hdc, holdb); SelectObject(hdc, holdp);
    DeleteDC(hdc); DeleteObject(hbm); DeleteObject(hp);




}

void CChild_2_Frame::AviPaint(HDC hdc)
{

    const int wMin = 80, hMin = 40;






}

/////////////////////////////////////////////////////////////////////////////
// Draw the graph
/////////////////////////////////////////////////////////////////////////////
void CChild_2_Frame::OnPaint()
{
    CPaintDC dc(this);							// device context for painting
    Paint(&dc);
}


void CChild_2_Frame::Paint(CDC *pDC)
{
    const int wMin = 80, hMin = 40;
    CRect       WinRec;	GetClientRect(WinRec);	// window size	

//Calculate graph rectangle
    CRect 		GrafRec;
    GrafRec.left = 90; GrafRec.right = WinRec.right - 100;
    if (GrafRec.Width() < wMin) return;
    GrafRec.top = 30; GrafRec.bottom = WinRec.bottom - 30;
    if (GrafRec.Height() < hMin) return;

    //Scale calculation
    float mX1 = GrafRec.Width() / (fX1max - fX1min);
    float mX2 = GrafRec.Height() / (fX2max - fX2min);

    //Draw axis lines
    pDC->MoveTo(GrafRec.left, GrafRec.top);
    pDC->LineTo(GrafRec.left, GrafRec.bottom);
    pDC->LineTo(GrafRec.right, GrafRec.bottom);
    CString ttt;
    CSize sSize;
    int j;

    //Draw X2-axe 
    pDC->TextOut(5, 0, sX2Name);								//X2-axe name
    int  nX = GrafRec.Height() / hMin;
    float hX = (fX2max - fX2min) / nX;
    float dd = 10 * Roundation(hX);
    hX = (Divide(hX, dd) + 1) * dd;
    float X = Divide(fX2min, hX) * hX;
    hX /= 2;
    for (; X <= fX2max; )
    {
        if (X >= fX2min)
        {
            ttt.Format("%g", X);	sSize = pDC->GetTextExtent(ttt);
            j = GrafRec.bottom - (int)((X - fX2min)*mX2);
            pDC->TextOut(GrafRec.left - 15 - sSize.cx, j - 8, ttt);	//X2-labels
            pDC->MoveTo(GrafRec.left - 10, j);
            pDC->LineTo(GrafRec.left, j);					//X2-main marks
            if (bGridL) pDC->LineTo(GrafRec.right, j);			//X2-grid lines
        }
        X += hX;
        if ((X >= fX2min) && (X <= fX2max))
        {
            j = GrafRec.bottom - (int)((X - fX2min)*mX2);
            pDC->MoveTo(GrafRec.left - 5, j);
            pDC->LineTo(GrafRec.left, j);					//X2-short marks
        }
        X += hX;  if (fabs(X) < 0.01*hX) X = (float)0;
    }

    //Draw X1-axe 
    sSize = pDC->GetTextExtent(sX1Name);						//X1-axe name
    pDC->TextOut(WinRec.right - sSize.cx - 20, GrafRec.bottom, sX1Name);
    nX = GrafRec.Width() / wMin;
    hX = (fX1max - fX1min) / nX;
    dd = 10 * Roundation(hX);
    hX = (Divide(hX, dd) + 1) * dd;
    X = Divide(fX1min, hX) * hX;
    hX /= 2;
    for (; X <= fX1max; )
    {
        if (X >= fX1min)
        {
            ttt.Format("%g", X);		sSize = pDC->GetTextExtent(ttt);
            j = GrafRec.left + (int)((X - fX1min)*mX1);
            pDC->TextOut(j - sSize.cx / 2, GrafRec.bottom + 10, ttt);	//X1-labels
            pDC->MoveTo(j, GrafRec.bottom + 10);
            pDC->LineTo(j, GrafRec.bottom);					//X1-main marks
            if (bGridL) pDC->LineTo(j, GrafRec.top);			//X1-grid lines				
        }
        X += hX;
        if ((X >= fX1min) && (X <= fX1max))
        {
            j = GrafRec.left + (int)((X - fX1min)*mX1);
            pDC->MoveTo(j, GrafRec.bottom + 5);
            pDC->LineTo(j, GrafRec.bottom);					//X1-short marks
        }
        X += hX; if (fabs(X) < 0.01*hX) X = (float)0;
    }


    ///////////////////////////////////////////////////////////////////////
    //Draw TWO-Dim graph
    CPen myPen(PS_SOLID, 1, ColorMin);			//change pen to ColorMin-pen
    CPen* oldPen = pDC->SelectObject(&myPen);

    int i;
    float   f[4], x1[4], x2[4];
    CPoint  P[4];
    BOOL	B[4];
    int	k, l;
    COLORREF color = ColorMin;
    int	n_hR, n_hG, n_hB;
    if (nGrids > 0)
    {
        n_hR = 5 * (GetRValue(ColorMax) - GetRValue(ColorMin)) / nGrids;
        n_hG = 5 * (GetGValue(ColorMax) - GetGValue(ColorMin)) / nGrids;
        n_hB = 5 * (GetBValue(ColorMax) - GetBValue(ColorMin)) / nGrids;
        if (GrafRec.Height() / nGrids >= hMin / 2) l = 1;	  //legend for every grid
        else
            l = (nGrids / (10 * GrafRec.Height() / hMin) + 1) * 5;//legend for several grids
    }

    X = fGmin;


    int ii, jj, ia, ja;
    double X1_, X2_;
    int nX1_real, nX2_real;
    int *X1_real_ind, *X2_real_ind;
    X1_real_ind = new int[nX1];
    X2_real_ind = new int[nX2];
    X1_real_ind[0] = 0;
    X2_real_ind[0] = 0;



    for (ii = 0, jj = 0; ii < nX1; ii++)
        if (mX1*(X1[ii] - X1[X1_real_ind[jj]]) > 0.5)
        {
            jj++;
            X1_real_ind[jj] = ii;
        }
    nX1_real = jj + 1;
    for (ii = 0, jj = 0; ii < nX2; ii++)
        if (mX2*(X2[ii    * nX1] - X2[X2_real_ind[jj] * nX1]) > 0.5)
        {
            jj++;
            X2_real_ind[jj] = ii;
        }
    nX2_real = jj + 1;


    int** MinIzo;
    MinIzo = new int*[nX1 - 1];
    for (k = 0; k < nX1 - 1; k++) MinIzo[k] = new int[nX2 - 1];
    int** MaxIzo;
    MaxIzo = new int*[nX1 - 1];
    for (k = 0; k < nX1 - 1; k++) MaxIzo[k] = new int[nX2 - 1];

    float fMin;
    float fMax;

    float temp_fun;

    for (i = 0; i < (nX1_real - 1); i++)
        for (j = 0; j < (nX2_real - 1); j++)
        {
            temp_fun = 0;
            for (ii = X1_real_ind[i]; ii < X1_real_ind[i + 1]; ii++)
                for (jj = X2_real_ind[j]; jj < X2_real_ind[j + 1]; jj++)
                    temp_fun +=
                    Fun[jj    * nX1 + ii] *
                    (X1[jj    * nX1 + ii + 1] - X1[jj    * nX1 + ii])*
                    (X2[(jj + 1)* nX1 + ii] - X2[jj    * nX1 + ii]);
            Fun[X2_real_ind[j] * nX1 + X1_real_ind[i]] =
                temp_fun / ((X1[X1_real_ind[i + 1]] - X1[X1_real_ind[i]])*
                (X2[X2_real_ind[j + 1] * nX1] - X2[X2_real_ind[j] * nX1]));
        }

    for (i = 0; i < (nX2_real - 1); i++)
        for (j = 0; j < (nX1_real - 1); j++)
        {
            fMin = 3.402823466e+38F;
            fMax = -3.402823466e+38F;

            f[0] = Fun[X2_real_ind[i] * nX1 + X1_real_ind[j]];	//			  P[2]
            f[1] = Fun[X2_real_ind[i] * nX1 + X1_real_ind[j + 1]];	//		 !		    !
            f[2] = Fun[X2_real_ind[(i + 1)] * nX1 + X1_real_ind[j]];	//		 !		    !
            f[3] = Fun[X2_real_ind[(i + 1)] * nX1 + X1_real_ind[j + 1]];	//		f[0]---*---f[1]
            for (k = 0; k < 4; k++)
            {
                if (f[k] < fMin) fMin = f[k];
                if (f[k] > fMax) fMax = f[k];
            }
            MinIzo[j][i] = (int)((fMin - fGmin) / fGstep - 2);
            MaxIzo[j][i] = (int)((fMax - fGmin) / fGstep + 2);

        }
    ///////////////////////////////
    // Make a DI Bitmap for memory painting
    CDC *cdcscreen = GetDC();
    HDC hdcscreen = cdcscreen->m_hDC, hdc = CreateCompatibleDC(hdcscreen); ReleaseDC(cdcscreen);
    BITMAPINFO bi; ZeroMemory(&bi, sizeof(bi)); BITMAPINFOHEADER &bih = bi.bmiHeader;
    bih.biSize = sizeof(bih);
    bih.biWidth = WinRec.Width();
    bih.biHeight = WinRec.Height();
    bih.biPlanes = 1;
    bih.biBitCount = 32;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = ((bih.biWidth*bih.biBitCount / 8 + 3) & 0xFFFFFFFC)*bih.biHeight;
    bih.biXPelsPerMeter = 10000;
    bih.biYPelsPerMeter = 10000;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;
    void *bits; HBITMAP hbm = CreateDIBSection(hdc, (BITMAPINFO*)&bih, DIB_RGB_COLORS, &bits, NULL, NULL);

    DWORD *dbits = (DWORD*)bits;



    CDC memDC;
    CBitmap MemBit;
    //	CBitmap* oldB;
        //MemBit.CreateBitmap(, 2,24,NULL);
    memDC.CreateCompatibleDC(pDC);

    //MemBit.CreateCompatibleBitmap(pDC, GrafRec.Width(), GrafRec.Height());
    //oldB = memDC.SelectObject(&MemBit);
    memDC.SelectObject(hbm);

    if (layers && pProject->has_cel)
    {

        for (i = 0; i < GrafRec.Height(); i++)
        {

            for (j = 0; j < GrafRec.Width(); j++)
            {

                Values[1] = j / mX1 + fX1min;
                Values[2] = i / mX2 + fX2min;
                //	memDC.SetPixelV(j,GrafRec.Height()-i,//GrafRec.left+j,GrafRec.bottom-i,
                //		LAYER_COL[pProject->GetLayer(Values[CEL_ind[1]],Values[CEL_ind[2]], Values[CEL_ind[3]]) ]);

                dbits[j + i*bih.biWidth] = LAYER_COL[pProject->GetLayer(Values[CEL_ind[1]], Values[CEL_ind[2]], Values[CEL_ind[3]])];


            }
        }
        //		pDC->BitBlt(GrafRec.left+1, GrafRec.top, GrafRec.Width(), GrafRec.Height(), 
        //			&memDC, 1, WinRec.Height()-GrafRec.Height(), SRCCOPY);
        //	SetDIBits(pDC->m_hDC, hbm,  0, 10000, dbits, &bi, DIB_RGB_COLORS);

    }//end if (layers)




//////////////////
    if (cloud)
    {
        jj = 1;
        for (i = 0; i < GrafRec.Height(); i++)
        {
            ii = 1;
            for (j = 0; j < GrafRec.Width(); j++)
            {

                X1_ = j / mX1 + fX1min;
                X2_ = i / mX2 + fX2min;
                for (; ii < nX1_real - 1; ii++) if (X1_ < X1[X1_real_ind[ii]]) break;
                for (; jj < nX2_real - 1; jj++) if (X2_ < X2[X2_real_ind[jj] * nX1]) break;
                X = ((-X2[X2_real_ind[jj - 1] * nX1] + X2_)*(X1_ - X1[X1_real_ind[ii - 1]])*Fun[X2_real_ind[jj] * nX1 + X1_real_ind[ii]] +
                    (X2[X2_real_ind[jj] * nX1] - X2_)*(X1_ - X1[X1_real_ind[ii - 1]])*Fun[X2_real_ind[jj - 1] * nX1 + X1_real_ind[ii]] +
                    (X2[X2_real_ind[jj] * nX1] - X2_)*(-X1_ + X1[X1_real_ind[ii]])*Fun[X2_real_ind[jj - 1] * nX1 + X1_real_ind[ii - 1]] +
                    (-X2[X2_real_ind[jj - 1] * nX1] + X2_)*(-X1_ + X1[X1_real_ind[ii]])*Fun[X2_real_ind[jj] * nX1 + X1_real_ind[ii - 1]]) /
                    ((X2[X2_real_ind[jj] * nX1] - X2[X2_real_ind[jj - 1] * nX1])*(X1[X1_real_ind[ii]] - X1[X1_real_ind[ii - 1]]));
                hX = (X - fFmin) / (fFmax - fFmin);
                if (hX > 1) hX = 1;
                if (hX < 0) hX = 0;

                dbits[j + i*bih.biWidth] = dbits[j + i*bih.biWidth] | MakeColor(hX, 0);//+((int)(256*(1-hX)));

            }
        }

        //	pDC->BitBlt(GrafRec.left+1, GrafRec.top, GrafRec.Width(), GrafRec.Height(), 
        //			&memDC, 1, WinRec.bottom-GrafRec.Height(), SRCCOPY);
    }//end if (cloud)
    pDC->BitBlt(GrafRec.left + 1, GrafRec.top, GrafRec.Width(), GrafRec.Height(),
        &memDC, 1, WinRec.Height() - GrafRec.Height(), SRCCOPY);

    //////////////////////////////////////		

    if (lines)
    {

        for (nX = 0; (int)nX < nGrids; nX++)
        {
            //Prepare pen
            k = nX - (int)((nX) / 5) * 5;
            pDC->SelectObject(oldPen);
            myPen.DeleteObject();
            myPen.CreatePen(iStyle[k], 1, color);
            pDC->SelectObject(&myPen);
            if (k == 4)	color = RGB(GetRValue(color) + n_hR,
                GetGValue(color) + n_hG,
                GetBValue(color) + n_hB); //change color
//Draw legend
            if ((nX != 0) && ((nX / l)*l == nX))
            {
                k = GrafRec.bottom - (int)((nX + 0.5)*GrafRec.Height() / nGrids);
                pDC->MoveTo(WinRec.right, k);
                pDC->LineTo(GrafRec.right, k);
                ttt.Format("%g", X);	pDC->TextOut(GrafRec.right + 10, k + 5, ttt);
            }

            for (i = 0; i < (nX2_real - 1); i++)
                for (j = 0; j < (nX1_real - 1); j++)
                {
                    //Find izoline points
                    if (!((nX >= MinIzo[j][i]) && (nX <= MaxIzo[j][i])))
                    {

                        continue;
                    }
                    for (k = 0; k < 4; k++) B[k] = FALSE;
                    ii = i;
                    jj = j;
                    i = X2_real_ind[ii];
                    j = X1_real_ind[jj];
                    ia = X2_real_ind[ii + 1];
                    ja = X1_real_ind[jj + 1];


                    f[0] = Fun[i   * nX1 + j] - X;//			  P[2]
                    x1[0] = X1[i   * nX1 + j];		//		f[2]---*---f[3]
                    x2[0] = X2[i   * nX1 + j];		//		 !		    !
                    f[1] = Fun[i   * nX1 + ja] - X;//		 !		    !
                    x1[1] = X1[i   * nX1 + ja];		//		 !		    !
                    x2[1] = X2[i   * nX1 + ja];		//  P[1] *		    * P[3]
                    f[2] = Fun[(ia)* nX1 + j] - X;//		 !		    !
                    x1[2] = X1[(ia)* nX1 + j];		//		 !		    !
                    x2[2] = X2[(ia)* nX1 + j];		//		 !		    !					
                    f[3] = Fun[(ia)* nX1 + ja] - X;//		f[0]---*---f[1]
                    x1[3] = X1[(ia)* nX1 + ja];		//			  P[0]	
                    x2[3] = X2[(ia)* nX1 + ja];

                    i = ii;
                    j = jj;


                    if ((f[0] * f[1] <= 0) && (f[1] != f[0]))
                        //find P[0] between x1,x2[0] and x1,x2[1]
                    {
                        hX = (float)(fabs(f[0] / (f[1] - f[0])));
                        P[0].x = GrafRec.left + (int)(mX1*(x1[0] + hX*(x1[1] - x1[0]) - fX1min));
                        P[0].y = GrafRec.bottom - (int)(mX2*(x2[0] + hX*(x2[1] - x2[0]) - fX2min));
                        B[0] = TRUE;
                    }
                    if ((f[0] * f[2] <= 0) && (f[0] != f[2]))
                        //find P[1] between x1,x2[0] and x1,x2[2]
                    {
                        hX = (float)(fabs(f[0] / (f[2] - f[0])));
                        P[1].x = GrafRec.left + (int)(mX1*(x1[0] + hX*(x1[2] - x1[0]) - fX1min));
                        P[1].y = GrafRec.bottom - (int)(mX2*(x2[0] + hX*(x2[2] - x2[0]) - fX2min));
                        B[1] = TRUE;
                    }
                    if ((f[2] * f[3] <= 0) && (f[3] != f[2]))
                        //find P[2] between x1,x2[2] and x1,x2[3]
                    {
                        hX = (float)(fabs(f[2] / (f[3] - f[2])));
                        P[2].x = GrafRec.left + (int)(mX1*(x1[2] + hX*(x1[3] - x1[2]) - fX1min));
                        P[2].y = GrafRec.bottom - (int)(mX2*(x2[2] + hX*(x2[3] - x2[2]) - fX2min));
                        B[2] = TRUE;
                    }
                    if ((f[1] * f[3] <= 0) && (f[1] != f[3]))
                        //find P[3] between x1,x2[1] and x1,x2[3]
                    {
                        hX = (float)(fabs(f[1] / (f[3] - f[1])));
                        P[3].x = GrafRec.left + (int)(mX1*(x1[1] + hX*(x1[3] - x1[1]) - fX1min));
                        P[3].y = GrafRec.bottom - (int)(mX2*(x2[1] + hX*(x2[3] - x2[1]) - fX2min));
                        B[3] = TRUE;
                    }
                    //Draw izolines:
                    /* P[0] - P[2] */   if (B[0] && B[2] && !B[1]) { pDC->MoveTo(P[0]); pDC->LineTo(P[2]); continue; }
                    /* P[1] - P[3] */  	if (B[1] && B[3] && !B[0]) { pDC->MoveTo(P[1]); pDC->LineTo(P[3]); continue; }
                    /* P[0] - P[1] */  	if (B[0] && B[1]) { pDC->MoveTo(P[0]); pDC->LineTo(P[1]); }
                    /* P[1] - P[2] */  	if (B[1] && B[2]) { pDC->MoveTo(P[1]); pDC->LineTo(P[2]); }
                    /* P[2] - P[3] */  	if (B[2] && B[3]) { pDC->MoveTo(P[2]); pDC->LineTo(P[3]); }
                    /* P[0] - P[3] */  	if (B[0] && B[3]) { pDC->MoveTo(P[0]); pDC->LineTo(P[3]); }
                }
            X += fGstep; if (fabs(X) < 0.01*fGstep) X = (float)0;
        }
        pDC->SelectObject(oldPen);
        myPen.DeleteObject();



        for (k = 0; k < nX1 - 1; k++) delete[] MinIzo[k];
        delete[] MinIzo;

        for (k = 0; k < nX1 - 1; k++) delete[] MaxIzo[k];
        delete[] MaxIzo;
    } // end lines
}

/////////////////////////////////////////////////////////////////////////////
// Draw nearest graph point on Left mouse button down
void CChild_2_Frame::OnLButtonDown(UINT nFlags, CPoint point)
{
    Invalidate();		// redraw
    const int wMin = 80, hMin = 40;
    CPaintDC dc(this);	// device context for painting -not used

//Clear status bar
    CMainFrame *pWnd = (CMainFrame *)AfxGetMainWnd();
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
            for (j = 0; j < nX2; j++) if (X2[j*nX1] > x2) break; j--;
            if (j < 0)   j = 0;
            else if (((j + 1) < nX2) && ((X2[(j + 1)*nX1] - x2) < (x2 - X2[j*nX1]))) j++;
            //Global index
            i = j*nX1 + i;
        } else					 // for polar coordinates
        {
            //Find nearest X1[i],X2[i] point
            i = 0;
            double delta = sqrt((X1[i] - x1)*(X1[i] - x1) + (X2[i] - x2)*(X2[i] - x2));
            for (j = 1; j < nX1*nX2; j++)
            {
                double delta_n = sqrt((X1[j] - x1)*(X1[j] - x1) + (X2[j] - x2)*(X2[j] - x2));
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
        j = GrafRec.bottom - (int)(mX2*(X2[i] - fX2min));
        i = GrafRec.left + (int)(mX1*(X1[i] - fX1min));
        //Draw nearest point
        dc.Ellipse(i - 5, j - 5, i + 5, j + 5);
        dc.MoveTo(GrafRec.left, j);
        dc.LineTo(i, j);
        dc.LineTo(i, GrafRec.bottom);
    }
    Invalidate(FALSE);	//Don't redraw
}





// First, we'll define the WAV file format.
#include <pshpack1.h>
typedef struct
{
    char id[4];         //="fmt "
    unsigned long size; //=16
    short wFormatTag;   //=WAVE_FORMAT_PCM=1
    unsigned short wChannels;       //=1 or 2 for mono or stereo
    unsigned long dwSamplesPerSec;  //=11025 or 22050 or 44100
    unsigned long dwAvgBytesPerSec; //=wBlockAlign * dwSamplesPerSec
    unsigned short wBlockAlign;     //=wChannels * (wBitsPerSample==8?1:2)
    unsigned short wBitsPerSample;  //=8 or 16, for bits per sample
} FmtChunk;

typedef struct
{
    char id[4];            //="data"
    unsigned long size;    //=datsize, size of the following array
    unsigned char data[1]; //=the raw data goes here
} DataChunk;

typedef struct
{
    char id[4];         //="RIFF"
    unsigned long size; //=datsize+8+16+4
    char type[4];       //="WAVE"
    FmtChunk fmt;
    DataChunk dat;
} WavChunk;
#include <poppack.h>




// This is the internal structure represented by the HAVI handle:
typedef struct
{
    IAVIFile *pfile;    // created by CreateAvi
    WAVEFORMATEX wfx;   // as given to CreateAvi (.nChanels=0 if none was given). Used when audio stream is first created.
    int period;         // specified in CreateAvi, used when the video stream is first created
    IAVIStream *as;     // audio stream, initialised when audio stream is first created
    IAVIStream *ps, *psCompressed;  // video stream, when first created
    unsigned long nframe, nsamp;    // which frame will be added next, which sample will be added next
    bool iserr;         // if true, then no function will do anything
} TAviUtil;


HAVI CreateAvi(const char *fn, int frameperiod, const WAVEFORMATEX *wfx)
{
    IAVIFile *pfile;
    AVIFileInit();
    HRESULT hr = AVIFileOpen(&pfile, fn, OF_WRITE | OF_CREATE, NULL);
    if (hr != AVIERR_OK) { AVIFileExit(); return NULL; }
    TAviUtil *au = new TAviUtil;
    au->pfile = pfile;
    if (wfx == NULL) ZeroMemory(&au->wfx, sizeof(WAVEFORMATEX)); else CopyMemory(&au->wfx, wfx, sizeof(WAVEFORMATEX));
    au->period = frameperiod;
    au->as = 0; au->ps = 0; au->psCompressed = 0;
    au->nframe = 0; au->nsamp = 0;
    au->iserr = false;
    return (HAVI)au;
}

HRESULT CloseAvi(HAVI avi)
{
    if (avi == NULL) return AVIERR_BADHANDLE;
    TAviUtil *au = (TAviUtil*)avi;
    if (au->as != 0) AVIStreamRelease(au->as); au->as = 0;
    if (au->psCompressed != 0) AVIStreamRelease(au->psCompressed); au->psCompressed = 0;
    if (au->ps != 0) AVIStreamRelease(au->ps); au->ps = 0;
    if (au->pfile != 0) AVIFileRelease(au->pfile); au->pfile = 0;
    AVIFileExit();
    delete au;
    return S_OK;
}


HRESULT SetAviVideoCompression(HAVI avi, HBITMAP hbm, AVICOMPRESSOPTIONS *opts, bool ShowDialog, HWND hparent)
{
    if (avi == NULL) return AVIERR_BADHANDLE;
    if (hbm == NULL) return AVIERR_BADPARAM;
    DIBSECTION dibs; int sbm = GetObject(hbm, sizeof(dibs), &dibs);
    if (sbm != sizeof(DIBSECTION)) return AVIERR_BADPARAM;
    TAviUtil *au = (TAviUtil*)avi;
    if (au->iserr) return AVIERR_ERROR;
    if (au->psCompressed != 0) return AVIERR_COMPRESSOR;
    //
    if (au->ps == 0) // create the stream, if it wasn't there before
    {
        AVISTREAMINFO strhdr; ZeroMemory(&strhdr, sizeof(strhdr));
        strhdr.fccType = streamtypeVIDEO;// stream type
        strhdr.fccHandler = 0;
        strhdr.dwScale = au->period;
        strhdr.dwRate = 1000;
        strhdr.dwSuggestedBufferSize = dibs.dsBmih.biSizeImage;
        SetRect(&strhdr.rcFrame, 0, 0, dibs.dsBmih.biWidth, dibs.dsBmih.biHeight);
        HRESULT hr = AVIFileCreateStream(au->pfile, &au->ps, &strhdr);
        if (hr != AVIERR_OK) { au->iserr = true; return hr; }
    }
    //
    if (au->psCompressed == 0) // set the compression, prompting dialog if necessary
    {
        AVICOMPRESSOPTIONS myopts; ZeroMemory(&myopts, sizeof(myopts));
        AVICOMPRESSOPTIONS *aopts[1];
        if (opts != NULL) aopts[0] = opts; else aopts[0] = &myopts;
        if (ShowDialog)
        {
            BOOL res = (BOOL)AVISaveOptions(hparent, 0, 1, &au->ps, aopts);
            if (!res) { AVISaveOptionsFree(1, aopts); au->iserr = true; return AVIERR_USERABORT; }
        }
        HRESULT hr = AVIMakeCompressedStream(&au->psCompressed, au->ps, aopts[0], NULL);
        AVISaveOptionsFree(1, aopts);
        if (hr != AVIERR_OK) { au->iserr = true; return hr; }
        DIBSECTION dibs; GetObject(hbm, sizeof(dibs), &dibs);
        hr = AVIStreamSetFormat(au->psCompressed, 0, &dibs.dsBmih, dibs.dsBmih.biSize + dibs.dsBmih.biClrUsed * sizeof(RGBQUAD));
        if (hr != AVIERR_OK) { au->iserr = true; return hr; }
    }
    //
    return AVIERR_OK;
}


HRESULT AddAviFrame(HAVI avi, HBITMAP hbm)
{
    if (avi == NULL) return AVIERR_BADHANDLE;
    if (hbm == NULL) return AVIERR_BADPARAM;
    DIBSECTION dibs; int sbm = GetObject(hbm, sizeof(dibs), &dibs);
    if (sbm != sizeof(DIBSECTION)) return AVIERR_BADPARAM;
    TAviUtil *au = (TAviUtil*)avi;
    if (au->iserr) return AVIERR_ERROR;
    //
    if (au->ps == 0) // create the stream, if it wasn't there before
    {
        AVISTREAMINFO strhdr; ZeroMemory(&strhdr, sizeof(strhdr));
        strhdr.fccType = streamtypeVIDEO;// stream type
        strhdr.fccHandler = 0;
        strhdr.dwScale = au->period;
        strhdr.dwRate = 1000;
        strhdr.dwSuggestedBufferSize = dibs.dsBmih.biSizeImage;
        SetRect(&strhdr.rcFrame, 0, 0, dibs.dsBmih.biWidth, dibs.dsBmih.biHeight);
        HRESULT hr = AVIFileCreateStream(au->pfile, &au->ps, &strhdr);
        if (hr != AVIERR_OK) { au->iserr = true; return hr; }
    }
    //
    // create an empty compression, if the user hasn't set any
    if (au->psCompressed == 0)
    {
        AVICOMPRESSOPTIONS opts; ZeroMemory(&opts, sizeof(opts));
        opts.fccHandler = mmioFOURCC('D', 'I', 'B', ' ');
        HRESULT hr = AVIMakeCompressedStream(&au->psCompressed, au->ps, &opts, NULL);
        if (hr != AVIERR_OK) { au->iserr = true; return hr; }
        hr = AVIStreamSetFormat(au->psCompressed, 0, &dibs.dsBmih, dibs.dsBmih.biSize + dibs.dsBmih.biClrUsed * sizeof(RGBQUAD));
        if (hr != AVIERR_OK) { au->iserr = true; return hr; }
    }
    //
    //Now we can add the frame
    HRESULT hr = AVIStreamWrite(au->psCompressed, au->nframe, 1, dibs.dsBm.bmBits, dibs.dsBmih.biSizeImage, AVIIF_KEYFRAME, NULL, NULL);
    if (hr != AVIERR_OK) { au->iserr = true; return hr; }
    au->nframe++; return S_OK;
}



HRESULT AddAviAudio(HAVI avi, void *dat, unsigned long numbytes)
{
    if (avi == NULL) return AVIERR_BADHANDLE;
    if (dat == NULL || numbytes == 0) return AVIERR_BADPARAM;
    TAviUtil *au = (TAviUtil*)avi;
    if (au->iserr) return AVIERR_ERROR;
    if (au->wfx.nChannels == 0) return AVIERR_BADFORMAT;
    unsigned long numsamps = numbytes * 8 / au->wfx.wBitsPerSample;
    if ((numsamps*au->wfx.wBitsPerSample / 8) != numbytes) return AVIERR_BADPARAM;
    //
    if (au->as == 0) // create the stream if necessary
    {
        AVISTREAMINFO ahdr; ZeroMemory(&ahdr, sizeof(ahdr));
        ahdr.fccType = streamtypeAUDIO;
        ahdr.dwScale = au->wfx.nBlockAlign;
        ahdr.dwRate = au->wfx.nSamplesPerSec*au->wfx.nBlockAlign;
        ahdr.dwSampleSize = au->wfx.nBlockAlign;
        ahdr.dwQuality = (DWORD)-1;
        HRESULT hr = AVIFileCreateStream(au->pfile, &au->as, &ahdr);
        if (hr != AVIERR_OK) { au->iserr = true; return hr; }
        hr = AVIStreamSetFormat(au->as, 0, &au->wfx, sizeof(WAVEFORMATEX));
        if (hr != AVIERR_OK) { au->iserr = true; return hr; }
    }
    //
    // now we can write the data
    HRESULT hr = AVIStreamWrite(au->as, au->nsamp, numsamps, dat, numbytes, 0, NULL, NULL);
    if (hr != AVIERR_OK) { au->iserr = true; return hr; }
    au->nsamp += numsamps; return S_OK;
}



HRESULT AddAviWav(HAVI avi, const char *src, DWORD flags)
{
    if (avi == NULL) return AVIERR_BADHANDLE;
    if (flags != SND_MEMORY && flags != SND_FILENAME) return AVIERR_BADFLAGS;
    if (src == 0) return AVIERR_BADPARAM;
    TAviUtil *au = (TAviUtil*)avi;
    if (au->iserr) return AVIERR_ERROR;
    //
    char *buf = 0; WavChunk *wav = (WavChunk*)src;
    if (flags == SND_FILENAME)
    {
        HANDLE hf = CreateFile(src, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hf == INVALID_HANDLE_VALUE) { au->iserr = true; return AVIERR_FILEOPEN; }
        DWORD size = GetFileSize(hf, NULL);
        buf = new char[size];
        DWORD red; ReadFile(hf, buf, size, &red, NULL);
        CloseHandle(hf);
        wav = (WavChunk*)buf;
    }
    //
    // check that format doesn't clash
    bool badformat = false;
    if (au->wfx.nChannels == 0)
    {
        au->wfx.wFormatTag = wav->fmt.wFormatTag;
        au->wfx.cbSize = 0;
        au->wfx.nAvgBytesPerSec = wav->fmt.dwAvgBytesPerSec;
        au->wfx.nBlockAlign = wav->fmt.wBlockAlign;
        au->wfx.nChannels = wav->fmt.wChannels;
        au->wfx.nSamplesPerSec = wav->fmt.dwSamplesPerSec;
        au->wfx.wBitsPerSample = wav->fmt.wBitsPerSample;
    } else
    {
        if (au->wfx.wFormatTag != wav->fmt.wFormatTag) badformat = true;
        if (au->wfx.nAvgBytesPerSec != wav->fmt.dwAvgBytesPerSec) badformat = true;
        if (au->wfx.nBlockAlign != wav->fmt.wBlockAlign) badformat = true;
        if (au->wfx.nChannels != wav->fmt.wChannels) badformat = true;
        if (au->wfx.nSamplesPerSec != wav->fmt.dwSamplesPerSec) badformat = true;
        if (au->wfx.wBitsPerSample != wav->fmt.wBitsPerSample) badformat = true;
    }
    if (badformat) { if (buf != 0) delete[] buf; return AVIERR_BADFORMAT; }
    //
    if (au->as == 0) // create the stream if necessary
    {
        AVISTREAMINFO ahdr; ZeroMemory(&ahdr, sizeof(ahdr));
        ahdr.fccType = streamtypeAUDIO;
        ahdr.dwScale = au->wfx.nBlockAlign;
        ahdr.dwRate = au->wfx.nSamplesPerSec*au->wfx.nBlockAlign;
        ahdr.dwSampleSize = au->wfx.nBlockAlign;
        ahdr.dwQuality = (DWORD)-1;
        HRESULT hr = AVIFileCreateStream(au->pfile, &au->as, &ahdr);
        if (hr != AVIERR_OK) { if (buf != 0) delete[] buf; au->iserr = true; return hr; }
        hr = AVIStreamSetFormat(au->as, 0, &au->wfx, sizeof(WAVEFORMATEX));
        if (hr != AVIERR_OK) { if (buf != 0) delete[] buf; au->iserr = true; return hr; }
    }
    //
    // now we can write the data
    unsigned long numbytes = wav->dat.size;
    unsigned long numsamps = numbytes * 8 / au->wfx.wBitsPerSample;
    HRESULT hr = AVIStreamWrite(au->as, au->nsamp, numsamps, wav->dat.data, numbytes, 0, NULL, NULL);
    if (buf != 0) delete[] buf;
    if (hr != AVIERR_OK) { au->iserr = true; return hr; }
    au->nsamp += numsamps; return S_OK;
}



unsigned int FormatAviMessage(HRESULT code, char *buf, unsigned int len)
{
    const char *msg = "unknown avi result code";
    switch (code)
    {
    case S_OK: msg = "Success"; break;
    case AVIERR_BADFORMAT: msg = "AVIERR_BADFORMAT: corrupt file or unrecognized format"; break;
    case AVIERR_MEMORY: msg = "AVIERR_MEMORY: insufficient memory"; break;
    case AVIERR_FILEREAD: msg = "AVIERR_FILEREAD: disk error while reading file"; break;
    case AVIERR_FILEOPEN: msg = "AVIERR_FILEOPEN: disk error while opening file"; break;
    case REGDB_E_CLASSNOTREG: msg = "REGDB_E_CLASSNOTREG: file type not recognised"; break;
    case AVIERR_READONLY: msg = "AVIERR_READONLY: file is read-only"; break;
    case AVIERR_NOCOMPRESSOR: msg = "AVIERR_NOCOMPRESSOR: a suitable compressor could not be found"; break;
    case AVIERR_UNSUPPORTED: msg = "AVIERR_UNSUPPORTED: compression is not supported for this type of data"; break;
    case AVIERR_INTERNAL: msg = "AVIERR_INTERNAL: internal error"; break;
    case AVIERR_BADFLAGS: msg = "AVIERR_BADFLAGS"; break;
    case AVIERR_BADPARAM: msg = "AVIERR_BADPARAM"; break;
    case AVIERR_BADSIZE: msg = "AVIERR_BADSIZE"; break;
    case AVIERR_BADHANDLE: msg = "AVIERR_BADHANDLE"; break;
    case AVIERR_FILEWRITE: msg = "AVIERR_FILEWRITE: disk error while writing file"; break;
    case AVIERR_COMPRESSOR: msg = "AVIERR_COMPRESSOR"; break;
    case AVIERR_NODATA: msg = "AVIERR_READONLY"; break;
    case AVIERR_BUFFERTOOSMALL: msg = "AVIERR_BUFFERTOOSMALL"; break;
    case AVIERR_CANTCOMPRESS: msg = "AVIERR_CANTCOMPRESS"; break;
    case AVIERR_USERABORT: msg = "AVIERR_USERABORT"; break;
    case AVIERR_ERROR: msg = "AVIERR_ERROR"; break;
    }
    unsigned int mlen = (unsigned int)strlen(msg);
    if (buf == 0 || len == 0) return mlen;
    unsigned int n = mlen; if (n + 1 > len) n = len - 1;
    strncpy(buf, msg, n); buf[n] = 0;
    return mlen;
}



void CChild_2_Frame::OnSaveWmf()
{
    CRect       WinRec;	GetClientRect(WinRec);	// window size	
    CDC *pDC = GetDC();
    CMetaFileDC myM;
    CString name, nm;

    PicsTotal++;
    GetWindowText(nm);
    name.Format("pictures\\%d %s.wmf", PicsTotal, nm);
    myM.CreateEnhanced(pDC, name, NULL, NULL);
    myM.BitBlt(0, 0, WinRec.Width(), WinRec.Height() - 2, GetDC(), 0, 0, SRCCOPY);

    DeleteEnhMetaFile(myM.CloseEnhanced());


}

int CChild_2_Frame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    pdc = new CClientDC(this);

    if (SetPixelFormat(pdc->m_hDC) == FALSE) return -1;
    hGLRC = wglCreateContext(pdc->m_hDC);
    if (hGLRC == NULL) return -1;

    if (wglMakeCurrent(pdc->m_hDC, hGLRC) == FALSE) return -1;

    glEnable(GL_ALPHA_TEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /*
        float pos[4] = {3,3,3,1};
        float dir[3] = {-1,-1,-1};
        glLightfv(GL_LIGHT0, GL_POSITION, pos);
        glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHTING);
     */


     //Initial Update//
    GLfloat ambient[] = { 0.9, 0.9, 0.9, 0.5 };
    GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat specular[] = { 0.5, 0.5, 0.5, 0.5 };
    GLfloat position[] = { 3.0, 3.0, 3.0, 0.0 };

    GLfloat lmodel_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat local_view[] = { 0.0 };

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);

    glFrontFace(GL_CW);
    //	glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Определяем цвет фона используемый по умолчанию
    ::glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
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

    wglMakeCurrent(pdc->m_hDC, hGLRC);
    glViewport(10, 10, cx - 20, cy - 20);


}
