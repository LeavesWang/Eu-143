#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdio>
#include <cstring>
using namespace std;
#include "TApplication.h"
#include "TF1.h"
#include "TGraph.h"

#include "setup.h"

struct fitInfo
{
	double calPar[2];
	double R2;
};

fitInfo GetCof(string calData, string mcaFile);

void CofFromMca()
{
	if (access("../MCA/Cal", F_OK) != 0)
		system("mkdir ../MCA/Cal");

	int min = 0, max = 0, runNum = 0;
	cout << "Please input the minium and maximum number of files:\n";
	cin >> min >> max;
	ostringstream runNumSS, iSS, jSS;
	string raw, cal, mcaFold, mcaFile, calData;
	struct fitInfo cof;
	int i = 0, j = 0, k = 0;
	for (runNum = min; runNum <= max; runNum++)
	{
		runNumSS.str("");
		runNumSS << setw(5) << setfill('0') << runNum;
		raw = "run" + runNumSS.str();
		cout << "\n**************************************************\n";
		cout << "Now processing " << raw << endl;

		mcaFold = "../MCA/mca" + runNumSS.str();

		cal = "../MCA/Cal/" + raw + ".cal";
		FILE *fCal = fopen(cal.c_str(), "w");
		fprintf(fCal, "#%s:\n", raw.c_str());
		fprintf(fCal, "\n %8s %7s %11s %12s", "ADCNum", "C0", "C1", "R2");
		fclose(fCal);
		for (i = 0; i < ADC_MODS; i++)
			for (j = 0; j < ADC_CHANS; j++)
			{
				iSS.str("");
				iSS << setw(2) << setfill('0') << i;
				jSS.str("");
				jSS << setw(2) << setfill('0') << j;

				mcaFile = mcaFold + "/" + raw + "_" + iSS.str() + "_" + jSS.str() + ".mca";
				calData = mcaFold + "/" + raw + "_" + iSS.str() + "_" + jSS.str() + "_cal.dat";
				memset(&cof, 0, sizeof(cof));
				cof.R2 = -1;
				cof = GetCof(calData, mcaFile);
				fCal = fopen(cal.c_str(), "a");
				fprintf(fCal, "\n%4d %4d", i, j);
				for (k = 0; k < 2; k++)
					fprintf(fCal, "%*.*f", (10 + k * 3), ((k + 1) * 3), cof.calPar[k]);
				fprintf(fCal, "%13.6f", cof.R2);
				fclose(fCal);
			}
	}
}

fitInfo GetCof(string calData, string mcaFile)
{
	fitInfo cofFit;
	memset(&cofFit, 0, sizeof(cofFit));
	cofFit.R2 = -1;
	bool isStart, isEnd, isCal, isOpen;
	int n, fitNum;
	size_t found;
	string s;
	ofstream fCalW(calData.c_str());
	ifstream fMca(mcaFile.c_str());
	isOpen = true;
	if (!fMca.is_open())
	{
		cout << "Error in Opening " << mcaFile << "!\n";
		isOpen = false;
	}
	if (isOpen)
	{
		TF1 *fitFunc = new TF1("fitFunc", "pol1", 0, 8192);
		isStart = false;
		isEnd = false;
		isCal = false;
		fitNum = 0;
		while (getline(fMca, s))
		{
			if ((found = s.find("LABEL")) != string::npos)
				isStart = true;
			if (isStart && (found = s.find("<<")) != string::npos)
				isEnd = true;
			if (isEnd)
				break;
			if (isStart && !isEnd && ((found = s.find("LABEL")) == string::npos))
				isCal = true;
			if (isCal)
			{
				fitNum++;
				fCalW << s << endl;
			}
		}
		fMca.close();
		fCalW.close();

		if (fitNum >= 2)
		{
			double *chan = new double[fitNum];
			double *engy = new double[fitNum];
			ifstream fCalR(calData.c_str());
			if (!fCalR.is_open())
				cout << "Error in Opening " << calData << "!\n";
			for (n = 0; n < fitNum; n++)
				fCalR >> chan[n] >> engy[n];
			fCalR.close();
			TGraph *gr = new TGraph(fitNum, chan, engy);
			gr->Fit(fitFunc, "Q");
			fitFunc->GetParameters(cofFit.calPar);
			double SStot = fitNum * pow(gr->GetRMS(2), 2);
			double SSres = fitFunc->GetChisquare();
			cofFit.R2 = 1 - SSres / SStot;
			delete gr;
			delete[] engy;
			delete[] chan;
		}
		else
			cout << "The number of fiting data in " << mcaFile << " is not enough!\n";
		delete fitFunc;
	}
	return cofFit;
}

#ifndef __CINT__
void StandaloneApplication(int argc, char **argv)
{
	CofFromMca();
}

int main(int argc, char **argv)
{
	TApplication app("ROOT Application", &argc, argv);
	StandaloneApplication(app.Argc(), app.Argv());
	// app.Run();
	return 0;
}
#endif