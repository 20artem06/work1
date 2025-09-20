/////////////////////////////////////////////////////////////////////////////
// Field_2.cpp : implementation file
//

#include "stdafx.h"
#include <fstream>

#include "video.h"
#include "F_2_dlg.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CField_2_Dlg dialog

CField_2_Dlg::CField_2_Dlg(CWnd* pParent /*=NULL*/)
    : CDialog(CField_2_Dlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CField_2_Dlg)
    m_nProject = -1;
    m_nField = -1;
    m_fFmin = 0.0f;
    m_fFmax = 0.0f;
    m_bPolar = FALSE;
    m_nArg1 = -1;
    m_nX1min = -1;
    m_nX1max = -1;
    m_nArg2 = -1;
    m_nX2min = -1;
    m_nX2max = -1;
    m_nArgument = -1;
    m_nParam = -1;
    m_nGrids = 10;
    m_bGridLines = FALSE;
    m_bGridAuto = FALSE;
    m_lines = FALSE;
    m_cloud = TRUE;
    //m_layers = TRUE;
    m_Palette = 1;
    m_bLog = FALSE;

    //}}AFX_DATA_INIT
}

void CField_2_Dlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CField_2_Dlg)
    DDX_CBIndex(pDX, IDC_PROJECT, m_nProject);
    DDX_CBIndex(pDX, IDC_FIELD, m_nField);
    DDX_Text(pDX, IDC_FMIN, m_fFmin);
    DDX_Text(pDX, IDC_FMAX, m_fFmax);
    //DDX_Check   (pDX, IDC_POLAR,   m_bPolar);
    DDX_CBIndex(pDX, IDC_ARG1, m_nArg1);
    DDX_CBIndex(pDX, IDC_ARG1MIN, m_nX1min);
    DDX_CBIndex(pDX, IDC_ARG1MAX, m_nX1max);
    DDX_CBIndex(pDX, IDC_ARG2, m_nArg2);
    DDX_CBIndex(pDX, IDC_ARG2MIN, m_nX2min);
    DDX_CBIndex(pDX, IDC_ARG2MAX, m_nX2max);
    DDX_CBIndex(pDX, IDC_ARGUMENT, m_nArgument);
    DDX_LBIndex(pDX, IDC_PARAM, m_nParam);
    DDX_Text(pDX, IDC_ARGNAME, m_sArgName);
    DDX_Text(pDX, IDC_NGRIDS, m_nGrids);
    DDX_Check(pDX, IDC_GRIDLINES, m_bGridLines);
    DDX_Check(pDX, IDC_LINES, m_lines);
    DDX_Check(pDX, IDC_CLOUD, m_cloud);
    //DDX_Check(pDX, IDC_LAYERS, m_layers);
    DDX_Text(pDX, IDC_EDIT1, m_Palette);
    DDX_Check(pDX, IDC_LOG, m_bLog);
    DDX_Check(pDX, IDC_GRDAUTO, m_bGridAuto);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CField_2_Dlg, CDialog)
    //{{AFX_MSG_MAP(CField_2_Dlg)
    ON_BN_CLICKED(IDC_OK, OnClickedOk)
    ON_CBN_SELCHANGE(IDC_PROJECT, OnSelectProject)
    ON_CBN_SELCHANGE(IDC_FIELD, OnFieldModify)
    ON_CBN_SELCHANGE(IDC_ARG1, OnChangeArg1)
    ON_CBN_SELCHANGE(IDC_ARG2, OnChangeArg2)
    ON_LBN_SELCHANGE(IDC_PARAM, OnSelectParam)
    ON_CBN_SELCHANGE(IDC_ARGUMENT, OnSelectNewArgument)
    ON_EN_KILLFOCUS(IDC_FMIN, OnChangeFmin)
    ON_EN_KILLFOCUS(IDC_FMAX, OnChangeFmax)
    ON_CBN_SELCHANGE(IDC_ARG1MAX, OnSelectArg1max)
    ON_CBN_SELCHANGE(IDC_ARG1MIN, OnSelectArg1min)
    ON_CBN_SELCHANGE(IDC_ARG2MAX, OnSelectArg2max)
    ON_CBN_SELCHANGE(IDC_ARG2MIN, OnSelectArg2min)
    ON_EN_KILLFOCUS(IDC_NGRIDS, OnChangeNgrids)
    ON_WM_PAINT()
    ON_BN_CLICKED(IDC_COLORMIN, OnColorMin)
    ON_BN_CLICKED(IDC_COLORMAX, OnColorMax)
    ON_BN_CLICKED(IDC_LOADPRESET, OnBnClickedLoadpreset)
    ON_BN_CLICKED(IDC_SAVEPRESET, OnBnClickedSavepreset)
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_LOG, &CField_2_Dlg::OnBnClickedLog)
    ON_BN_CLICKED(IDC_LINES, &CField_2_Dlg::OnBnClickedLines)
    ON_BN_CLICKED(IDC_CHECK1, &CField_2_Dlg::OnBnClickedCheck1)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CField_2_Dlg message handlers
void CField_2_Dlg::OnOK()
{
    NextDlgCtrl();
}

void CField_2_Dlg::OnClickedOk()
{
    CDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// Beginning
BOOL CField_2_Dlg::OnInitDialog()
{
    CDialog::OnInitDialog();			// standart call
    CVideoApp* pApp = (CVideoApp*)AfxGetApp();
    CComboBox* pFBox = (CComboBox*)GetDlgItem(IDC_PROJECT);

    // Clear for NEW Graph
    if (bNewGraph)
    {
        pProject = pApp->First;			// Current project
        ColorMin = RGB(0, 0, 255);		// Initial palette - Blue-Red
        ColorMax = RGB(255, 0, 0);
        SetWindowText("Select new TWO-DIM graph");
    } else
        SetWindowText("Modify TWO-DIM graph");

    // Project - all items
    int i = 0;
    CProject* pTmp = pApp->First;
    do
    {
        pFBox->AddString(pTmp->m_sName);
        if (pTmp == pProject) m_nProject = (int)i;
        pTmp = pTmp->Next;
        i++;
    } while (pTmp != pApp->First);

    // Project - current item
    pFBox->SetCurSel(m_nProject);
    UpdateData(FALSE);
    OnSelectProject();

    return TRUE; //standart return
}

/////////////////////////////////////////////////////////////////////////////
// Change Project
void CField_2_Dlg::OnSelectProject()
{
    CVideoApp* pApp = (CVideoApp*)AfxGetApp();
    CComboBox* pFBox = (CComboBox*)GetDlgItem(IDC_FIELD);
    int i, n;

    //Find project
    int	nProjectOld = m_nProject;		if (bNewGraph) UpdateData();
    n = abs(m_nProject - nProjectOld);
    if (nProjectOld < m_nProject)
        for (i = 0; i < n; i++) pProject = pProject->Next;
    else
        for (i = 0; i < n; i++) pProject = pProject->Prev;

    // Clear for NEW Graph
    if (bNewGraph)
    {
        if (pProject->m_bOneDimOnly)
            m_nField = 0;					// Any field
        else
        {
            for (i = 0; i < pProject->m_nFields; i++)
                if (pProject->m_field[i].nAxis > 0) break;
            m_nField = i;					// TWO-Dim field 
        }
    }

    // Fields - all items
    pFBox->ResetContent();				    // Clear fields
    for (i = 0; i < pProject->m_nFields; i++)
        pFBox->AddString(pProject->m_field[i].name + " ["
            + pProject->m_field[i].size + "] ");

    // Fields - current	item
    pFBox->SetCurSel(m_nField);
    UpdateData(FALSE);
    OnFieldModify();
}

////////////////////////////////////////////////////////////////////////
// Change Field
VOID CField_2_Dlg::OnFieldModify()
{
    CComboBox* pX1Box = (CComboBox*)GetDlgItem(IDC_ARG1);
    CComboBox* pX1Min = (CComboBox*)GetDlgItem(IDC_ARG1MIN);
    CComboBox* pX1Max = (CComboBox*)GetDlgItem(IDC_ARG1MAX);
    CComboBox* pX2Box = (CComboBox*)GetDlgItem(IDC_ARG2);
    CComboBox* pX2Min = (CComboBox*)GetDlgItem(IDC_ARG2MIN);
    CComboBox* pX2Max = (CComboBox*)GetDlgItem(IDC_ARG2MAX);
    CListBox* pParam = (CListBox*)GetDlgItem(IDC_PARAM);
    CComboBox* pArg = (CComboBox*)GetDlgItem(IDC_ARGUMENT);

    int i;
    CString ttt;

    // Clear for NEW Graph
    if (bNewGraph)
    {
        UpdateData();
        m_nArg1 = 0;
        m_nArg2 = 1;
        m_bPolar = FALSE;   // Decart
        m_nGrids = 10;

        INDEX.resize(pProject->m_field[m_nField].nAxis + 1);
        for (i = 0; i <= pProject->m_field[m_nField].nAxis; i++) {
            INDEX[i] = 0;
        }
    }

    // TWO-Dim graph can't be build
    if (pProject->m_field[m_nField].nAxis == 0)
    {
        pX1Box->EnableWindow(FALSE);//close all
        pX1Min->EnableWindow(FALSE);
        pX1Max->EnableWindow(FALSE);
        pX2Box->EnableWindow(FALSE);
        pX2Min->EnableWindow(FALSE);
        pX2Max->EnableWindow(FALSE);
        pParam->EnableWindow(FALSE);
        pArg->EnableWindow(FALSE);
        return;						//and bye
    }
    // TWO-Dim graph can be build
    else
    {
        pX1Box->EnableWindow();		//open all
        pX1Min->EnableWindow();
        pX1Max->EnableWindow();
        pX2Box->EnableWindow();
        pX2Min->EnableWindow();
        pX2Max->EnableWindow();
        pParam->EnableWindow();
        pArg->EnableWindow();	   //and continue
    }

    // Arg1,Arg2 - all items
    pX1Box->ResetContent();					    // Clear Arg1,Arg2
    pX2Box->ResetContent();
    for (i = 0; i < pProject->m_field[m_nField].nAxis; i++)
    {
        ttt = pProject->m_field[m_nField].axe[i].name + " ["
            + pProject->m_field[m_nField].axe[i].size + "] ";
        pX1Box->AddString(ttt);
        pX2Box->AddString(ttt);					// Add axes
    }
    ttt = pProject->m_sTime + " [" + pProject->m_sTimeSize + "]";
    pX1Box->AddString(ttt);
    pX2Box->AddString(ttt);					// Add time

//	Arg1 - current item
    pX1Box->SetCurSel(m_nArg1);
    UpdateData(FALSE);
    OnChangeArg1();
}

////////////////////////////////////////////////////////////////////////
// Change Argument 1
void CField_2_Dlg::OnChangeArg1()
{
    int i;
    CString ttt;

    CComboBox* pX1Min = (CComboBox*)GetDlgItem(IDC_ARG1MIN);
    CComboBox* pX1Max = (CComboBox*)GetDlgItem(IDC_ARG1MAX);
    CComboBox* pX2Box = (CComboBox*)GetDlgItem(IDC_ARG2);
    CButton* pOK = (CButton*)GetDlgItem(IDC_OK);

    // Clear for NEW Graph
    if (bNewGraph)
    {
        UpdateData();

        // Arg2 != Arg1 EVERYWHERE !!!
        if (m_nArg2 == m_nArg1) m_nArg2 = m_nArg1 + 1;
        if (m_nArg2 == (int)pProject->m_field[m_nField].nAxis + 1) m_nArg2 = 0;

        // Arg1 - current min,max
        m_nX1min = 0;							// Min is [0] everywhere
        if (m_nArg1 < (int)pProject->m_field[m_nField].nAxis) // Max for axe
            m_nX1max = pProject->m_field[m_nField].axe[m_nArg1].nPoints - 1;
        else										       // Max for time
            m_nX1max = pProject->m_nTimePoints - 1;
        if (m_nX1max > TabMax)m_nX1max = TabMax;
    }

    // Arg1 min,max - all items
    pX1Min->ResetContent();					// Clear Arg1Min, Arg1Max
    pX1Max->ResetContent();
    if (m_nArg1 < (int)pProject->m_field[m_nField].nAxis)	//Min,max for axe
        for (i = 0; i < pProject->m_field[m_nField].axe[m_nArg1].nPoints; i++)
        {
            ttt.Format("%g", pProject->m_field[m_nField].axe[m_nArg1].point[i]);
            pX1Min->AddString(ttt);
            pX1Max->AddString(ttt);
        } else												//Min,max for time
            for (i = 0; i < pProject->m_nTimePoints; i++)
            {
                ttt.Format("%g", pProject->m_time[i]);
                pX1Min->AddString(ttt);
                pX1Max->AddString(ttt);
            }

        //	Arg2 - current item
        pX2Box->SetCurSel(m_nArg2);
        UpdateData(FALSE);
        OnChangeArg2();

        pOK->EnableWindow(true);
        if ((m_nArg1 < pProject->m_field[m_nField].nAxis) && (m_nArg2 < pProject->m_field[m_nField].nAxis))
        {
            if ((pProject->m_field[m_nField].axe[m_nArg1].nPoints < 2) || (pProject->m_field[m_nField].axe[m_nArg2].nPoints < 2))
                pOK->EnableWindow(false);
        }
        if (pProject->m_nTimePoints < 2)
            pOK->EnableWindow(false);
}

////////////////////////////////////////////////////////////////////////
// Change Argument 2			
void CField_2_Dlg::OnChangeArg2()
{
    int i;
    CString ttt1, ttt2, ttt3;

    CComboBox* pX2Min = (CComboBox*)GetDlgItem(IDC_ARG2MIN);
    CComboBox* pX2Max = (CComboBox*)GetDlgItem(IDC_ARG2MAX);
    CListBox* pParam = (CListBox*)GetDlgItem(IDC_PARAM);
    CComboBox* pArg = (CComboBox*)GetDlgItem(IDC_ARGUMENT);
    CButton* pOK = (CButton*)GetDlgItem(IDC_OK);

    // Clear for NEW Graph
    if (bNewGraph)
    {
        UpdateData();

        // Arg2 != Arg1 EVERYWHERE !!!
        if (m_nArg2 == m_nArg1) m_nArg2 = m_nArg1 + 1;
        if (m_nArg2 == (int)pProject->m_field[m_nField].nAxis + 1) m_nArg2 = 0;

        // Arg2 - current min,max
        m_nX2min = 0;							// Min is [0] everywhere
        if (m_nArg2 < (int)pProject->m_field[m_nField].nAxis) // Max for axe
            m_nX2max = pProject->m_field[m_nField].axe[m_nArg2].nPoints - 1;
        else										       // Max for time
            m_nX2max = pProject->m_nTimePoints - 1;
        if (m_nX2max > TabMax)m_nX2max = TabMax;
    }

    // Arg2 min,max - all
    pX2Min->ResetContent();					//Clear Agr2Min,Agr2Max
    pX2Max->ResetContent();
    if (m_nArg2 < (int)pProject->m_field[m_nField].nAxis)	//Min,max for axe
        for (i = 0; i < pProject->m_field[m_nField].axe[m_nArg2].nPoints; i++)
        {
            ttt1.Format("%g", pProject->m_field[m_nField].axe[m_nArg2].point[i]);
            pX2Min->AddString(ttt1);
            pX2Max->AddString(ttt1);
        } else												//Min,max for time
            for (i = 0; i < pProject->m_nTimePoints; i++)
            {
                ttt1.Format("%g", pProject->m_time[i]);
                pX2Min->AddString(ttt1);
                pX2Max->AddString(ttt1);
            }

        //  Parameters - all
        pParam->ResetContent();					//Clear Parameters
        for (i = 0; i < pProject->m_field[m_nField].nAxis; i++)	//Axes  Parameters
        {
            ttt1 = pProject->m_field[m_nField].axe[i].name;
            ttt2 = " [" + pProject->m_field[m_nField].axe[i].size + "]";
            if ((int)i == m_nArg1)					   //Axe is Arg1 
                pParam->AddString(ttt1 + ttt2 + " - AGRUMENT 1");
            else  if ((int)i == m_nArg2)					   //Axe is Arg2 
                pParam->AddString(ttt1 + ttt2 + " - AGRUMENT 2");
            else											   //Axe isn't Argument
            {
                ttt1 = ttt1 + " = ";
                ttt3.Format("%g", pProject->m_field[m_nField].axe[i].point[INDEX[i]]);
                pParam->AddString(ttt1 + ttt3 + ttt2);
            }
        }
        ttt1 = pProject->m_sTime;							   //Time Parameter
        ttt2 = " [" + pProject->m_sTimeSize + "]";
        if ((int)i == m_nArg1)						   //Time is Arg1 
            pParam->AddString(ttt1 + ttt2 + " - AGRUMENT 1");
        else  if ((int)i == m_nArg2)						   //Time is Arg2 
            pParam->AddString(ttt1 + ttt2 + " - AGRUMENT 2");
        else												   //Time isn't Argument
        {
            ttt1 = ttt1 + " = ";
            ttt3.Format("%12.3e", pProject->m_time[INDEX[i]]);
            pParam->AddString(ttt1 + ttt3 + ttt2);
        }

        //	Paramert - current
        if (pProject->m_field[m_nField].nAxis < 2)
        {
            m_nParam = 0;				// Any parameter
            pParam->EnableWindow(FALSE);// Close all parameter windows
            pArg->ResetContent();
            pArg->EnableWindow(FALSE);
            m_sArgName = "";
            if (bNewGraph)	// Fmin,Fmax for new graph			
            {
                m_fFmin = pProject->m_field[m_nField].min;
                m_fFmax = pProject->m_field[m_nField].max;
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
            UpdateData(FALSE);
            bNewGraph = TRUE;
            return; 					// and bye
        } else
        {
            for (i = 0; i <= pProject->m_field[m_nField].nAxis; i++)
                if (((int)i != m_nArg1) && ((int)i != m_nArg2)) break;
            m_nParam = i;				// First parameter not equal to Arg1,Arg2
            pParam->EnableWindow();		// Open all parameter windows
            pArg->EnableWindow();
            UpdateData(FALSE);
            OnSelectParam(); 			// and continue
        }

        pOK->EnableWindow(true);
        if ((m_nArg1 < pProject->m_field[m_nField].nAxis) && (m_nArg2 < pProject->m_field[m_nField].nAxis))
        {
            if ((pProject->m_field[m_nField].axe[m_nArg1].nPoints < 2) || (pProject->m_field[m_nField].axe[m_nArg2].nPoints < 2))
                pOK->EnableWindow(false);
        }
        if (pProject->m_nTimePoints < 2)
            pOK->EnableWindow(false);

}

////////////////////////////////////////////////////////////////////////
// Change Parameter Number
void CField_2_Dlg::OnSelectParam()
{
    int i;
    CString ttt;

    CListBox* pParam = (CListBox*)GetDlgItem(IDC_PARAM);
    CComboBox* pArg = (CComboBox*)GetDlgItem(IDC_ARGUMENT);

    // NEW Graph
    if (bNewGraph)		UpdateData();

    // Change Parameter if it equals to Arg1 or Arg2
    if ((m_nParam == m_nArg1) || (m_nParam == m_nArg2))
    {
        for (i = 0; i <= pProject->m_field[m_nField].nAxis; i++)
            if (((int)i != m_nArg1) && ((int)i != m_nArg2)) break;
        m_nParam = i;
    }
    pParam->SetCurSel(m_nParam);

    // Current Parameter - all items
    pArg->ResetContent();								//Clear CurrentParameter
    if (m_nParam < (int)pProject->m_field[m_nField].nAxis)//CurrentParameter - axe
    {
        for (i = 0; i < pProject->m_field[m_nField].axe[m_nParam].nPoints; i++)
        {
            ttt.Format("%g", pProject->m_field[m_nField].axe[m_nParam].point[i]);
            pArg->AddString(ttt);
        }
        m_sArgName = pProject->m_field[m_nField].axe[m_nParam].name + " ["
            + pProject->m_field[m_nField].axe[m_nParam].size + "] ";
    } else												//CurrentParameter - time
    {
        for (i = 0; i < pProject->m_nTimePoints; i++)
        {
            ttt.Format("%g", pProject->m_time[i]);
            pArg->AddString(ttt);
        }
        m_sArgName = pProject->m_sTime + " [" + pProject->m_sTimeSize + "]";
    }

    //	Current Parameter - current 
    m_nArgument = INDEX[m_nParam];
    pArg->SetCurSel(m_nArgument);
    UpdateData(FALSE);
    OnSelectNewArgument();
}

////////////////////////////////////////////////////////////////////////
// Change Current Parameter
void CField_2_Dlg::OnSelectNewArgument()
{
    CString ttt1, ttt2, ttt3;

    CListBox* pParam = (CListBox*)GetDlgItem(IDC_PARAM);

    // NEW Graph
    if (bNewGraph)
    {
        UpdateData();
        m_fFmin = pProject->m_field[m_nField].min;
        m_fFmax = pProject->m_field[m_nField].max;
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
    INDEX[m_nParam] = m_nArgument;

    //Prepare Parameter string
    pParam->DeleteString(m_nParam);
    if (m_nParam < (int)pProject->m_field[m_nField].nAxis)//Current Parameter - axe
    {
        ttt1 = pProject->m_field[m_nField].axe[m_nParam].name + " = ";
        ttt2 = " [" + pProject->m_field[m_nField].axe[m_nParam].size + "]";
        ttt3.Format("%g", pProject->m_field[m_nField].axe[m_nParam].point[m_nArgument]);
    } else
    {													    //Current Parameter - time
        ttt1 = pProject->m_sTime + " = ";
        ttt2 = " [" + pProject->m_sTimeSize + "]";
        ttt3.Format("%g", pProject->m_time[m_nArgument]);
    }
    pParam->InsertString(m_nParam, ttt1 + ttt3 + ttt2);
    pParam->SetCurSel(m_nParam);

    UpdateData(FALSE);
}

////////////////////////////////////////////////////////////////////////
//	  Change Min,Max. Prepare Min<Max .

void CField_2_Dlg::OnChangeFmin()
{
    UpdateData();
    if (m_bLog)
    {
        float FminCurr = pProject->m_field[m_nField].min;
        float FmaxCurr = pProject->m_field[m_nField].max;
        if (FminCurr > 0)
            FminCurr = log10(FminCurr);
        else
            FminCurr = -37;

        if (FmaxCurr > 0)
            FmaxCurr = log10(FmaxCurr);
        else
            FmaxCurr = 37;

        if (m_fFmin < m_fFmin)
            m_fFmin = m_fFmin;
        if (m_fFmin > FmaxCurr)
            m_fFmin = FmaxCurr;
    }
    else
    {
        if (m_fFmin < pProject->m_field[m_nField].min)
            m_fFmin = pProject->m_field[m_nField].min;
        if (m_fFmin > m_fFmax)
            m_fFmin = m_fFmax;
    }
    UpdateData(FALSE);
}

void CField_2_Dlg::OnChangeFmax()
{
    UpdateData();
    if (m_bLog)
    {
        float FminCurr = pProject->m_field[m_nField].min;
        float FmaxCurr = pProject->m_field[m_nField].max;
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
        if (m_fFmax > pProject->m_field[m_nField].max)
            m_fFmax = pProject->m_field[m_nField].max;
    }
    UpdateData(FALSE);
}

void CField_2_Dlg::OnSelectArg1max()
{
    UpdateData();
    if (m_nX1max <= m_nX1min) m_nX1max = m_nX1min + 1;
    m_fFmin = pProject->m_field[m_nField].min;
    m_fFmax = pProject->m_field[m_nField].max;
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

void CField_2_Dlg::OnSelectArg1min()
{
    UpdateData();
    if (m_nX1min >= m_nX1max) m_nX1min = m_nX1max - 1;
    m_fFmin = pProject->m_field[m_nField].min;
    m_fFmax = pProject->m_field[m_nField].max;
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

void CField_2_Dlg::OnSelectArg2max()
{
    UpdateData();
    if (m_nX2max <= m_nX2min) m_nX2max = m_nX2min + 1;
    m_fFmin = pProject->m_field[m_nField].min;
    m_fFmax = pProject->m_field[m_nField].max;
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

void CField_2_Dlg::OnSelectArg2min()
{
    UpdateData();
    if (m_nX2min >= m_nX2max) m_nX2min = m_nX2max - 1;
    m_fFmin = pProject->m_field[m_nField].min;
    m_fFmax = pProject->m_field[m_nField].max;
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

void CField_2_Dlg::OnChangeNgrids() //    ( 2 < Ngrids <300 )
{
    UpdateData();
    if (m_nGrids < 2) m_nGrids = 2;
    if (m_nGrids > 300) m_nGrids = 300;
    UpdateData(FALSE);
}

////////////////////////////////////////////////////////////////////////
// REDRAW "Show color" rectangle
void CField_2_Dlg::OnPaint()
{
    CPaintDC dc(this); // device context for painting	
    const int nL = 12;

    CStatic* pWnd = (CStatic*)GetDlgItem(IDC_COL);
    CDC* pControlDC = pWnd->GetDC();
    CRect	 pW;          pWnd->GetClientRect(&pW); //Take window size

    pWnd->Invalidate();
    pWnd->UpdateWindow();

    pControlDC->FillSolidRect(pW, RGB(255, 255, 255)); //Fill window by white color

    int n_hR = (GetRValue(ColorMax) - GetRValue(ColorMin)) / nL;
    int	n_hG = (GetGValue(ColorMax) - GetGValue(ColorMin)) / nL;
    int n_hB = (GetBValue(ColorMax) - GetBValue(ColorMin)) / nL;
    COLORREF Color = ColorMin;

    CPen  myPen(PS_SOLID, 1, Color);
    CPen* oldPen = pControlDC->SelectObject(&myPen);
    int i, k;

    for (i = 0; i < nL; i++)
    {
        dc.SelectObject(oldPen);
        myPen.DeleteObject();
        myPen.CreatePen(PS_SOLID, 1, Color);
        pControlDC->SelectObject(&myPen);

        k = pW.bottom - (int)((i + 0.5) * pW.Height() / nL);
        pControlDC->MoveTo(pW.left + 2, k);
        pControlDC->LineTo(pW.right - 2, k);

        Color = RGB(GetRValue(Color) + n_hR,
            GetGValue(Color) + n_hG,
            GetBValue(Color) + n_hB);
    }
    pControlDC->SelectObject(oldPen);
    myPen.DeleteObject();

    pWnd->ReleaseDC(pControlDC);
}

////////////////////////////////////////////////////////////////////////
// Call Color dialogs for ColorMax and ColorMin

void CField_2_Dlg::OnColorMin()
{
    CColorDialog dlg;
    if (dlg.DoModal() == IDOK)
        ColorMin = dlg.GetColor();//Change Color after dialog
    UpdateData(FALSE);
    OnPaint();					//Redraw
}

void CField_2_Dlg::OnColorMax()
{
    CColorDialog dlg;
    if (dlg.DoModal() == IDOK)
        ColorMax = dlg.GetColor();//Change Color after dialog
    UpdateData(FALSE);
    OnPaint();					//Redraw
}


void CField_2_Dlg::OnBnClickedLoadpreset()
{   
    CString fName = "VideoPreset.json";
    std::ifstream source (pProject->projectPath + fName);

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

    CComboBox* pX2Box = (CComboBox*)GetDlgItem(IDC_ARG2);
    CComboBox* pX2Min = (CComboBox*)GetDlgItem(IDC_ARG2MIN);
    CComboBox* pX2Max = (CComboBox*)GetDlgItem(IDC_ARG2MAX);

    CListBox* pParam = (CListBox*)GetDlgItem(IDC_PARAM);
    CComboBox* pArg = (CComboBox*)GetDlgItem(IDC_ARGUMENT);



    m_nArg1 = root["TwoDim"]["Arg1"]["Value"].get<int>();
    pX1Box->SetCurSel(root["TwoDim"]["Arg1"]["Value"].get<int>());
    OnChangeArg1();

    m_nArg2 = root["TwoDim"]["Arg2"]["Value"].get<int>();
    pX2Box->SetCurSel(root["TwoDim"]["Arg2"]["Value"].get<int>());
    OnChangeArg2();

    int arg1_points, arg2_points;

    if (m_nArg1 < pProject->m_field[m_nField].nAxis)
        arg1_points = pProject->m_field[m_nField].axe[m_nArg1].nPoints - 1;
    else
        arg1_points = pProject->m_nTimePoints - 1;
    

    if (m_nArg2 < pProject->m_field[m_nField].nAxis) 
        arg2_points = pProject->m_field[m_nField].axe[m_nArg2].nPoints - 1;
    else 
        arg2_points = pProject->m_nTimePoints - 1;
    


    int arg1LoadMax = root["TwoDim"]["Arg1"]["Max"].get<int>();
    int arg2LoadMax = root["TwoDim"]["Arg2"]["Max"].get<int>();
    int arg1LoadMin = root["TwoDim"]["Arg1"]["Min"].get<int>();
    int arg2LoadMin = root["TwoDim"]["Arg2"]["Min"].get<int>();


    int arg1_max = (arg1LoadMax <= arg1_points) ? arg1LoadMax : arg1_points;
    int arg2_max = (arg2LoadMax <= arg2_points) ? arg2LoadMax : arg2_points;
    int arg1_min = (arg1LoadMin <  arg1_points) ? arg1LoadMin : 0;
    int arg2_min = (arg2LoadMin <  arg2_points) ? arg2LoadMin : 0;



    pX1Min->SetCurSel(arg1_min);
    pX1Max->SetCurSel(arg1_max);
    pX2Min->SetCurSel(arg2_min);
    pX2Max->SetCurSel(arg2_max);
    UpdateData();

    //INDEX массив, где хранятся индексы, полученные из Меню параметров (правый листбокс)
    // нагружаю в этот массив данные из файла
    for (int i = 0; i < pProject->m_field[m_nField].nAxis + 1; i++) {
        INDEX[i] = root["TwoDim"]["MainArg"].value(std::to_string(i), 0);
    }

    //цикл, вызывающий обновление интерфейса с данными из массива INDEX
    for (int i = 0; i < pProject->m_field[m_nField].nAxis + 1; i++) {
        if (i != m_nArg1 && i != m_nArg2) {
            pParam->SetCurSel(i);
            OnSelectParam();
        }
    }
}


void CField_2_Dlg::OnBnClickedSavepreset()
{
    json root;

	root["TwoDim"]["MainArg"]["Min"] = m_fFmin;
	root["TwoDim"]["MainArg"]["Max"] = m_fFmax;

    root["TwoDim"]["Arg1"]["Value"] = m_nArg1;
    root["TwoDim"]["Arg1"]["Min"] = m_nX1min;
    root["TwoDim"]["Arg1"]["Max"] = m_nX1max;

    root["TwoDim"]["Arg2"]["Value"] = m_nArg2;
    root["TwoDim"]["Arg2"]["Min"] = m_nX2min;
    root["TwoDim"]["Arg2"]["Max"] = m_nX2max;


    for (int i = 0; i < pProject->m_field[m_nField].nAxis + 1; i++) {
        root["TwoDim"]["MainArg"][std::to_string(i)] = INDEX[i];
    }

    std::ofstream out;
    CString fName = "VideoPreset.json";

    //подкачка из файла значений для одномерных графиков
    std::ifstream source(pProject->projectPath + fName);
    json tmpRoot;
    if (source.is_open())
    {
        try {
            source >> tmpRoot;
        }
        catch (std::exception e) {
            tmpRoot["OneDim"];
        }
    } else {
        return;
    }
    source.close();

    root["OneDim"] = tmpRoot["OneDim"];

    out.open(pProject->projectPath + fName);
    if (out.is_open())
    {
        out << root << std::endl;
    }

    out.close();
}


void CField_2_Dlg::OnBnClickedLog()
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
}


void CField_2_Dlg::OnBnClickedLines()
{
   
}


void CField_2_Dlg::OnBnClickedCheck1()
{
    // TODO: добавьте свой код обработчика уведомлений
}
