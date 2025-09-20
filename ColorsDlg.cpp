// ColorsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "video.h"
#include "ColorsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorsDlg dialog


CColorsDlg::CColorsDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CColorsDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CColorsDlg)
    m_val = 0.0;
    //}}AFX_DATA_INIT
}


void CColorsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CColorsDlg)
    DDX_Control(pDX, IDC_COLOR, m_ColorBox);
    DDX_Text(pDX, IDC_VAL, m_val);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CColorsDlg, CDialog)
    //{{AFX_MSG_MAP(CColorsDlg)
    ON_BN_CLICKED(IDC_ADD, OnAdd)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorsDlg message handlers

void CColorsDlg::OnAdd()
{
    UpdateData();
    //	int i;
    //	if (m_val>(1-PREC))  refcolor[pointnum]=m_color;
    //	for (i=0; i<pointnum; i++)	if 

}
