// AviDlg.cpp : implementation file
//

#include "stdafx.h"
#include "video.h"
#include "AviDlg.h"

#include "F_2_dlg.h"
#include "Child_2.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAviDlg dialog


CAviDlg::CAviDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CAviDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CAviDlg)
    m_FileName = _T("");
    m_MinMaxMode = FALSE;
    m_Delay = 0;
    //}}AFX_DATA_INIT
    pGraph = (CChild_2_Frame*)pParent;
}


void CAviDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAviDlg)
    DDX_Control(pDX, IDC_TIME_END, m_Time_end);
    DDX_Control(pDX, IDC_TIME_START, m_Time_start);
    DDX_Text(pDX, IDC_AVINAME, m_FileName);
    DDX_Check(pDX, IDC_CHECK1, m_MinMaxMode);
    DDX_Text(pDX, IDC_DELAY, m_Delay);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAviDlg, CDialog)
    //{{AFX_MSG_MAP(CAviDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAviDlg message handlers

BOOL CAviDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    int i;
    CString ttt;

    for (i = 0; i < pGraph->pProject->m_nTimePoints; i++)
    {
        ttt.Format("%g", pGraph->pProject->m_time[i]);
        m_Time_start.AddString(ttt);
        m_Time_end.AddString(ttt);
    }
    m_Time_start.SetCurSel(0);
    m_Time_end.SetCurSel(pGraph->pProject->m_nTimePoints - 1);


    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}


void CAviDlg::OnOK()
{
    // TODO: Add extra validation here
    start_frame = m_Time_start.GetCurSel();
    frames = m_Time_end.GetCurSel() - start_frame;
    CDialog::OnOK();
}
