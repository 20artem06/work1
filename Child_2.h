
// Child_2.h : interface of the CChild_2_Frame class
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CHILD_2_H
#define CHILD_2_H

#include <vector>
#include "Session.h"

#define MAXLAYERS 20

class CChild_2_Frame : public CMDIChildWnd
{
    DECLARE_DYNCREATE(CChild_2_Frame)
public:
    CChild_2_Frame() = default;
    CChild_2_Frame(CField_2_Dlg* pDlg);     //Main constructor
    CChild_2_Frame(VideoSession2Dim* session);
    ~CChild_2_Frame() = default;

// Attributes
public:
    std::vector<int> INDEX; // Attributes from CField_2_Dlg dialog
    COLORREF ColorMin, ColorMax;
    COLORREF LAYER_COL[MAXLAYERS];

    float   fFmin, fFmax;
    float   frealmax, frealmin;
    int     nX1min, nX1max, nX2min, nX2max;
    BOOL    bPolar, bGridL, lines, cloud, layers, bLog, bGridAuto;
    int     nField, nArg1, nArg2, nGrids;
    CProject* pProject;

    std::vector<float> Fun;                 //Function values
    CString sFName;                         //Function name
    std::vector<float> X1;                  //X1 values
    CString sX1Name;                        //X1-axe name
    std::vector<float> X2;                  //X2 values
    CString sX2Name;                        //X2-axe name
    int nX1, nX2;                           //number of points
    CString sName;                          //Window name
    CString sDesc;                          //Graph description
    float fX1min, fX1max, fX2min, fX2max;   //Axes MIN/MAX values 
    float fGmin, fGstep;                    //1-st Grid and step

    HGLRC m_hrc;

    int PicsTotal;
    int Palette;

    double Values[6];
    int CEL_ind[4];

    CString MainDir;

private:
    HGLRC hGLRC;
    CClientDC* pdc;

private:
    BOOL SetPixelFormat(HDC hdc);

    // Operations
public:
    void Paint(CDC* pDc);
    void PrepareData_time(BOOL MinMax);
    void PrepareData_max();
    VideoSession2Dim GetSession();

public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    //}}AFX_VIRTUAL

// Implementation
public:
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    // Generated message map functions
protected:
    void InitLayers();
    void PrepareData();
    void SetDataFromDto(CField_2_Dlg* pDlg);
    void SetDataFromDto(VideoSession2Dim* s);
    //{{AFX_MSG(CChild_2_Frame)
    afx_msg void OnGraphModify();
    afx_msg void OnPaint();
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnFileSavedata();
    afx_msg void OnFileSavepict();
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnSaveWmf();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnPhotoFilm();
    afx_msg void OnResize();
    afx_msg void OnTempmax();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif // !CHILD_2_H
