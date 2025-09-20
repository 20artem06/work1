/////////////////////////////////////////////////////////////////////////////
// video.cpp : Defines the class behaviors for the application.
//				 
#include "stdafx.h"
#include "afxpriv.h"
#include  <math.h>

#include "video.h"
#include "MainFrm.h"

#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#include <io.h>
#include <fcntl.h>

/////////////////////////////////////////////////////////////////////////////
//Utils

/////////////////////////////////////////////////////////////////////////////
//Roundation value by 1-2 digits
DWORD MakeColor(float hX, int Palette, int type /*=0*/)
{
    DWORD value;
    float red, green, blue;


    if (hX > 1) hX = 1;
    if (hX < 0) hX = 0;

    if (Palette == 1)
    {
        if (hX > 0.5)  red = 1; else red = hX * 2;
        if (hX > 0.5) blue = 0; else blue = 2 * (0.5 - hX);
        //blue = 0.999-hX;
        //red  = 0.001+hX;
        green = 1.5 * (hX) * (1 - hX) / 0.5;
    }
    if (Palette == 0)
    {
        red = hX;
        blue = 1 - hX;
        green = 0;


    }
    if (Palette == 2)
    {
        green = red = blue = 1 - hX;
    }



    if (type == 1) return RGB(255 * red, 255 * green, 255 * blue);
    value = 256 * 256 * (int)(255 * red) + 256 * (int)(255 * green) + (int)(255 * blue);

    return value;
}

float Roundation(float x)
{
    int i;
    float tmpX, tmp1;

    tmp1 = (float)1.e-37;
    tmpX = (float)fabs(x);
    for (i = -37; i < 38; i++)
    {
        tmp1 *= 10;	if (tmp1 > tmpX) break;
    }
    tmp1 /= 10;
    i = Divide(tmpX, tmp1);

    if (i >= 5) return(tmp1);
    else     return(tmp1 / 10);

}

float Scaling(int nGrids, float Fmax, float Fmin, bool grdauto, bool log ,float* Step)//return fGmin - 1st grad on axe
{
    float step = (Fmax - Fmin) / nGrids; // step between grids on axe
    int istep;
    int i;
    float fGmin;
    if (step == 0)
    {
        return 0;
    }
    float tmp1 = (float)1.e-37;
    float tmpX = step;
    for (i = -37; i < 38; i++)
    {
        tmp1 *= 10;	if (tmp1 > tmpX) break;
    }
    istep = (int)(step * 100 / tmp1);
    if ((istep - (step * 100 / tmp1)) < 0) istep++;
    (*Step) = step = istep * tmp1 / 100;
    if (grdauto)
    {
        if (log)
        {
            (*Step) = step = int(step);
        }
        else
        {
            double tmpa = 1e-37;
            for (i = -37; i < 38; i++)
            {
                tmpa *= 10;	if (tmpa > step) break;
            }
            (*Step) = step = tmpa / 10;
        }
    }
    fGmin = Fmin - (step * nGrids - (Fmax - Fmin)) / 2;

    tmp1 = (float)1.e-37;
    tmpX = (float)fabs(fGmin);
    int sgn = (fGmin * tmpX >= 0) ? 1 : -1;
    for (i = -37; i < 38; i++)
    {
        tmp1 *= 10;	if (tmp1 > tmpX) break;
    }
    fGmin = sgn * (1 + ((int)(tmpX * 10 / tmp1))) * tmp1 / 10;
    while (Fmin < fGmin) fGmin -= tmp1 / 10;
    return fGmin;


}

/////////////////////////////////////////////////////////////////////////////
// (int) (x1/x2) <= x1/x2
int Divide(float x1, float x2)
{
    float x;
    x = x1 / x2;
    if (x >= 0) return((int)x);
    else     return((int)x - 1);
}

/////////////////////////////////////////////////////////////////////////////
// New types contructors/destructors

/////////////////////////////////////////////////////////////////////////////
// CAxe
CAxe::~CAxe()
{
    if (point) delete[] point;
}

/////////////////////////////////////////////////////////////////////////////
// CField
CField::~CField()
{
    if (point) delete[] point;
    if (axe)   delete[] axe;
}

/////////////////////////////////////////////////////////////////////////////
// CProject
CProject::CProject()
{
    m_bInMemory = TRUE;

    m_field = NULL;
    m_time = NULL;

    m_bOneDimOnly = TRUE;
    m_nFieldPoints = 0;
    m_nFields = 0;
    m_nTimePoints = 0;

    Next = NULL;
    Prev = NULL;

    projectPath = "";
}

CProject::~CProject()
{
    if (!m_bInMemory) _close(m_iHandle);
    if (m_field) delete[] m_field;
    if (m_time)  delete[] m_time;
}

/////////////////////////////////////////////////////////////////////////////
// Reading data for 1-dim graph
void CProject::read_point(const int nField  // field number
    , const int nP0                         // initial point
    , const int nPst                        // step
    , const int nPnum                       // number of points
    , std::vector<float>& pFun             // output data vector
    , bool Log)             // Log or not
{
    if (m_bInMemory) {      // data in memory
        int l = nP0;
        for (int k = 0; k < nPnum; k++) {
            pFun[k] = m_field[nField].point[l];
            if (Log) 
            {
                if (pFun[k] > 0)
                    pFun[k] = log10(pFun[k]);
                else
                    pFun[k] = 0;
            }

            l += nPst;
        }
    } else {                // data in file
        __int64 l = nP0 / m_field[nField].nPoints;
        __int64 k = nP0 - m_field[nField].nPoints * l;

        // First point search
        l = l * m_nFieldPoints + 1 + k;
        for (int i = 0; i < nField; i++) {
            l += m_field[i].nPoints;
        }

        _lseeki64(m_iHandle, l * sizeof(float), SEEK_SET);

        // Step calculation
        if (nPst == m_field[nField].nPoints) {  //f(t)
            k = m_nFieldPoints - 1;
        } else {                                //f(x)
            k = nPst - 1;
        }

        // Prepare progress indicator window
        CRect Rec; 	CWnd* pMainFrame = AfxGetMainWnd();
        pMainFrame->GetClientRect(Rec);
        Rec.top += Rec.Height() * 1 / 10;
        Rec.bottom = Rec.top + 50;
        int m = Rec.Width();
        Rec.left += m / 3;
        Rec.right -= m / 3;
        CProgressCtrl pProg;

        pProg.Create(WS_CHILD | WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION,
            Rec, pMainFrame, ID_PROGRESS);
        pProg.SetWindowText("Reading data...");
        pProg.ShowWindow(SW_SHOW);
        int nProcentOld = 0, nProcent = 0;
        _read(m_iHandle, &pFun[0], sizeof(float));
        if (Log) 
        {
            if (pFun[0] > 0)
                pFun[0] = log10(pFun[0]);
            else
                pFun[0] = 0;
        }
        for (int i = 1; i < nPnum; i++) {
            // Reading data
            _lseeki64(m_iHandle, k * sizeof(float), SEEK_CUR);

            _read(m_iHandle, &pFun[i], sizeof(float));
            if (Log)
            {
                if (pFun[i] > 0)
                    pFun[i] = log10(pFun[i]);
                else
                    pFun[i] = 0;
            }
            // Show progress
            nProcent = 100 * i / nPnum;
            if (nProcent != nProcentOld) {
                pProg.SetPos(nProcent);
                pProg.UpdateWindow();
                nProcentOld = nProcent;
            }
        }

        pProg.DestroyWindow();
    }
}

/////////////////////////////////////////////////////////////////////////////
// Reading data for 2-dim graph
void CProject::read_points(const int nField // field number
    , const __int64 nP0                     // initual point
    , const __int64 nPst1                   // step by X1
    , const int nPn1                        // number of points by X1
    , const __int64 nPst2                   // step by X2
    , const int nPn2                        // number of points by X2
    , std::vector<float> &pFun             // output adress
    , bool Log)             // Log or not
{
    if (m_bInMemory) {  // data in memory
        __int64 l = nP0;
        size_t index = 0;
        for (int j = 0; j < nPn2; j++) {
            for (int i = 0; i < nPn1; i++) {
                pFun[index] = m_field[nField].point[l];
                if (Log)
                {
                    if (pFun[index] > 0)
                        pFun[index] = log10(pFun[index]);
                    else
                        pFun[index] = 0;
                }
                l += nPst1;
                index++;
            }
            l += nPst2;
        }
    } else {            // data in file
        __int64 l = nP0 / m_field[nField].nPoints;
        __int64 k = nP0 - m_field[nField].nPoints * l;

        // First point search
        l = l * m_nFieldPoints + 1 + k;
        for (int i = 0; i < nField; i++) {
            l += m_field[i].nPoints;
        }

        _lseeki64(m_iHandle, l * sizeof(float), SEEK_SET);

        // Steps calculation
        if (nPst1 == m_field[nField].nPoints) {						//f(t,y)
            k = m_nFieldPoints - 1;
            l = -(long)(nPn1 * m_nFieldPoints - nPst2 - nPn1 * m_field[nField].nPoints);
        } else {
            k = nPst1 - 1;
            if (nPst2 == (long)(m_field[nField].nPoints - nPn1 * nPst1))	//f(x,t)
                l = m_nFieldPoints - nPn1 * nPst1;
            else
                l = nPst2;											//f(x,y)
        }

        // Prepare progress indicator window
        CRect Rec; 	CWnd* pMainFrame = AfxGetMainWnd();
        pMainFrame->GetClientRect(Rec);
        Rec.top += Rec.Height() * 1 / 10;
        Rec.bottom = Rec.top + 50;
        int m = Rec.Width();
        Rec.left += m / 3;
        Rec.right -= m / 3;
        CProgressCtrl pProg;

        pProg.Create(WS_CHILD | WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION,
            Rec, pMainFrame, ID_PROGRESS);
        pProg.SetWindowText("Reading data...");
        pProg.ShowWindow(SW_SHOW);
        int nProcentOld = 0, nProcent = 0;

        size_t index = 0;
        for (int j = 0; j < nPn2; j++) {
            // Reading data
            for (int i = 0; i < nPn1; i++) {
                _read(m_iHandle, &pFun[index], sizeof(float));
                if (Log)
                {
                    if (pFun[index] > 0)
                        pFun[index] = log10(pFun[index]);
                    else
                        pFun[index] = 0;
                }
                _lseeki64(m_iHandle, k * sizeof(float), SEEK_CUR);
                index++;
            }
            _lseeki64(m_iHandle, l * sizeof(float), SEEK_CUR);

            // Show progress
            nProcent = 100 * j / nPn2;
            if (nProcent != nProcentOld) {
                pProg.SetPos(nProcent);
                pProg.UpdateWindow();
                nProcentOld = nProcent;
            }
        }

        pProg.DestroyWindow();
    }
}


/////////////////////////////////////////////////////////////////////////////
// CVideoApp

BEGIN_MESSAGE_MAP(CVideoApp, CWinApp)
    //{{AFX_MSG_MAP(CVideoApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    ON_COMMAND_RANGE(ID_FILE_MRU_FIRST, ID_FILE_MRU_LAST, OnOpenMRUFile)
    // NOTE - the ClassWizard will add and remove mapping macros here.
        //    DO NOT EDIT what you see in these blocks of generated code!
    //}}AFX_MSG_MAP
    // Standard file based document commands
    //ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVideoApp construction

CVideoApp::CVideoApp()
{
    First = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CVideoApp operations and commands

CString& CVideoApp::GetRecentFile(int ind)
{
    return (*m_pRecentFileList)[ind];
}

void CVideoApp::DeleteRecentFile(int ind)
{
    m_pRecentFileList->Remove(ind);
}

void CVideoApp::OnOpenMRUFile(UINT nID)
{
    nID -= ID_FILE_MRU_FIRST;
    CString file = ((CVideoApp*)AfxGetApp())->GetRecentFile(nID);
    CMainFrame* pWnd = (CMainFrame*)AfxGetMainWnd();
    if (!pWnd->OpenProject(file))
        ((CVideoApp*)AfxGetApp())->DeleteRecentFile(nID);
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CVideoApp object

CVideoApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CVideoApp initialization
BOOL CVideoApp::InitInstance()
{
    LoadStdProfileSettings();  // Load standard INI file options (including MRU)

    // create main MDI Frame window
    CMainFrame* pMainFrame = new CMainFrame;
    if (!(pMainFrame->Create()))return FALSE;
    m_pMainWnd = pMainFrame;


    /*
        // Enable drag/drop open
        m_pMainWnd->DragAcceptFiles();

        // Enable DDE Execute open
        EnableShellOpen();
    */
    // Parse command line for standard shell commands, DDE, file open
    CCommandLineInfo cmdInfo;
    //	ParseCommandLine(cmdInfo);

        // Dispatch commands specified on the command line
    //	if (!ProcessShellCommand(cmdInfo))
    //		return FALSE;

        // The main window has been initialized, so show and update it.
    pMainFrame->ShowWindow(SW_SHOWMAXIMIZED);
    pMainFrame->UpdateWindow();

    //////////////////////////////Command line//////////////////////////////
    if (m_lpCmdLine[0] != '\0') pMainFrame->OpenProject(m_lpCmdLine);
    ////////////////////////////////////////////////////////////////////////

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

    // Dialog Data
        //{{AFX_DATA(CAboutDlg)
    enum { IDD = IDD_ABOUTBOX };
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CAboutDlg)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    //{{AFX_MSG(CAboutDlg)
        // No message handlers
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedLog();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
    //{{AFX_DATA_INIT(CAboutDlg)
    //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAboutDlg)
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    //{{AFX_MSG_MAP(CAboutDlg)
        // No message handlers
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_LOG, &CAboutDlg::OnBnClickedLog)
END_MESSAGE_MAP()

// App command to run the dialog
void CVideoApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

BOOL CProject::ReadGrid(CString name)
{
    int k;
    std::fstream file;
    char szBuffer[100];
    CString buf;


    X_num = Y_num = Z_num = 0;
    X_scale = Y_scale = Z_scale = NULL;
    has_cel = TRUE;

    file.open(name, std::ios::in | std::ios::_Nocreate);
    if (file.fail())
    {
        //BadData(0);
        //AfxMessageBox("Не удалось открыть файл.");
        has_cel = FALSE;
        return 0;
    }

    //Считать количество ячеек по X, Y, Z и их координаты
    file.getline(szBuffer, 255); //EQUAL
    file.getline(szBuffer, 255);
    file.getline(szBuffer, 255); //DIM
    file.getline(szBuffer, 255);


    file >> szBuffer;	//Skip "X"
    buf = szBuffer;
    if ((buf != "X") && (buf != "x"))
    {
        //	BadData(1);
        AfxMessageBox("Неверный формат файла.");
        has_cel = FALSE;
        return 0;
    };
    file >> X_num;
    if ((X_num) <= 0)
    {
        //	BadData(-11);
        AfxMessageBox("Неверный формат файла.");
        has_cel = FALSE;
        return 0;
    };
    X_scale = new double[X_num + 1];
    for (k = 0; k < X_num + 1; k++)
    {
        file >> X_scale[k];// Read koordinates 
        if (k != 0) if (X_scale[k] <= X_scale[k - 1]) // Если координаты записаны не по возрастанию
        {
            //			BadData(4);
            AfxMessageBox("Неверный формат файла.");
            has_cel = FALSE;
            return 0;
        };
    };

    file >> szBuffer;	//Skip "Y"
    buf = szBuffer;
    if ((buf != "Y") && (buf != "y"))
    {
        //	BadData(2);
        AfxMessageBox("Неверный формат файла.");
        has_cel = FALSE;
        return 0;
    };
    file >> Y_num;
    if (Y_num <= 0)
    {
        //	BadData(-12);
        AfxMessageBox("Неверный формат файла.");
        has_cel = FALSE;
        return 0;
    };
    Y_scale = new double[Y_num + 1];
    for (k = 0; k < Y_num + 1; k++)
    {
        file >> Y_scale[k];// Read koordinates 
        if (k != 0) if (Y_scale[k] <= Y_scale[k - 1]) // Если координаты записаны не по возрастанию
        {
            //	BadData(5);
            AfxMessageBox("Неверный формат файла.");
            has_cel = FALSE;
            return 0;
        };
    };

    file >> szBuffer;	//Skip "Z"
    buf = szBuffer;
    if ((buf != "Z") && (buf != "z"))
    {
        //	BadData(3);
        AfxMessageBox("Неверный формат файла.");
        has_cel = FALSE;
        return 0;
    };
    file >> Z_num;
    if (Z_num <= 0)
    {
        //	BadData(-13);
        AfxMessageBox("Неверный формат файла.");
        has_cel = FALSE;
        return 0;
    };
    Z_scale = new double[Z_num + 1];
    for (k = 0; k < Z_num + 1; k++)
    {
        file >> Z_scale[k];// Read koordinates 
        if (k != 0) if (Z_scale[k] <= Z_scale[k - 1]) // Если координаты записаны не по возрастанию
        {
            //			BadData(6);
            AfxMessageBox("Неверный формат файла.");
            has_cel = FALSE;
            return 0;
        };
    };
    file.close();

    int i, j, l, m;

    Cel = new int** [X_num];                    //prepared place
    for (i = 0; i < X_num; i++)
    {
        Cel[i] = new int* [Y_num];
        for (j = 0; j < Y_num; j++) Cel[i][j] = new int[Z_num];
    };

    m = 0;

    file.open("data.cel", std::ios::in | std::ios::_Nocreate);
    if (file.fail())
    {
        AfxMessageBox("Не удалось открыть файл проекта ");
        has_cel = FALSE;
        return FALSE;
    }
    for (i = 0; i < X_num; i++)
        for (j = 0; j < Y_num; j++)
            for (k = 0; k < Z_num; k++)
            {
                while (m <= 0)
                {
                    file >> l;
                    file >> m;
                    flag[l] = TRUE;
                    if (file.fail())
                    {
                        AfxMessageBox("Не удалось открыть файл проекта ");
                        has_cel = FALSE;
                        return FALSE;
                    };
                }
                Cel[i][j][k] = l;

                m--;
            }
    file.close();


    return 1;
}

int CProject::GetLayer(double X, double  Y, double  Z)
{
    if (!has_cel) return 0;

    if (X > X_scale[X_num - 1]) return 0;
    if (X < X_scale[0])		  return 0;
    if (Y > Y_scale[Y_num - 1]) return 0;
    if (Y < Y_scale[0])		  return 0;
    if (Z > Z_scale[Z_num - 1]) return 0;
    if (Z < Z_scale[0])		  return 0;

    int n_max, n_min, n_middle;
    int nX, nY, nZ;

    n_max = X_num - 1;
    n_min = 0;
    do
    {
        n_middle = n_min + 0.5 * (n_max - n_min);
        if (X_scale[n_middle] > X) n_max = n_middle;
        else n_min = n_middle;
    } while ((n_max - n_min) > 1);

    nX = n_min;

    n_max = Y_num - 1;
    n_min = 0;
    do
    {
        n_middle = n_min + 0.5 * (n_max - n_min);
        if (Y_scale[n_middle] > Y) n_max = n_middle;
        else n_min = n_middle;
    } while ((n_max - n_min) > 1);
    nY = n_min;

    n_max = Z_num - 1;
    n_min = 0;
    do
    {
        n_middle = n_min + 0.5 * (n_max - n_min);
        if (Z_scale[n_middle] > Z) n_max = n_middle;
        else n_min = n_middle;
    } while ((n_max - n_min) > 1);
    nZ = n_min;
    //if (Cel[nX][nY][nZ]>14) return 0;
    return Cel[nX][nY][nZ];

}


PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)
{
    BITMAP bmp;
    PBITMAPINFO pbmi;
    WORD    cClrBits;

    // Retrieve the bitmap color format, width, and height. 
    GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp);


    // Convert the color format to a count of bits. 
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
    if (cClrBits == 1)
        cClrBits = 1;
    else if (cClrBits <= 4)
        cClrBits = 4;
    else if (cClrBits <= 8)
        cClrBits = 8;
    else if (cClrBits <= 16)
        cClrBits = 16;
    else if (cClrBits <= 24)
        cClrBits = 24;
    else cClrBits = 32;

    // Allocate memory for the BITMAPINFO structure. (This structure 
    // contains a BITMAPINFOHEADER structure and an array of RGBQUAD 
    // data structures.) 

    if (cClrBits != 24)
        pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
            sizeof(BITMAPINFOHEADER) +
            sizeof(RGBQUAD) * (1 << cClrBits));

    // There is no RGBQUAD array for the 24-bit-per-pixel format. 

    else
        pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
            sizeof(BITMAPINFOHEADER));

    // Initialize the fields in the BITMAPINFO structure. 

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = bmp.bmWidth;
    pbmi->bmiHeader.biHeight = bmp.bmHeight;
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
    if (cClrBits < 24)
        pbmi->bmiHeader.biClrUsed = (1 << cClrBits);

    // If the bitmap is not compressed, set the BI_RGB flag. 
    pbmi->bmiHeader.biCompression = BI_RGB;

    // Compute the number of bytes in the array of color 
    // indices and store the result in biSizeImage. 
    // For Windows NT, the width must be DWORD aligned unless 
    // the bitmap is RLE compressed. This example shows this. 
    // For Windows 95/98/Me, the width must be WORD aligned unless the 
    // bitmap is RLE compressed.
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8
        * pbmi->bmiHeader.biHeight;
    // Set biClrImportant to 0, indicating that all of the 
    // device colors are important. 
    pbmi->bmiHeader.biClrImportant = 0;
    return pbmi;
}



void CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi,
    HBITMAP hBMP, HDC hDC)
{
    HANDLE hf;                 // file handle 
    BITMAPFILEHEADER hdr;       // bitmap file-header 
    PBITMAPINFOHEADER pbih;     // bitmap info-header 
    LPBYTE lpBits;              // memory pointer 
    DWORD dwTotal;              // total count of bytes 
    DWORD cb;                   // incremental count of bytes 
    BYTE* hp;                   // byte pointer 
    DWORD dwTmp;

    pbih = (PBITMAPINFOHEADER)pbi;
    lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

    /*  if (!lpBits)
             errhandler("GlobalAlloc", hwnd);
    */
    // Retrieve the color table (RGBQUAD array) and the bits 
    // (array of palette indices) from the DIB. 
    if (!GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, lpBits, pbi,
        DIB_RGB_COLORS))
    {
        //    errhandler("GetDIBits", hwnd); 
    }

    // Create the .BMP file. 


    hf = CreateFile(pszFile,
        GENERIC_READ | GENERIC_WRITE,
        (DWORD)0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        (HANDLE)NULL);
    //  if (hf == INVALID_HANDLE_VALUE) 
    //      errhandler("CreateFile", hwnd); 
    hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M" 
    // Compute the size of the entire file. 
    hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) +
        pbih->biSize + pbih->biClrUsed
        * sizeof(RGBQUAD) + pbih->biSizeImage);
    hdr.bfReserved1 = 0;
    hdr.bfReserved2 = 0;

    // Compute the offset to the array of color indices. 
    hdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) +
        pbih->biSize + pbih->biClrUsed
        * sizeof(RGBQUAD);

    // Copy the BITMAPFILEHEADER into the .BMP file. 
    if (!WriteFile(hf, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER),
        (LPDWORD)&dwTmp, NULL))
    {
        //     errhandler("WriteFile", hwnd); 
    }

    // Copy the BITMAPINFOHEADER and RGBQUAD array into the file. 
    (!WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER)
        + pbih->biClrUsed * sizeof(RGBQUAD),
        (LPDWORD)&dwTmp, (NULL)));
    //   errhandler("WriteFile", hwnd); 

     // Copy the array of color indices into the .BMP file. 
    dwTotal = cb = pbih->biSizeImage;
    hp = lpBits;
    (!WriteFile(hf, (LPSTR)hp, (int)cb, (LPDWORD)&dwTmp, NULL));
    //         errhandler("WriteFile", hwnd); 

        // Close the .BMP file. 
    (!CloseHandle(hf));
    //         errhandler("CloseHandle", hwnd); 

        // Free memory. 
    GlobalFree((HGLOBAL)lpBits);
}

void CAboutDlg::OnBnClickedLog()
{
    
    // TODO: добавьте свой код обработчика уведомлений
}


