#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
using namespace std;
#include "TApplication.h"
#include "TFile.h"
#include "TTree.h"

#include "setup.h"

void Root2Mca()
{
	const int LADC = 100, UADC = 7600;

	if (access("../MCA", F_OK) != 0)
		system("mkdir ../MCA");

	int min, max, runNum;
	cout << "Please input the minium and maximum number of files:\n";
	cin >> min >> max;
	ostringstream runNumSS;
	string rootFile, mcaFold, mcaFile;
	struct StrBranch Data;
	int countsADC[ADC_MODS][ADC_CHANS][8192];
	int i, j, k;
	ostringstream iSS, jSS;
	unsigned long long nEntries, iEvent;

	for (runNum = min; runNum <= max; runNum++)
	{
		runNumSS.str("");
		runNumSS << setw(5) << setfill('0') << runNum;
		rootFile = "../RootData/run" + runNumSS.str() + "_final.root";
		cout << "\n**************************************************\n";
		cout << "Now processing " << rootFile << endl;
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
		memset(countsADC, 0, sizeof(countsADC));
		nEntries = tRaw->GetEntries();
		for (iEvent = 0; iEvent < nEntries; iEvent++)
		{
			tRaw->GetEntry(iEvent);
			for (i = 0; i < ADC_MODS; i++)
				for (j = 0; j < ADC_CHANS; j++)
					if (Data.adc[i][j] > LADC && Data.adc[i][j] < UADC)
						countsADC[i][j][Data.adc[i][j]]++;
			if (iEvent % 10000 == 0)
				printf("**********Please be patient, I'm working ...... %llu/%llu  of Run#%d have been processed!**********\n", iEvent, nEntries, runNum);
		}
		fRaw->Close();

		mcaFold = "../MCA/mca" + runNumSS.str();
		if (access(mcaFold.c_str(), F_OK) != 0)
			system(("mkdir " + mcaFold).c_str());

		for (i = 0; i < ADC_MODS; i++)
			for (j = 0; j < ADC_CHANS; j++)
			{
				iSS.str("");
				iSS << setw(2) << setfill('0') << i;
				jSS.str("");
				jSS << setw(2) << setfill('0') << j;

				mcaFile = mcaFold + "/run" + runNumSS.str() + "_" + iSS.str() + "_" + jSS.str() + ".mca";
				ofstream fMca(mcaFile.c_str());
				fMca << "<<PMCA SPECTRUM>>\n"
					 << "TAG - KODAQ\n"
					 << "DESCRIPTION - \n"
					 << "GAIN - 5\n"
					 << "THRESHOLD - \n"
					 << "LIVE_MODE - 0\n"
					 << "PRESET_TIME - 0\n"
					 << "LIVE_TIME - \n"
					 << "REAL_TIME - \n"
					 << "START_TIME - \n"
					 << "SERIAL_NUMBER - \n"
					 << "<<DATA>>\n";
				for (k = 0; k < 8192; k++)
					fMca << countsADC[i][j][k] << endl;
				fMca << "<<END>>\n";
				fMca.close();
			}
	}
}

#ifndef __CINT__
void StandaloneApplication(int argc, char **argv)
{
	Root2Mca();
}

int main(int argc, char **argv)
{
	TApplication app("ROOT Application", &argc, argv);
	StandaloneApplication(app.Argc(), app.Argv());
	// app.Run();
	return 0;
}
#endif