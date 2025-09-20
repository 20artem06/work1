
// video.h : main header file for the VIDEO application
//
/////////////////////////////////////////////////////////////////////////////

#ifndef VIDEO_H
#define VIDEO_H

#include <vector>

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#define MAXLAYERS 20
/////////////////////////////////////////////////////////////////////////////

// New types 
/////////////////////////////////////////////////////////////////////////////

// Style of drawing
const int iStyle[5] = { PS_SOLID,PS_DASH,PS_DASHDOT,PS_DASHDOTDOT,PS_DOT };

// Max memory size in bytes
const __int64 MemMax = 4000000;

// Max table size in dialog
const int TabMax = 32735;

// Axe
struct CAxe
{
    CString name, size;
    int	nPoints;
    float* point;

    ~CAxe();
};

// Field
class CField
{
    friend class CProject;
    friend class CMainFrame;

public:
    CString name, size;
    int    nAxis, nPoints;
    CAxe* axe;
    float   min, max;
private:
    float* point;

public:
    ~CField();
};

//Project
class CProject
{
public:
    CString m_sName;

    int		m_iHandle;
    BOOL    m_bInMemory;

    CString m_sTime, m_sTimeSize;
    int	m_nTimePoints;
    float* m_time;

    int	m_nFields;
    int    m_nFieldPoints;

    BOOL    m_bOneDimOnly;

    CProject* Next;
    CProject* Prev;

    CField* m_field;

    BOOL has_cel;
    BOOL flag[MAXLAYERS];
    int*** Cel;
    int X_num, Y_num, Z_num;
    double* X_scale, * Y_scale, * Z_scale;
    CString projectPath;

public:
    // Reading data for 1-dim graph
    void read_point(const int nField, const int nP0
        , const int nPst, const int nPnum
        , std::vector<float> &pFun, bool Log);

    // Reading data for 2-dim graph
    void read_points(const int nField, const __int64 nP0
        , const __int64 nPst1, const int nPn1
        , const __int64 nPst2, const int nPn2
        , std::vector<float>& pFun
        , bool Log);

    int GetLayer(double X, double Y, double Z);

    BOOL ReadGrid(CString name);
    CProject();
    ~CProject();
};

/////////////////////////////////////////////////////////////////////////////
// Utils headers

float Roundation(float x);
int   Divide(float x1, float x2);
float Scaling(int nGrids, float Fmax, float Fmin,bool grdauto, bool log, float* Step);//return fGmin - 1st grad on axe
DWORD MakeColor(float hX, int Palette, int type = 0);

void CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi,
    HBITMAP hBMP, HDC hDC);
PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp);

/////////////////////////////////////////////////////////////////////////////
// CVideoApp:
// See video.cpp for the implementation of this class
//
class CVideoApp : public CWinApp
{
public:
    CVideoApp();

    // class members
public:
    CProject* First;

public:
    CString& GetRecentFile(int ind);
    void DeleteRecentFile(int ind);
    // Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CVideoApp)
public:
    virtual BOOL InitInstance();
    //}}AFX_VIRTUAL

// Implementation
    //{{AFX_MSG(CVideoApp)
    afx_msg void OnAppAbout();
    afx_msg void OnOpenMRUFile(UINT nID);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif // !VIDEO_H
