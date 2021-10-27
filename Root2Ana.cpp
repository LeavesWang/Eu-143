#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>
using namespace std;
#include "TApplication.h"
#include "TTree.h"
#include "TFile.h"
#include "TRandom3.h"

#include "setup.h"

struct StrLaBrSi
{
	double ESi;
	long long DtsLaBrSi;
	double ELaBr[2];
	double DtLaBr;
	int numLaBr;
};

struct StrGeSi
{
	double ESi;
	long long DtsGeSi;
	double EGe;
};

void Root2Ana()
{
	const unsigned short LADC = 100, UADC = 7600;
	const unsigned int LTDC = 100000;
	const long long DtsLaBrSiL = -100, DtsLaBrSiU = 4000, DtsGeSiL = -200, DtsGeSiU = 4000;
	/*const double LaBrTimeDiff[4][4] = {
											{0   , -19.5, -15.5,  -10.2},
											{19.5,  0   ,  4.0 ,   9.3},
											{15.5, -4.0 ,  0   ,   5.3},
											{10.2, -9.3 ,  -5.3,   0}
										};*/
	const double LaBrTimeDiff[4][4] = {
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
	};
	double calLaBr[4][2] = {{-11.13, 0.2567}, {-8.231, 0.272221}, {-4.852, 0.229072}, {-6.048, 0.227693}};
	double calGe[2] = {-4.775, 0.190693};
	double calSi[2] = {-75.5, 4.6};
	struct StrBranch Data;

	struct StrLaBrSi LaBrSi;
	struct StrGeSi GeSi;

	unsigned long long tsSi, tsLaBr, tsGe;
	unsigned short aLaBr = 0, bLaBr = 0;
	double c0, c1;
	int k = 0;

	int min, max, runNum;
	ostringstream runNumSS, minSS, maxSS;
	string raw, rootFile, ana, cal, s;

	cout << "Please input the minium and maximum number of files: " << endl;
	cin >> min >> max;

	minSS.str("");
	minSS << min;
	maxSS.str("");
	maxSS << max;

	ana = "ana_" + minSS.str() + "_" + maxSS.str() + ".root";
	TFile *fAna = new TFile(ana.c_str(), "RECREATE");
	TTree *tLaBrSi = new TTree("tLaBrSi", "tree for LaBr3-Si");
	TTree *tGeSi = new TTree("tGeSi", "tree for HPGe-Si");

	tLaBrSi->Branch("LaBrSi", &LaBrSi, "ESi/D:DtsLaBrSi/L:ELaBr[2]/D:DtLaBr/D:numLaBr/I");
	tGeSi->Branch("GeSi", &GeSi, "ESi/D:DtsGeSi/L:EGe/D");

	bool isStart;
	size_t found;
	int adcMd, adcCh;

	unsigned long long i, j, n;
	int p;

	for (runNum = min; runNum <= max; runNum++)
	{
		runNumSS.str("");
		runNumSS << setw(5) << setfill('0') << runNum;
		raw = "run" + runNumSS.str();
		rootFile = raw + "_final.root";
		cout << "\n**************************************************\n";
		cout << "Now processing " << rootFile << "!\n";

		TFile *fRaw = new TFile(rootFile.c_str());
		if (fRaw->IsZombie())
		{
			cout << "Error open the file!" << endl;
			continue;
		}
		TTree *tRaw = (TTree *)fRaw->Get("Trigger");
		if (!tRaw)
		{
			cout << "Error read the tree of Trigger!\n";
			continue;
		}
		memset(&Data, 0, sizeof(Data));
		tRaw->SetBranchAddress("Data", &Data);

		cal = "./mca/cal/" + raw + ".cal";
		ifstream fCal(cal.c_str());
		if (!fCal.is_open())
		{
			cout << "There is no calibrated file for " + raw + "!\n";
			continue;
		}
		isStart = false;
		while (getline(fCal, s))
		{
			if ((found = s.find("ADCNum")) != string::npos)
				isStart = true;
			if (isStart && ((found = s.find("ADCNum")) == string::npos))
			{
				istringstream iss(s);
				iss >> adcMd >> adcCh >> c0 >> c1;
				if (adcMd == 0 && adcCh < 4)
				{
					calLaBr[adcCh][0] = c0;
					calLaBr[adcCh][1] = c1;
				}
				if (adcMd == 1 && adcCh == 0)
				{
					calSi[0] = c0;
					calSi[1] = c1;
				}
				if (adcMd == 2 && adcCh == 0)
				{
					calGe[0] = c0;
					calGe[1] = c1;
				}
			}
		}
		fCal.close();
		for (k = 0; k < 4; k++)
			printf("Calibration parameters for LaBr3: c[%d][0]=%f, c[%d][1]=%f;\n", k, calLaBr[k][0], k, calLaBr[k][1]);
		cout << "Calibration parameters for Si: " << calSi[0] << ", " << calSi[1] << "; " << endl;
		cout << "Calibration parameters for HPGe: " << calGe[0] << ", " << calGe[1] << ". " << endl;

		n = tRaw->GetEntries();
		p = 0;
		for (i = 0; i < n; i++)
		{
			if ((100.0 * i / n) >= p)
			{
				cout << "Please be patient, I'm working ...... " << p << "% has been completed!" << endl;
				p += 10;
			}

			tRaw->GetEntry(i);

			memset(&LaBrSi, 0, sizeof(LaBrSi));
			memset(&GeSi, 0, sizeof(GeSi));

			if (Data.adc[1][0] > LADC && Data.adc[1][0] < UADC && Data.adc[1][2] < LADC) // adc[1][2] is for veto-Si
			{
				tsSi = Data.timestamp[2];
				TRandom3 r(0);
				LaBrSi.ESi = calSi[0] + calSi[1] * (Data.adc[1][0] + r.Uniform(-0.5, 0.5));
				GeSi.ESi = calSi[0] + calSi[1] * (Data.adc[1][0] + r.Uniform(-0.5, 0.5));

				for (j = i; j > 0; j--)
				{
					tRaw->GetEntry(j);

					tsLaBr = Data.timestamp[1];
					LaBrSi.DtsLaBrSi = (long long)(tsLaBr - tsSi);
					if (LaBrSi.DtsLaBrSi >= DtsLaBrSiL)
						for (aLaBr = 0; aLaBr < 4; aLaBr++)
							for (bLaBr = 0; bLaBr < 4; bLaBr++)
								if (Data.adc[0][aLaBr] > LADC && Data.adc[0][bLaBr] > LADC && Data.adc[0][aLaBr] < UADC && Data.adc[0][bLaBr] < UADC && Data.tdc[0][aLaBr] > LTDC && Data.tdc[0][bLaBr] > LTDC)
								{
									LaBrSi.ELaBr[0] = calLaBr[aLaBr][0] + calLaBr[aLaBr][1] * (Data.adc[0][aLaBr] + r.Uniform(-0.5, 0.5));
									LaBrSi.ELaBr[1] = calLaBr[bLaBr][0] + calLaBr[bLaBr][1] * (Data.adc[0][bLaBr] + r.Uniform(-0.5, 0.5));
									LaBrSi.DtLaBr = 1.0 * Data.tdc[0][aLaBr] - 1.0 * Data.tdc[0][bLaBr] - LaBrTimeDiff[aLaBr][bLaBr];
									LaBrSi.numLaBr = 10 * aLaBr + bLaBr + 11;
									tLaBrSi->Fill();
								}

					tsGe = Data.timestamp[3];
					GeSi.DtsGeSi = (long long)(tsGe - tsSi);
					if (GeSi.DtsGeSi >= DtsGeSiL && Data.adc[2][0] > LADC && Data.adc[2][0] < UADC)
					{
						GeSi.EGe = calGe[0] + calGe[1] * (Data.adc[2][0] + r.Uniform(-0.5, 0.5));
						tGeSi->Fill();
					}

					if (LaBrSi.DtsLaBrSi < DtsLaBrSiL && GeSi.DtsGeSi < DtsGeSiL)
						break;
				}

				for (j = i + 1; j < n; j++)
				{
					tRaw->GetEntry(j);

					tsLaBr = Data.timestamp[1];
					LaBrSi.DtsLaBrSi = (long long)(tsLaBr - tsSi);
					if (LaBrSi.DtsLaBrSi <= DtsLaBrSiU)
						for (aLaBr = 0; aLaBr < 4; aLaBr++)
							for (bLaBr = 0; bLaBr < 4; bLaBr++)
								if (Data.adc[0][aLaBr] > LADC && Data.adc[0][bLaBr] > LADC && Data.adc[0][aLaBr] < UADC && Data.adc[0][bLaBr] < UADC && Data.tdc[0][aLaBr] > LTDC && Data.tdc[0][bLaBr] > LTDC)
								{
									LaBrSi.ELaBr[0] = calLaBr[aLaBr][0] + calLaBr[aLaBr][1] * (Data.adc[0][aLaBr] + r.Uniform(-0.5, 0.5));
									LaBrSi.ELaBr[1] = calLaBr[bLaBr][0] + calLaBr[bLaBr][1] * (Data.adc[0][bLaBr] + r.Uniform(-0.5, 0.5));
									LaBrSi.DtLaBr = 1.0 * Data.tdc[0][aLaBr] - 1.0 * Data.tdc[0][bLaBr] - LaBrTimeDiff[aLaBr][bLaBr];
									LaBrSi.numLaBr = 10 * aLaBr + bLaBr + 11;
									tLaBrSi->Fill();
								}

					tsGe = Data.timestamp[3];
					GeSi.DtsGeSi = (long long)(tsGe - tsSi);
					if (GeSi.DtsGeSi <= DtsGeSiU && Data.adc[2][0] > LADC && Data.adc[2][0] < UADC)
					{
						GeSi.EGe = calGe[0] + calGe[1] * (Data.adc[2][0] + r.Uniform(-0.5, 0.5));
						tGeSi->Fill();
					}

					if (LaBrSi.DtsLaBrSi > DtsLaBrSiU && GeSi.DtsGeSi > DtsGeSiU)
						break;
				}
			}
		}
		fRaw->Close();
	}
	fAna->Write();
	fAna->Close();
}

#ifndef __CINT__
void StandaloneApplication(int argc, char **argv)
{
	Root2Ana();
}

int main(int argc, char **argv)
{
	TApplication app("ROOT Application", &argc, argv);
	StandaloneApplication(app.Argc(), app.Argv());
	// app.Run();
	return 0;
}
#endif
