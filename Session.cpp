

#include "Session.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#define _VERSION "video_session_0.1"

std::istream& operator >> (std::istream& stream, VideoSaveDto& data) {
	json root;
	std::vector<std::string> projects;
	std::vector<VideoSession2Dim> sessions2dim;
	std::vector<OneDimWindowDto> sessions1dim;


	stream >> root;

	std::string version = root.value("version", "");
	if (version != _VERSION) {
		throw std::exception("Unvalid version of json");
	}

	for (auto& p : root["projects"]){
		projects.push_back(p.get<std::string>());
	}

	for (auto p : root["two_dim"]) {
		VideoSession2Dim tmp;
				
		tmp.m_nField = p.value("m_nField", 0);
		tmp.m_fFmin = p.value("m_fFmin", 0.) ;
		tmp.m_fFmax = p.value("m_fFmax", 0.) ;
		tmp.m_bPolar = p.value("m_bPolar", 0);
		tmp.m_bLog = p.value("m_bLog", 0)	 ;
		tmp.m_nArg1 = p.value("m_nArg1", 0)  ;
		tmp.m_nX1min = p.value("m_nX1min", 0);
		tmp.m_nX1max = p.value("m_nX1max", 0);
		tmp.m_nArg2 = p.value("m_nArg2", 0)  ;
		tmp.m_nX2min = p.value("m_nX2min", 0);
		tmp.m_nX2max = p.value("m_nX2max", 0);
		tmp.m_nGrids = p.value("m_nGrids", 0);
		tmp.m_bGridLines = p.value("m_bGridLines", 0);
		tmp.m_bGridAuto = p.value("m_bGridAuto", 0)	 ;
		tmp.m_Palette = p.value("m_Palette", 0)		 ;
		tmp.m_layers = p.value("m_layers", 0)		 ;
		tmp.m_cloud = p.value("m_cloud", 0)			 ;
		tmp.m_lines = p.value("m_lines", 0)			 ;
		tmp.projectIndex = p.value("projectIndex", -1);

		
		for (auto&i : p["index"]) {
			tmp.INDEX.push_back(i.get<int>());
		}
		
		sessions2dim.push_back(tmp);
	}


	for (auto p : root["one_dim"]){
		OneDimWindowDto win;

		for (auto w : p["values"]) {

			VideoSession1Dim d;
			d.Style = w.value("Style", 0);
			d.bGridL = w.value("bGridL", 0);
			d.bLog = w.value("bLog", 0);
			d.bPolar = w.value("bPolar", 0);
			d.fFmax = w.value("fFmax", 0.);
			d.fFmin = w.value("fFmin", 0.);
			d.nArg1 = w.value("nArg1", 0);
			d.nField = w.value("nField", 0);
			d.nX1max = w.value("nX1max", 0);
			d.nX1min = w.value("nX1min", 0);
			d.sName = w.value("sName", "");
			d.projectIndex = w.value("projectIndex", -1);

			if (w.contains("color")) {
				d.Color = RGB(
					w["color"].value("r", 0),
					w["color"].value("g", 0),
					w["color"].value("b", 255)
				);
			}
			else {
				d.Color = RGB(0, 0, 255);
			}
			

			for (auto& i : w["index"]) {
				d.INDEX.push_back(i.get<int>());
			}


			win.graphs.push_back(d);
		}

		win.fX1min = p["meta"].value("fX1min", 0.);
		win.fX1max = p["meta"].value("fX1max", 0.);
		win.fX2min = p["meta"].value("fX2min", 0.);
		win.fX2max = p["meta"].value("fX2max", 0.);
		win.AxeStatus = p["meta"].value("AxeStatus", 2);

		sessions1dim.push_back(win);
	}

	data.projects = projects;
	data.twoDimGraphs = sessions2dim;
	data.oneDimGraphs = sessions1dim;
	return stream;
}


std::ostream& operator << (std::ostream& stream, const VideoSaveDto& data) {

	json root;

	auto projects = json::array();
	for (auto& p : data.projects) {
		projects.push_back(p);
	}

	auto twoDim = json::array();
	for (auto& p : data.twoDimGraphs) {
		json tmp;
		auto index = json::array();


		tmp["m_nField"] = p.m_nField;
		tmp["m_fFmin"] = p.m_fFmin;
		tmp["m_fFmax"] = p.m_fFmax;
		tmp["m_bPolar"] = p.m_bPolar;
		tmp["m_bLog"] = p.m_bLog;
		tmp["m_nArg1"] = p.m_nArg1;
		tmp["m_nX1min"] = p.m_nX1min;
		tmp["m_nX1max"] = p.m_nX1max;
		tmp["m_nArg2"] = p.m_nArg2;
		tmp["m_nX2min"] = p.m_nX2min;
		tmp["m_nX2max"] = p.m_nX2max;
		tmp["m_nGrids"] = p.m_nGrids;
		tmp["m_bGridLines"] = p.m_bGridLines;
		tmp["m_bGridAuto"] = p.m_bGridAuto;
		tmp["m_Palette"] = p.m_Palette;
		
		tmp["m_layers"] = p.m_layers;
		tmp["m_cloud"] = p.m_cloud;
		tmp["m_lines"] = p.m_lines;
		tmp["projectIndex"] = p.projectIndex;

		for (auto& i : p.INDEX) {
			index.push_back(i);
		}
		tmp["index"] = index;

		twoDim.push_back(tmp);
	}

	auto oneDim = json::array();
	for (auto& w : data.oneDimGraphs) {
		json d;
		auto winArray = json::array();

		for (auto& p : w.graphs) {
			json tmp;
			auto index = json::array();
			
			tmp["nField"] = p.nField;
			tmp["bPolar"] = p.bPolar;
			tmp["fFmin"] = p.fFmin;
			tmp["fFmax"] = p.fFmax;
			tmp["nArg1"] = p.nArg1;
			tmp["nX1min"] = p.nX1min;
			tmp["nX1max"] = p.nX1max;
			tmp["Style"] = p.Style;
			tmp["bGridL"] = p.bGridL;
			tmp["bLog"] = p.bLog;
			tmp["sName"] = p.sName;
			tmp["sName"] = p.sName;

			tmp["projectIndex"] = p.projectIndex;

			json color;
			color["r"] = GetRValue(p.Color);
			color["g"] = GetGValue(p.Color);
			color["b"] = GetBValue(p.Color);

			tmp["color"] = color;
			
			for (auto& i : p.INDEX) {
				index.push_back(i);
			}
			tmp["index"] = index;

			winArray.push_back(tmp);
		}
		
		d["values"] = winArray;
		d["meta"]["fX1min"] = w.fX1min;
		d["meta"]["fX1max"] = w.fX1max;
		d["meta"]["fX2min"] = w.fX2min;
		d["meta"]["fX2max"] = w.fX2max;
		d["meta"]["AxeStatus"] = w.AxeStatus;
		oneDim.push_back(d);
	}

	root["projects"] = projects;
	root["two_dim"] = twoDim;
	root["one_dim"] = oneDim;

	root["version"] = _VERSION;

	stream << root.dump(4);
	return stream;
}
