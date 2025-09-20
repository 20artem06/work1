
#ifndef SESSION_H
#define SESSION_H

#include "stdafx.h"
#include <string>
#include <vector>
#include <fstream>


#include "Video.h"


struct VideoSession1Dim {

	CProject* pProject;

	int nField;
	int bPolar;
	float fFmin;
	float fFmax;
	int nArg1;
	int nX1min;
	int nX1max;
	COLORREF Color;
	int     Style;
	int     bGridL;
	int     bLog;
	std::string sName;

	std::vector<int> INDEX;

	int projectIndex;

};


struct VideoSession2Dim {
	CProject* pProject;
	
	int m_nField;
	float m_fFmin;
	float m_fFmax;
	int m_bPolar;
	int m_bLog;
	int m_nArg1;
	int m_nX1min;
	int m_nX1max;
	int m_nArg2;
	int m_nX2min;
	int m_nX2max;
	int m_nGrids;
	int m_bGridLines;
	int m_bGridAuto;
	int m_Palette;
	
	COLORREF ColorMin = RGB(0, 0, 255);
	COLORREF ColorMax = RGB(255, 0, 0);
	int m_layers;
	int m_cloud;
	int m_lines;
	std::vector<int> INDEX;

	int projectIndex;
};

struct OneDimWindowDto {
	std::vector<VideoSession1Dim> graphs;

	float fX1min;
	float fX1max;
	float fX2min;
	float fX2max;
	int AxeStatus;
};


struct VideoSaveDto {
	std::vector<std::string> projects;
	std::vector<VideoSession2Dim> twoDimGraphs;
	std::vector<OneDimWindowDto> oneDimGraphs;

};


std::istream& operator >> (std::istream& stream, VideoSaveDto &data);
std::ostream& operator << (std::ostream& stream, const VideoSaveDto &data);


#endif