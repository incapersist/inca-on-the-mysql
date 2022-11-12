//---------------------------------------------------------------------------
#include <stdio.h>
#include <iomanip>
#include <ios>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>

#include "CResultsGroup.h"
#include "CParSet.h"

#include "version.h"
#include "utils.h"
#include "Constant.h"

using std::endl;
using std::setprecision;
using std::setw;
using std::string;
using std::streamsize;
using std::ifstream;
using std::ofstream;
using std::istringstream;
//---------------------------------------------------------------------------

//extern CParSet *ParSet;
//extern CStructure *structure;

CResultsGroup::CResultsGroup(int _index, int _reach, CParSet* ParSet, CStructure* strctr, unsigned int _land, bool _full) :
	Saved(false),
	Count(_index),
    Reaches(_reach),
    LandCount(_land),
    fullSet(_full)
{
    structure = strctr;

    structure->SetDirection(CStructure::MainStemFirst);
    structure->Reset();

    do
    {
        CStream stream = structure->GetCurrentStream();


        do
        {
 //           CResultsSet res(Count, ParSet, 4, LandCount);
            CResultsSet* res = new CResultsSet(Count, ParSet, yearsList, 4, LandCount);

            std::string reachKey = stream.GetCurrentReachID();
            SubCatchRes.insert(resultsValueType(reachKey, res));
        }
        while (stream.NextReach());

    }
    while (structure->NextStream());

    available = false;
    writeDates = false;
}

//---------------------------------------------------------------------------
void CResultsGroup::FindYears(const dateCl::Date& startDate)
{
    yearsList.clear();
    yearsList.push_back(startDate.NYear4());

    for (unsigned int TimeStep=1; TimeStep<Count; TimeStep++)
    {
        // Work out the date from the current time step and the start date
        dateCl::Date CurrentDate = startDate + (long)TimeStep;

        if (NewYear(CurrentDate))
        {
            yearsList.push_back(CurrentDate.NYear4());
        }
    }
}
//---------------------------------------------------------------------------
bool CResultsGroup::NewYear(dateCl::Date& Today)
{
    dateCl::Date Yesterday;

    Yesterday = Today - 1;

    return ((Today.NYear4() - Yesterday.NYear4()) != 0);
}
//---------------------------------------------------------------------------
CResultsGroup::CResultsGroup(CResultsGroup& rhs)
{
    Copy(rhs);
}
//---------------------------------------------------------------------------
CResultsGroup::CResultsGroup(const CResultsGroup& rhs)
{
    Copy(rhs);
}
//---------------------------------------------------------------------------
CResultsGroup& CResultsGroup::operator=(const CResultsGroup& rhs)
{
    if (&rhs != this)
    {
        Copy(rhs);
    }

    return *this;
}
//---------------------------------------------------------------------------
void CResultsGroup::Copy(const CResultsGroup& rhs)
{
    Saved = rhs.Saved;
    Count = rhs.Count;
    Reaches = rhs.Reaches;
    LandCount = rhs.LandCount;
    fullSet = rhs.fullSet;
    available = rhs.available;
    writeDates = rhs.writeDates;
//    OptionSRP = rhs.OptionSRP;
//    OptionSRPa = rhs.OptionSRPa;
//    OptionSRPb = rhs.OptionSRPb;
    structure = rhs.structure;

    for (resultsConstIter it=rhs.SubCatchRes.begin(); it != rhs.SubCatchRes.end(); ++it)
    {
            CResultsSet* res = new CResultsSet(*(it->second));
            SubCatchRes.insert(resultsValueType(it->first, res));
    }
}
//---------------------------------------------------------------------------
void CResultsGroup::Init(double val)
{
//    for (int i=0; i<Reaches; i++)
//    {
    for (resultsIter it=SubCatchRes.begin(); it != SubCatchRes.end(); ++it)
    {
        it->second->Init(val);
    }
}
//---------------------------------------------------------------------------

CResultsGroup::~CResultsGroup()
{
    for (resultsIter it=SubCatchRes.begin(); it != SubCatchRes.end(); ++it)
    {
        delete it->second;
    }
}

//---------------------------------------------------------------------------
bool CResultsGroup::LoadFromFile(const char* FileName)
{
    std::string str, landname;

    bool filesContainDates = false;
    dateCl::Date tempDate;


	std::string fname(FileName);
	ifstream infile;



    // Read total instream results

	utils::change_extension(fname, ".dsd");
	infile.open(fname.c_str());

    unsigned int i = 0;
    std::vector<std::string> contents;
    unsigned int row = 0;

    if (infile.good())
    {
    contents.resize((Count + 6) * Reaches + 5);

    while (!infile.eof())
    {
        getline(infile, contents[i]);
        ++i;
    }


    structure->SetDirection(CStructure::MainStemFirst);
    structure->Reset();

    row = 3;

    do
    {
        CStream stream = structure->GetCurrentStream();

        do
        {
            CResultsSet* results = SubCatchRes[stream.GetCurrentReachID()];

            row += 5;
            std::size_t columnCount = results->Daily[0].Data.size();

            for (unsigned int i=0; i<Count; i++)
            {
                istringstream rec(contents[row]);

                filesContainDates = (countColumns(contents[row]) == (columnCount + 1));

                if (filesContainDates)
                {
                    rec >> tempDate;
                }

                for (std::size_t k=0; k<columnCount; k++)
                {
                    rec >> results->Daily[0].Data[k][i];
                }

                ++row;
            }
        }
        while (stream.NextReach());
    }
    while (structure->NextStream());

    infile.close();
    }





	utils::change_extension(fname, ".ald");
	infile.open(fname.c_str());

    if (infile.good())
    {
    i = 0;

    while (!infile.eof())
    {
        getline(infile, contents[i]);
        ++i;
    }

    structure->SetDirection(CStructure::MainStemFirst);
    structure->Reset();

    row = 3;

    do
    {
        CStream stream = structure->GetCurrentStream();

        do
        {
            CResultsSet* results = SubCatchRes[stream.GetCurrentReachID()];

            row += 6;

            for (int j=0; j<results->LandCount; j++)
            {
                istringstream rec(contents[row]);
                rec >> landname;

	            for (int i=0; i<13; i++)
                {
                    rec >> results->Loads.Data[j][i];
	            }

                ++row;
            }
        }
        while (stream.NextReach());
    }
    while (structure->NextStream());

    infile.close();
    }




    for (unsigned int l=0; l<LandCount; ++l)
    {
		utils::change_extension(fname, ".dl", l);
		infile.open(fname.c_str());

        if (infile.good())
        {
            i = 0;

            while (!infile.eof())
            {
                getline(infile, contents[i]);
                ++i;
            }

    		structure->SetDirection(CStructure::MainStemFirst);
            structure->Reset();

            row = 3;

            do
            {
                CStream stream = structure->GetCurrentStream();

                do
                {
                    CResultsSet* results = SubCatchRes[stream.GetCurrentReachID()];

                    row += 6;

                    for (unsigned int j=0; j<Count; ++j)
                    {
                        istringstream rec(contents[row]);

                        if (filesContainDates)
                        {
                            rec >> tempDate;
                        }

	                    for (int i=0; i<11; ++i)
                        {
                            rec >> results->Land[l].Data[i][j];
	                    }

                        ++row;
                    }
                }
                while (stream.NextReach());
            }
            while (structure->NextStream());

            infile.close();
        }
    }





    // Read direct runoff results

    for (unsigned int l=0; l<LandCount; ++l)
    {
		utils::change_extension(fname, ".dr", l);
		infile.open(fname.c_str());

    if (infile.good())
    {
            i = 0;

            while (!infile.eof())
            {
                getline(infile, contents[i]);
                ++i;
            }

    	structure->SetDirection(CStructure::MainStemFirst);
        structure->Reset();

        row = 3;

        do
        {
            CStream stream = structure->GetCurrentStream();

            do
            {
                CResultsSet* results = SubCatchRes[stream.GetCurrentReachID()];

                row += 5;

                for (unsigned int i=0; i<Count; ++i)
                {
                    istringstream rec(contents[row]);

                    if (filesContainDates)
                    {
                        rec >> tempDate;
                    }

                    for (int k=0; k<4; ++k)
                    {
                        rec >> results->Direct[k].Data[l][i];
                    }

                    ++row;
                }
            }
            while (stream.NextReach());
        }
        while (structure->NextStream());

        infile.close();
        }
    }





    // Read soil water results

    for (unsigned int l=0; l<LandCount; ++l)
    {
		utils::change_extension(fname, ".sw", l);
		infile.open(fname.c_str());

    if (infile.good())
    {
            i = 0;

            while (!infile.eof())
            {
                getline(infile, contents[i]);
                ++i;
            }

    	structure->SetDirection(CStructure::MainStemFirst);
        structure->Reset();

        row = 3;

        do
        {
            CStream stream = structure->GetCurrentStream();

            do
            {
                CResultsSet* results = SubCatchRes[stream.GetCurrentReachID()];

                row += 5;

                for (unsigned int i=0; i<Count; ++i)
                {
                    istringstream rec(contents[row]);

                    if (filesContainDates)
                    {
                        rec >> tempDate;
                    }

                    for (int k=0; k<6; ++k)
                    {
                        rec >> results->Soil[k].Data[l][i];
                    }

                    ++row;
                }
            }
            while (stream.NextReach());
        }
        while (structure->NextStream());

        infile.close();
        }
    }





    // Read groundwater results

    for (unsigned int l=0; l<LandCount; ++l)
    {
		utils::change_extension(fname, ".gw", l);
		infile.open(fname.c_str());

    if (infile.good())
    {
            i = 0;

            while (!infile.eof())
            {
                getline(infile, contents[i]);
                ++i;
            }

    	structure->SetDirection(CStructure::MainStemFirst);
        structure->Reset();

        row = 3;

        do
        {
            CStream stream = structure->GetCurrentStream();

            do
            {
                CResultsSet* results = SubCatchRes[stream.GetCurrentReachID()];

                row += 5;

                for (unsigned int i=0; i<Count; ++i)
                {
                    istringstream rec(contents[row]);

                    if (filesContainDates)
                    {
                        rec >> tempDate;
                    }

                    for (int k=0; k<5; ++k)
                    {
                        rec >> results->Ground[k].Data[l][i];
                    }

                    ++row;
                }
            }
            while (stream.NextReach());
        }
        while (structure->NextStream());

        infile.close();
        }
    }





/*

    strcpy(fname, FileName);
    fnsplit(fname,drive,dir,file,ext);
    fnmerge(fname, drive, dir, file, ".std");
    infile.open(fname);

    if (infile.good())
    {
        for (int j=0; j<3; ++j)
        {
            getline(infile, str);
        }

        structure->Reset();

        do
        {
            CStream stream = structure->GetCurrentStream();

            do
            {
                resultsIter it = SubCatchRes.find(stream.GetCurrentReachID());

                for (int k=0; k<5; ++k)
                {
                    getline(infile, str);
                }

                for (int i=0; i<Count; ++i)
                {
                    getline(infile, str);
                    istringstream rec(str);

                    if (filesContainDates)
                    {
                        rec >> tempDate;
                    }

                    for (unsigned int k=0; k<LandCount; ++k)
                    {
                        rec >> it->second->SoilTemp.Data[k][i];
                    }
                }
            }
            while (stream.NextReach());
        }
        while (structure->NextStream());

        infile.close();
    }





    strcpy(fname, FileName);
    fnsplit(fname,drive,dir,file,ext);
    fnmerge(fname, drive, dir, file, ".sdd");
    infile.open(fname);

    if (infile.good())
    {
        for (int j=0; j<3; ++j)
        {
            getline(infile, str);
        }

        structure->Reset();

        do
        {
            CStream stream = structure->GetCurrentStream();

            do
            {
                resultsIter it = SubCatchRes.find(stream.GetCurrentReachID());

                for (int k=0; k<5; ++k)
                {
                    getline(infile, str);
                }

                for (int i=0; i<Count; ++i)
                {
                    getline(infile, str);
                    istringstream rec(str);

                    if (filesContainDates)
                    {
                        rec >> tempDate;
                    }

                    for (unsigned int k=0; k<LandCount; ++k)
                    {
                        rec >> it->second->SnowDepth.Data[k][i];
                    }
                }
            }
            while (stream.NextReach());
        }
        while (structure->NextStream());

        infile.close();
    }





    strcpy(fname, FileName);
    fnsplit(fname,drive,dir,file,ext);
    fnmerge(fname, drive, dir, file, ".wed");
    infile.open(fname);

    if (infile.good())
    {
        for (int j=0; j<3; ++j)
        {
            getline(infile, str);
        }

        structure->Reset();

        do
        {
            CStream stream = structure->GetCurrentStream();

            do
            {
                resultsIter it = SubCatchRes.find(stream.GetCurrentReachID());

                for (int k=0; k<5; ++k)
                {
                    getline(infile, str);
                }

                for (int i=0; i<Count; ++i)
                {
                    getline(infile, str);
                    istringstream rec(str);

                    if (filesContainDates)
                    {
                        rec >> tempDate;
                    }

                    for (unsigned int k=0; k<LandCount; ++k)
                    {
                        rec >> it->second->SnowWater.Data[k][i];
                    }
                }
            }
            while (stream.NextReach());
        }
        while (structure->NextStream());

        infile.close();
    }
*/
    Saved = true;

    return true;
}

//---------------------------------------------------------------------------
bool CResultsGroup::SaveToFile(const char* FileName,
                                CParSet *ParSet, int output, int _header,
                                unsigned int groupResults,
								bool writeDaily,
								bool writeRiverBalance,
								bool writeCellBalance,
								bool writeSoilTemperature,
								bool writeSnowDepth,
								bool writeSnowWater)
{
    streamsize prec, width;

    header = _header;

    const int outPrec = 4;
    const int outWidth = 26;

	std::ostringstream ostr;

	dateCl::Date t;
	t.Set();
	t.formatDate(dateCl::Date::MDY);
	ostr << "File created " << t;

	time = ostr.str();

	std::string name(ParSet->Name.c_str());
	parfile = "Using parameter file " + name;

    ParSet->StartDate.setFormat(dateCl::Date::DMY);

    if (groupResults == 0)
    {
        OpenFile(FileName, "AQUATIC COMPARTMENT RESULTS",
                    ".dsd", "", "", "");
    }

    	structure->SetDirection(CStructure::MainStemFirst);
        structure->Reset();

        do
        {
            CStream stream = structure->GetCurrentStream();

            if (groupResults == 1)
            {
                OpenFile(FileName, "AQUATIC COMPARTMENT RESULTS",
                            ".dsd", "", stream.ID(), "");
            }

            do
            {
                resultsIter it = SubCatchRes.find(stream.GetCurrentReachID());

                if (groupResults == 2)
                {
                    OpenFile(FileName, "AQUATIC COMPARTMENT RESULTS",
                                ".dsd", "", "", it->first);
                }

                if (header)
                {
                    reachIter itReach = ParSet->ReachPar.find(it->first);

                    outfile << "Reach " << it->first << " (";
                    outfile << itReach->second.Name() << ")" << endl;
                }

                prec = outfile.precision(outPrec);
                width = outfile.width();

                if (header)
                {
                    if (writeDates)
                    {
                        outfile << setw(outWidth) << "Date";
                    }

                    outfile << setw(outWidth) << "Flow"
                            << setw(outWidth) << "Nitrate"
                            << setw(outWidth) << "Ammonium"
                            << setw(outWidth) << "Volume"
                            << setw(outWidth) << "Organic N"
                            << setw(outWidth) << "Velocity"
                            << setw(outWidth) << "Width"
                            << setw(outWidth) << "Depth"
                            << setw(outWidth) << "Area"
                            << setw(outWidth) << "Perimeter"
                            << setw(outWidth) << "Radius"
                            << setw(outWidth) << "Residence time"
                            << endl;

                    if (writeDates)
                    {
                        outfile << setw(outWidth) << "(dd/mm/yyyy)";
                    }

                    outfile << setw(outWidth) << "(m³/s)"
                            << setw(outWidth) << "(mg/l)"
                            << setw(outWidth) << "(mg/l)"
                            << setw(outWidth) << "(m³)"
                            << setw(outWidth) << "(mg/l)"
                            << setw(outWidth) << "(m/s)"
                            << setw(outWidth) << "(m)"
                            << setw(outWidth) << "(m)"
                            << setw(outWidth) << "(m²)"
                            << setw(outWidth) << "(m)"
                            << setw(outWidth) << "(m)"
                            << setw(outWidth) << "(days)"
                            << endl;

                    if (writeDates)
                    {
                        outfile << setw(outWidth) << "====================";
                    }
                    for (int i=0; i<12; ++i)
                    {
                        outfile << setw(outWidth) << "====================";
                    }
                    outfile << endl;
                }

                for (unsigned int i=0; i<Count; ++i)
                {
                    if (writeDates)
                    {
                        outfile << setw(outWidth) << (ParSet->StartDate + (long)i);
                    }

                    for (int k=0; k<12; ++k)
                    {
                        outfile << setw(outWidth) << it->second->Daily[0].Data[k][i];
                    }

                    outfile << endl;
                }

                outfile << setprecision(prec) << setw(width) << endl;

                if (groupResults == 2) outfile.close();
            }
            while (stream.NextReach());

            if (groupResults == 1) outfile.close();
        }
        while (structure->NextStream());

        if (groupResults == 0) outfile.close();


    const int outW = 18;

    std::vector<std::string> monthStr;

    monthStr.push_back("Whole Period");
    monthStr.push_back("Jan");
    monthStr.push_back("Feb");
    monthStr.push_back("Mar");
    monthStr.push_back("Apr");
    monthStr.push_back("May");
    monthStr.push_back("Jun");
    monthStr.push_back("Jul");
    monthStr.push_back("Aug");
    monthStr.push_back("Sep");
    monthStr.push_back("Oct");
    monthStr.push_back("Nov");
    monthStr.push_back("Dec");

    structure->SetDirection(CStructure::MainStemFirst);
    structure->Reset();

    CStream stream = structure->GetCurrentStream();
    resultsIter it = SubCatchRes.find(stream.GetCurrentReachID());
    const std::vector<unsigned int> yearList = it->second->YearList();

    for (unsigned int i=0; i<yearList.size(); ++i)
    {
        std::ostringstream ostr;
        ostr << yearList[i];
        monthStr.push_back(ostr.str());
    }

    if (groupResults == 0)
    {
        OpenFile(FileName, "SIMPLE STATISTICS", ".stats", "", "", "");
    }

    structure->SetDirection(CStructure::MainStemFirst);
    structure->Reset();

    do
    {
        CStream stream = structure->GetCurrentStream();

        if (groupResults == 1)
        {
            OpenFile(FileName, "SIMPLE STATISTICS", ".stats",
                        "", stream.ID(), "");
        }

        do
        {
            resultsIter it = SubCatchRes.find(stream.GetCurrentReachID());

            if (groupResults == 2)
            {
                OpenFile(FileName, "SIMPLE STATISTICS", ".stats",
                            "", "", it->first);
            }

            std::string reachKey = stream.GetCurrentReachID();

            for (unsigned int i=0; i<reachKey.size()+10; ++i)
            {
                outfile << "*";
            }
            outfile << std::endl;

            outfile << "* Reach " << reachKey << " *" << std::endl;

            for (unsigned int i=0; i<reachKey.size()+10; ++i)
            {
                outfile << "*";
            }
            outfile << std::endl;

            unsigned int numCols = it->second->mean[0].size();
            unsigned int numRows = monthStr.size();

            outfile << std::endl << "MEAN" << std::endl;
            WriteStatsHeader(outfile, outW);

            for (unsigned int row=0; row<numRows; ++row)
            {
                outfile << setw(outW) << monthStr[row].c_str();

                for (unsigned int col=0; col<numCols; ++col)
                {
					outfile << setw(outW) << it->second->mean[row][col];
				}

                outfile << std::endl;
            }
            outfile << std::endl;

            outfile << std::endl << "MINIMUM" << std::endl;
            WriteStatsHeader(outfile, outW);

            for (unsigned int row=0; row<numRows; ++row)
            {
                outfile << setw(outW) << monthStr[row].c_str();

                for (unsigned int col=0; col<numCols; ++col)
                {
					outfile << setw(outW) << it->second->min[row][col];
				}

                outfile << std::endl;
            }
            outfile << std::endl;

            outfile << std::endl << "MAXIMUM" << std::endl;
            WriteStatsHeader(outfile, outW);

            for (unsigned int row=0; row<numRows; ++row)
            {
                outfile << setw(outW) << monthStr[row].c_str();

                for (unsigned int col=0; col<numCols; ++col)
                {
					outfile << setw(outW) << it->second->max[row][col];
				}

                outfile << std::endl;
            }
            outfile << std::endl;

            outfile << std::endl << "MEDIAN" << std::endl;
            WriteStatsHeader(outfile, outW);

            for (unsigned int row=0; row<numRows; ++row)
            {
                outfile << setw(outW) << monthStr[row].c_str();

                for (unsigned int col=0; col<numCols; ++col)
                {
					outfile << setw(outW) << it->second->median[row][col];
				}

                outfile << std::endl;
            }
            outfile << std::endl;

            outfile << std::endl << "RANGE" << std::endl;
            WriteStatsHeader(outfile, outW);

            for (unsigned int row=0; row<numRows; ++row)
            {
                outfile << setw(outW) << monthStr[row].c_str();

                for (unsigned int col=0; col<numCols; ++col)
                {
					outfile << setw(outW) << it->second->range[row][col];
				}

                outfile << std::endl;
            }
            outfile << std::endl;

            outfile << std::endl << "VARIANCE" << std::endl;
            WriteStatsHeader(outfile, outW);

            for (unsigned int row=0; row<numRows; ++row)
            {
                outfile << setw(outW) << monthStr[row].c_str();

                for (unsigned int col=0; col<numCols; ++col)
                {
					outfile << setw(outW) << it->second->variance[row][col];
				}

                outfile << std::endl;
            }
            outfile << std::endl;

            outfile << std::endl << "STANDARD DEVIATION" << std::endl;
            WriteStatsHeader(outfile, outW);

            for (unsigned int row=0; row<numRows; ++row)
            {
                outfile << setw(outW) << monthStr[row].c_str();

                for (unsigned int col=0; col<numCols; ++col)
                {
					outfile << setw(outW) << it->second->stddev[row][col];
				}

                outfile << std::endl;
            }
            outfile << std::endl;

            outfile << std::endl << "STANDARD ERROR" << std::endl;
            WriteStatsHeader(outfile, outW);

            for (unsigned int row=0; row<numRows; ++row)
            {
                outfile << setw(outW) << monthStr[row].c_str();

                for (unsigned int col=0; col<numCols; ++col)
                {
					outfile << setw(outW) << it->second->stderror[row][col];
				}

                outfile << std::endl;
            }
            outfile << std::endl;

            outfile << std::endl << "PERCENTILE" << std::endl;
            WriteStatsHeader(outfile, outW);

            for (unsigned int row=0; row<13; ++row)
            {
                outfile << setw(outW) << double(it->second->pcents[row]) / 100.0;

                for (unsigned int col=0; col<numCols; ++col)
                {
					outfile << setw(outW) << it->second->dist[row][col];
				}

                outfile << std::endl;
            }

            outfile << std::endl << std::endl << std::endl;

            if (groupResults == 2) outfile.close();
        }
        while (stream.NextReach());

        if (groupResults == 1) outfile.close();
    }
    while (structure->NextStream());

    if (groupResults == 0) outfile.close();


if (output > 0)
{
    if (groupResults == 0)
    {
        OpenFile(FileName, "ANNUAL LOAD DATA", ".ald", "", "", "");
    }

    structure->SetDirection(CStructure::MainStemFirst);
    structure->Reset();

    do
    {
        CStream stream = structure->GetCurrentStream();

        if (groupResults == 1)
        {
            OpenFile(FileName, "ANNUAL LOAD DATA",
                        ".ald", "", stream.ID(), "");
        }

        do
        {
            resultsIter it = SubCatchRes.find(stream.GetCurrentReachID());

            if (groupResults == 2)
            {
                OpenFile(FileName, "ANNUAL LOAD DATA",
                            ".ald", "", "", it->first);
            }

            prec = outfile.precision(outPrec);
            width = outfile.width(outWidth);

            if (header)
            {
                reachIter itReach = ParSet->ReachPar.find(it->first);

                outfile << "Sub-catchment " << it->first << " (";
                outfile << itReach->second.Name() << ")" << endl;

            outfile << setw(outWidth) << "Land"
                    << setw(outWidth) << "Reactive"
                    << setw(outWidth) << "Groundwater"
                    << setw(outWidth) << "Nitrate-N"
                    << setw(outWidth) << "Ammonium-N"
                    << setw(outWidth) << "Nitrate-N"
                    << setw(outWidth) << "Ammonium-N"
                    << setw(outWidth) << "Nitrate-N"
                    << setw(outWidth) << "Ammonium-N"
                    << setw(outWidth) << "Ammonium-N"
                    << setw(outWidth) << "Ammonium-N"
                    << setw(outWidth) << "Nitrate-N"
                    << setw(outWidth) << "Nitrate-N"
                    << setw(outWidth) << "Ammonium-N"
                    << setw(outWidth) << "DON"
                    << setw(outWidth) << "Organic N"
                    << setw(outWidth) << "Groundwater"
                    << setw(outWidth) << "Groundwater"
                    << setw(outWidth) << "Groundwater"
                    << setw(outWidth) << "DON"
                    << endl;

            outfile << setw(outWidth) << "Use"
                    << setw(outWidth) << "Zone Flow"
                    << setw(outWidth) << "Flow"
                    << setw(outWidth) << "Total Load"
                    << setw(outWidth) << "Total Load"
                    << setw(outWidth) << "Leaching"
                    << setw(outWidth) << "Leaching"
                    << setw(outWidth) << "Uptake"
                    << setw(outWidth) << "Uptake"
                    << setw(outWidth) << "Mineralisation"
                    << setw(outWidth) << "Nitrification"
                    << setw(outWidth) << "Denitrification"
                    << setw(outWidth) << "Fixation"
                    << setw(outWidth) << "Retention"
                    << setw(outWidth) << "Leaching"
                    << setw(outWidth) << "Sorption"
                    << setw(outWidth) << "Mineralisation"
                    << setw(outWidth) << "Nitrification"
                    << setw(outWidth) << "Denitrification"
                    << setw(outWidth) << "Total Load"
                    << endl;

            outfile << setw(outWidth) << "Group"
                    << setw(outWidth) << "(m³/km²)"
                    << setw(outWidth) << "(m³/km²)"
                    << setw(outWidth) << "(kg/ha)"
                    << setw(outWidth) << "(kg/ha)"
                    << setw(outWidth) << "(kg/ha)"
                    << setw(outWidth) << "(kg/ha)"
                    << setw(outWidth) << "(kg/ha)"
                    << setw(outWidth) << "(kg/ha)"
                    << setw(outWidth) << "(kg/ha)"
                    << setw(outWidth) << "(kg/ha)"
                    << setw(outWidth) << "(kg/ha)"
                    << setw(outWidth) << "(kg/ha)"
                    << setw(outWidth) << "(kg/ha)"
                    << setw(outWidth) << "(kg/ha)"
                    << setw(outWidth) << "(kg/ha)"
                    << setw(outWidth) << "(kg/ha)"
                    << setw(outWidth) << "(kg/ha)"
                    << setw(outWidth) << "(kg/ha)"
                    << setw(outWidth) << "(kg/ha)"
                    << endl;

                outfile << setw(outWidth) << "===============";

                for (int i=0; i<19; ++i)
                {
                    outfile << setw(outWidth) << "===============";
                }
                outfile << endl;
            }

            for (unsigned int j=0; j<LandCount; ++j)
            {
                outfile << setw(outWidth) << ParSet->ShortLand[j].c_str();

                for (int i=0; i<19; ++i)
                {
                    outfile << setw(outWidth) << it->second->Loads.Data[j][i];
                }
                outfile << endl;
            }

            outfile << setprecision(prec) << setw(width) << endl;

            if (groupResults == 2) outfile.close();
        }
        while (stream.NextReach());

        if (groupResults == 1) outfile.close();
    }
    while (structure->NextStream());

    if (groupResults == 0) outfile.close();




    if (writeDaily)
    {
        for (unsigned int k=0; k<LandCount; ++k)
        {

            if (groupResults == 0)
            {
                OpenFile(FileName, "DAILY LOAD DATA", "dl",
                            ParSet->LongLand[k], "", "", k);
            }

    		structure->SetDirection(CStructure::MainStemFirst);
            structure->Reset();

            do
            {
                CStream stream = structure->GetCurrentStream();

                if (groupResults == 1)
                {
                    OpenFile(FileName, "DAILY LOAD DATA", "dl",
                                ParSet->LongLand[k], stream.ID(), "", k);
                }

                do
                {
                    resultsIter it = SubCatchRes.find(stream.GetCurrentReachID());

                    if (groupResults == 2)
                    {
                        OpenFile(FileName, "DAILY LOAD DATA", "dl",
                                    ParSet->LongLand[k], "", it->first, k);
                    }

                    prec = outfile.precision(outPrec);
                    width = outfile.width(outWidth);

                    if (header)
                    {
                        reachIter itReach = ParSet->ReachPar.find(it->first);

                        outfile << "Sub-catchment " << it->first << " (";
                        outfile << itReach->second.Name() << ")" << endl;

                        if (writeDates)
                        {
                            outfile << setw(outWidth) << "Date";
                        }
                    outfile << setw(outWidth) << "Nitrate-N"
                            << setw(outWidth) << "Ammonium-N"
                            << setw(outWidth) << "Nitrate-N"
                            << setw(outWidth) << "Ammonium-N"
                            << setw(outWidth) << "Nitrate-N"
                            << setw(outWidth) << "Ammonium-N"
                            << setw(outWidth) << "Ammonium-N"
                            << setw(outWidth) << "Ammonium-N"
                            << setw(outWidth) << "Nitrate-N"
                            << setw(outWidth) << "Nitrate-N"
                            << setw(outWidth) << "Ammonium-N"
                            << setw(outWidth) << "DON"
                            << setw(outWidth) << "Organic N"
                            << setw(outWidth) << "Groundwater"
                            << setw(outWidth) << "Groundwater"
                            << setw(outWidth) << "Groundwater"
                            << setw(outWidth) << "DON"
                            << endl;

                        if (writeDates)
                        {
                            outfile << setw(outWidth) << " ";
                        }
                    outfile << setw(outWidth) << "Total Load"
                            << setw(outWidth) << "Total Load"
                            << setw(outWidth) << "Leaching"
                            << setw(outWidth) << "Leaching"
                            << setw(outWidth) << "Uptake"
                            << setw(outWidth) << "Uptake"
                            << setw(outWidth) << "Mineralisation"
                            << setw(outWidth) << "Nitrification"
                            << setw(outWidth) << "Denitrification"
                            << setw(outWidth) << "Fixation"
                            << setw(outWidth) << "Retention"
                            << setw(outWidth) << "Leaching"
                            << setw(outWidth) << "Sorption"
                            << setw(outWidth) << "Mineralisation"
                            << setw(outWidth) << "Nitrification"
                            << setw(outWidth) << "Denitrification"
                            << setw(outWidth) << "Total Load"
                            << endl;

                        if (writeDates)
                        {
                            outfile << setw(outWidth) << "(dd/mm/yyyy)";
                        }
                    outfile << setw(outWidth) << "(kg/ha)"
                            << setw(outWidth) << "(kg/ha)"
                            << setw(outWidth) << "(kg/ha)"
                            << setw(outWidth) << "(kg/ha)"
                            << setw(outWidth) << "(kg/ha)"
                            << setw(outWidth) << "(kg/ha)"
                            << setw(outWidth) << "(kg/ha)"
                            << setw(outWidth) << "(kg/ha)"
                            << setw(outWidth) << "(kg/ha)"
                            << setw(outWidth) << "(kg/ha)"
                            << setw(outWidth) << "(kg/ha)"
                            << setw(outWidth) << "(kg/ha)"
                            << setw(outWidth) << "(kg/ha)"
                            << setw(outWidth) << "(kg/ha)"
                            << setw(outWidth) << "(kg/ha)"
                            << setw(outWidth) << "(kg/ha)"
                            << setw(outWidth) << "(kg/ha)"
                            << endl;

                        if (writeDates)
                        {
                            outfile << setw(outWidth) << "===============";
                        }
                        for (int i=0; i<17; ++i)
                        {
                            outfile << setw(outWidth) << "===============";
                        }
                        outfile << endl;
                    }

                    for (unsigned int j=0; j<Count; ++j)
                    {
                        if (writeDates)
                        {
                            outfile << setw(outWidth) << (ParSet->StartDate + (long)j);
                        }

                        for (int i=0; i<17; ++i)
                        {
                            outfile << setw(outWidth) << it->second->Land[k].Data[i][j];
                        }
                        outfile << endl;
                    }

                    outfile << setprecision(prec) << setw(width) << endl;

                    if (groupResults == 2) outfile.close();
                }
                while (stream.NextReach());

                if (groupResults == 1) outfile.close();
            }
            while (structure->NextStream());

            if (groupResults == 0) outfile.close();
        }
    }
}




if (output > 1)
{

    for (unsigned int l=0; l<LandCount; l++)
    {
        if (groupResults == 0)
        {
            OpenFile(FileName, "DAILY DIRECT RUNOFF DATA",
                       "dr", ParSet->LongLand[l], "", "", l);
        }

    	structure->SetDirection(CStructure::MainStemFirst);
        structure->Reset();

        do
        {
            CStream stream = structure->GetCurrentStream();

            if (groupResults == 1)
            {
                OpenFile(FileName, "DAILY DIRECT RUNOFF DATA",
					"dr", ParSet->LongLand[l], stream.ID(), "", l);
            }

            do
            {
                resultsIter it = SubCatchRes.find(stream.GetCurrentReachID());

                if (groupResults == 2)
                {
                    OpenFile(FileName, "DAILY DIRECT RUNOFF DATA",
						"dr", ParSet->LongLand[l], "", it->first, l);
                }

                prec = outfile.precision(outPrec);
                width = outfile.width(outWidth);

                if (header)
                {
                    reachIter itReach = ParSet->ReachPar.find(it->first);

                    outfile << "Sub-catchment " << it->first << " (";
                    outfile << itReach->second.Name() << ")" << endl;

                    if (writeDates)
                    {
                        outfile << setw(outWidth) << "Date";
                    }
                outfile << setw(outWidth) << "Flow"
                        << setw(outWidth) << "Nitrate"
                        << setw(outWidth) << "Ammonium"
                        << setw(outWidth) << "Volume"
                        << setw(outWidth) << "DON"
                        << endl;

                    if (writeDates)
                    {
                        outfile << setw(outWidth) << "(dd/mm/yyyy)";
                    }
                outfile << setw(outWidth) << "(m³/s)"
                        << setw(outWidth) << "(mg/l)"
                        << setw(outWidth) << "(mg/l)"
                        << setw(outWidth) << "(m³)"
                        << setw(outWidth) << "(mg/l)"
                        << endl;

                    if (writeDates)
                    {
                        outfile << setw(outWidth) << "=========================";
                    }
                    for (int i=0; i<5; ++i)
                    {
                        outfile << setw(outWidth) << "=========================";
                    }
                    outfile << endl;
                }

                for (unsigned int i=0; i<Count; ++i)
                {
                    if (writeDates)
                    {
                        outfile << setw(outWidth) << (ParSet->StartDate + (long)i);
                    }

                    for (int k=0; k<5; ++k)
                    {
                        outfile << setw(outWidth) << it->second->Direct[k].Data[l][i];
                    }
                    outfile << endl;
                }
                outfile << setprecision(prec) << setw(width) << endl;

                if (groupResults == 2) outfile.close();
            }
            while (stream.NextReach());

            if (groupResults == 1) outfile.close();
        }
        while (structure->NextStream());

        if (groupResults == 0) outfile.close();
    }



    for (unsigned int l=0; l<LandCount; l++)
    {
        if (groupResults == 0)
        {
            OpenFile(FileName, "DAILY SOIL WATER DATA",
                        "sw", ParSet->LongLand[l], "", "", l);
        }

    	structure->SetDirection(CStructure::MainStemFirst);
        structure->Reset();

        do
        {
            CStream stream = structure->GetCurrentStream();

            if (groupResults == 1)
            {
                OpenFile(FileName, "DAILY SOIL WATER DATA",
                           "sw", ParSet->LongLand[l], stream.ID(), "", l);
            }

            do
            {
                resultsIter it = SubCatchRes.find(stream.GetCurrentReachID());

                if (groupResults == 2)
                {
                    OpenFile(FileName, "DAILY SOIL WATER DATA",
                                "sw", ParSet->LongLand[l], "", it->first, l);
                }

                prec = outfile.precision(outPrec);
                width = outfile.width(outWidth);

                if (header)
                {
                    reachIter itReach = ParSet->ReachPar.find(it->first);

                    outfile << "Sub-catchment " << it->first << " (";
                    outfile << itReach->second.Name() << ")" << endl;

                    if (writeDates)
                    {
                        outfile << setw(outWidth) << "Date";
                    }
                    outfile << setw(outWidth) << "Flow"
                            << setw(outWidth) << "Nitrate"
                            << setw(outWidth) << "Ammonium"
                            << setw(outWidth) << "Volume"
                            << setw(outWidth) << "Solid Organic-N"
                            << setw(outWidth) << "Organic-N"
                            << setw(outWidth) << "Drainage Volume"
                            << setw(outWidth) << "Retention Volume"
                            << endl;

                    if (writeDates)
                    {
                        outfile << setw(outWidth) << "(dd/mm/yyyy)";
                    }
                    outfile << setw(outWidth) << "(m³/s)"
                            << setw(outWidth) << "(mg/l)"
                            << setw(outWidth) << "(mg/l)"
                            << setw(outWidth) << "(m³)"
                            << setw(outWidth) << "(mg N/kg soil)"
                            << setw(outWidth) << "(mg/l)"
                            << setw(outWidth) << "(m³)"
                            << setw(outWidth) << "(m³)"
                            << endl;

                    if (writeDates)
                    {
                        outfile << setw(outWidth) << "=========================";
                    }
                    for (int i=0; i<8; ++i)
                    {
                        outfile << setw(outWidth) << "=========================";
                    }
                    outfile << endl;
                }

                for (unsigned int i=0; i<Count; ++i)
                {
                    if (writeDates)
                    {
                        outfile << setw(outWidth) << (ParSet->StartDate + (long)i);
                    }

                    for (int k=0; k<8; ++k)
                    {
                        outfile << setw(outWidth) << it->second->Soil[k].Data[l][i];
                    }
                    outfile << endl;
                }
                outfile << setprecision(prec) << setw(width) << endl;

                if (groupResults == 2) outfile.close();
            }
            while (stream.NextReach());

            if (groupResults == 1) outfile.close();
        }
        while (structure->NextStream());

        if (groupResults == 0) outfile.close();
    }



    for (unsigned int l=0; l<LandCount; l++)
    {
        if (groupResults == 0)
        {
            OpenFile(FileName, "DAILY GROUNDWATER DATA",
                        "gw", ParSet->LongLand[l], "", "", l);
        }

    	structure->SetDirection(CStructure::MainStemFirst);
        structure->Reset();

        do
        {
            CStream stream = structure->GetCurrentStream();

            if (groupResults == 1)
            {
                OpenFile(FileName, "DAILY GROUNDWATER DATA",
                            "gw", ParSet->LongLand[l], stream.ID(), "", l);
            }

            do
            {
                resultsIter it = SubCatchRes.find(stream.GetCurrentReachID());

                if (groupResults == 2)
                {
                    OpenFile(FileName, "DAILY GROUNDWATER DATA",
                                "gw", ParSet->LongLand[l], "", it->first, l);
                }

                prec = outfile.precision(outPrec);
                width = outfile.width(outWidth);

                if (header)
                {
                    reachIter itReach = ParSet->ReachPar.find(it->first);

                    outfile << "Sub-catchment " << it->first << " (";
                    outfile << itReach->second.Name() << ")" << endl;

                    if (writeDates)
                    {
                        outfile << setw(outWidth) << "Date";
                    }
                    outfile << setw(outWidth) << "Flow"
                            << setw(outWidth) << "Nitrate"
                            << setw(outWidth) << "Ammonium"
                            << setw(outWidth) << "Volume"
                            << setw(outWidth) << "Organic N"
                            << endl;

                    if (writeDates)
                    {
                        outfile << setw(outWidth) << "(dd/mm/yyyy)";
                    }
                    outfile << setw(outWidth) << "(m³/s)"
                            << setw(outWidth) << "(mg/l)"
                            << setw(outWidth) << "(mg/l)"
                            << setw(outWidth) << "(mg/l)"
                            << setw(outWidth) << "(m³)"
                            << endl;

                    if (writeDates)
                    {
                        outfile << setw(outWidth) << "=========================";
                    }
                    for (int i=0; i<5; ++i)
                    {
                        outfile << setw(outWidth) << "=========================";
                    }
                    outfile << endl;
                }

                for (unsigned int i=0; i<Count; ++i)
                {
                    if (writeDates)
                    {
                        outfile << setw(outWidth) << (ParSet->StartDate + (long)i);
                    }

                    for (int k=0; k<5; ++k)
                    {
                        outfile << setw(outWidth) << it->second->Ground[k].Data[l][i];
                    }
                    outfile << endl;
                }
                outfile << setprecision(prec) << setw(width) << endl;

                if (groupResults == 2) outfile.close();
            }
            while (stream.NextReach());

            if (groupResults == 1) outfile.close();
        }
        while (structure->NextStream());

        if (groupResults == 0) outfile.close();
    }


}





if (output > 2)
{
    if (writeRiverBalance)
    {
        if (groupResults == 0)
        {
            OpenFile(FileName, "RIVER BALANCE",
                        ".rb", "", "", "");
        }

    	structure->SetDirection(CStructure::MainStemFirst);
        structure->Reset();

        do
        {
            CStream stream = structure->GetCurrentStream();

            if (groupResults == 1)
            {
                OpenFile(FileName, "RIVER BALANCE",
                            ".rb", "", stream.ID(), "");
            }

            do
            {
                resultsIter it = SubCatchRes.find(stream.GetCurrentReachID());

                if (groupResults == 2)
                {
                    OpenFile(FileName, "RIVER BALANCE",
                                ".rb", "", "", it->first);
                }

                prec = outfile.precision(outPrec);
                width = outfile.width(outWidth);

                if (header)
                {
                    reachIter itReach = ParSet->ReachPar.find(it->first);

                    outfile << "Reach " << it->first << " (";
                    outfile << itReach->second.Name() << ")" << endl;

                    if (writeDates)
                    {
                        outfile << setw(outWidth) << "Date";
                    }
                outfile << setw(outWidth) << "Flow"
                        << setw(outWidth) << "Nitrate"
                        << setw(outWidth) << "Ammonium"
                        << setw(outWidth) << "NO3 Input"
                        << setw(outWidth) << "NO3 Output"
                        << setw(outWidth) << "NH4 Input"
                        << setw(outWidth) << "NH4 Output"
                        << setw(outWidth) << "Reach Volume"
                        << setw(outWidth) << "Input Volume"
                        << setw(outWidth) << "Output Volume"
                        << setw(outWidth) << "Denitrification"
                        << setw(outWidth) << "Nitrification"
                        << setw(outWidth) << "NO3 storage"
                        << setw(outWidth) << "NH4 storage"
                        << setw(outWidth) << "Organic N"
                        << setw(outWidth) << "DON Input"
                        << setw(outWidth) << "DON Output"
                        << setw(outWidth) << "DON storage"
                        << setw(outWidth) << "Mineralisation"
                        << setw(outWidth) << "Immobilisation"
                        << endl;

                    if (writeDates)
                    {
                        outfile << setw(outWidth) << "(dd/mm/yyyy)";
                    }
                outfile << setw(outWidth) << "(m³/s)"
                        << setw(outWidth) << "(kg)"
                        << setw(outWidth) << "(kg)"
                        << setw(outWidth) << "(kg)"
                        << setw(outWidth) << "(kg)"
                        << setw(outWidth) << "(kg)"
                        << setw(outWidth) << "(kg)"
                        << setw(outWidth) << "(m³)"
                        << setw(outWidth) << "(m³)"
                        << setw(outWidth) << "(m³)"
                        << setw(outWidth) << "(kg)"
                        << setw(outWidth) << "(kg)"
                        << setw(outWidth) << "(kg)"
                        << setw(outWidth) << "(kg)"
                        << setw(outWidth) << "(kg)"
                        << setw(outWidth) << "(kg)"
                        << setw(outWidth) << "(kg)"
                        << setw(outWidth) << "(kg)"
                        << setw(outWidth) << "(kg)"
                        << setw(outWidth) << "(kg)"
                        << endl;

                    if (writeDates)
                    {
                        outfile << setw(outWidth) << "===============";
                    }
                    for (int i=0; i<NUM_RIVER_EQN; ++i)
                    {
                        outfile << setw(outWidth) << "===============";
                    }
                    outfile << endl;
                }

                for (unsigned int i=0; i<Count; ++i)
                {
                    if (writeDates)
                    {
                        outfile << setw(outWidth) << (ParSet->StartDate + (long)i);
                    }

                    for (int k=0; k<NUM_RIVER_EQN; ++k)
                    {
                        outfile << setw(outWidth) << it->second->RiverBalance[0].Data[k][i];
                    }
                    outfile << endl;
                }
                outfile << setprecision(prec) << setw(width) << endl;

                if (groupResults == 2) outfile.close();
            }
            while (stream.NextReach());

            if (groupResults == 1) outfile.close();
        }
        while (structure->NextStream());

        if (groupResults == 0) outfile.close();
    }




    if (writeCellBalance)
    {
        for (unsigned int l=0; l<LandCount; l++)
        {
            if (groupResults == 0)
            {
                OpenFile(FileName, "LAND PHASE BALANCE", "cb",
                            ParSet->LongLand[l], "", "", l);
            }

    		structure->SetDirection(CStructure::MainStemFirst);
            structure->Reset();

            do
            {
                CStream stream = structure->GetCurrentStream();

                if (groupResults == 1)
                {
                    OpenFile(FileName, "LAND PHASE BALANCE", "cb",
                                ParSet->LongLand[l], stream.ID(), "", l);
                }

                do
                {
                    resultsIter it = SubCatchRes.find(stream.GetCurrentReachID());

                    if (groupResults == 2)
                    {
                        OpenFile(FileName, "LAND PHASE BALANCE", "cb",
                                    ParSet->LongLand[l], "", it->first, l);
                    }

                    prec = outfile.precision(outPrec);
                    width = outfile.width(outWidth);

                    if (header)
                    {
                        reachIter itReach = ParSet->ReachPar.find(it->first);

                        outfile << "Sub-catchment " << it->first << " (";
                        outfile << itReach->second.Name() << ")" << endl;

                        if (writeDates)
                        {
                            outfile << setw(outWidth) << "Date";
                        }
                    outfile << setw(outWidth) << "Soil water"
                            << setw(outWidth) << "Groundwater"
                            << setw(outWidth) << "Soil water"
                            << setw(outWidth) << "Groundwater"
                            << setw(outWidth) << "Soil water"
                            << setw(outWidth) << "Groundwater"
                            << setw(outWidth) << "Soil water"
                            << setw(outWidth) << "Groundwater"
                            << setw(outWidth) << "Total water"
                            << setw(outWidth) << "Total water"
                            << setw(outWidth) << "Total NO3"
                            << setw(outWidth) << "Total NH4"
                            << setw(outWidth) << "Total NO3"
                            << setw(outWidth) << "Total NH4"
                            << setw(outWidth) << " "
                            << setw(outWidth) << " "
                            << setw(outWidth) << " "
                            << setw(outWidth) << " "
                            << setw(outWidth) << " "
                            << setw(outWidth) << " "
                            << setw(outWidth) << " "
                            << setw(outWidth) << "Direct runoff"
                            << setw(outWidth) << "Direct runoff"
                            << setw(outWidth) << "Direct runoff"
                            << setw(outWidth) << "Direct runoff"
                            << setw(outWidth) << "Soil water"
                            << setw(outWidth) << "Soil solid"
                            << setw(outWidth) << "Groundwater"
                            << setw(outWidth) << "Total DON"
                            << setw(outWidth) << "Organic N"
                            << setw(outWidth) << "Groundwater"
                            << setw(outWidth) << "Groundwater"
                            << setw(outWidth) << "Groundwater"
                            << setw(outWidth) << "Total DON"
                            << setw(outWidth) << "Direct runoff"
                            << endl;

                        if (writeDates)
                        {
                            outfile << setw(outWidth) << " ";
                        }
                    outfile << setw(outWidth) << "Flow"
                            << setw(outWidth) << "Flow"
                            << setw(outWidth) << "Nitrate"
                            << setw(outWidth) << "Nitrate"
                            << setw(outWidth) << "Ammonium"
                            << setw(outWidth) << "Ammonium"
                            << setw(outWidth) << "Volume"
                            << setw(outWidth) << "Volume"
                            << setw(outWidth) << "Input"
                            << setw(outWidth) << "Output"
                            << setw(outWidth) << "Input"
                            << setw(outWidth) << "Input"
                            << setw(outWidth) << "Output"
                            << setw(outWidth) << "Output"
                            << setw(outWidth) << "NO3 uptake"
                            << setw(outWidth) << "NH4 uptake"
                            << setw(outWidth) << "Mineralisation"
                            << setw(outWidth) << "Nitrification"
                            << setw(outWidth) << "Denitrification"
                            << setw(outWidth) << "Fixation"
                            << setw(outWidth) << "Immobilisation"
                            << setw(outWidth) << "Flow"
                            << setw(outWidth) << "Nitrate"
                            << setw(outWidth) << "Ammonium"
                            << setw(outWidth) << "Volume"
                            << setw(outWidth) << "Organic N"
                            << setw(outWidth) << "Organic N"
                            << setw(outWidth) << "Organic N"
                            << setw(outWidth) << "Output"
                            << setw(outWidth) << "Sorption"
                            << setw(outWidth) << "Mineralisation"
                            << setw(outWidth) << "Nitrification"
                            << setw(outWidth) << "Denitrification"
                            << setw(outWidth) << "Input"
                            << setw(outWidth) << "Organic N"
                            << endl;

                        if (writeDates)
                        {
                            outfile << setw(outWidth) << "(dd/mm/yyyy)";
                        }
                    outfile << setw(outWidth) << "(m³/s)"
                            << setw(outWidth) << "(m³/s)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(m³)"
                            << setw(outWidth) << "(m³)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(m³/s)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(m³)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << setw(outWidth) << "(kg/km²)"
                            << endl;

                        if (writeDates)
                        {
                            outfile << setw(outWidth) << "===============";
                        }
                        for (int i=0; i<NUM_CELL_EQN; ++i)
                        {
                            outfile << setw(outWidth) << "===============";
                        }
                        outfile << endl;
                    }

                    for (unsigned int i=0; i<Count; ++i)
                    {
                        if (writeDates)
                        {
                            outfile << setw(outWidth) << (ParSet->StartDate + (long)i);
                        }

                        for (int k=0; k<NUM_CELL_EQN; ++k)
                        {
                            outfile << setw(outWidth) << it->second->CellBalance[l].Data[k][i];
                        }
                        outfile << endl;
                    }
                    outfile << setprecision(prec) << setw(width) << endl;

                    if (groupResults == 2) outfile.close();
                }
                while (stream.NextReach());

                if (groupResults == 1) outfile.close();
            }
            while (structure->NextStream());

            if (groupResults == 0) outfile.close();
        }
    }
}



    return true;
}
//---------------------------------------------------------------------------

std::ofstream& CResultsGroup::WriteStatsHeader(std::ofstream& outfile, const int& outW)
{
                    outfile << setw(outW) << " "
                            << setw(outW) << "Discharge"
                            << setw(outW) << "Nitrate"
                            << setw(outW) << "Ammonium"
                            << setw(outW) << "Volume"
                            << setw(outW) << "DON"
                            << std::endl;

    return outfile;
}
//---------------------------------------------------------------------------

void CResultsGroup::CalcCellBal(CParSet& ParSet)
{
    for (resultsIter it=SubCatchRes.begin(); it != SubCatchRes.end(); ++it)
    {
        for (unsigned int LandUse=0; LandUse<ParSet.LandCount(); LandUse++)
        {
            subIter itSub = ParSet.SubPar.find(it->first);

            if (itSub->second.Land[LandUse] > 0)
            {
                // Input = total water input
                it->second->CellWBal[LandUse][0] = it->second->CellBalance[LandUse].Data[8][ParSet.TimeSteps-1];

                // Output = total water flow output
                it->second->CellWBal[LandUse][1] = it->second->CellBalance[LandUse].Data[9][ParSet.TimeSteps-1];

                // Drainage store = soil water drainage volume change
                //                  + groundwater drainage volume change
                //                  + direct runoff water volume change
                it->second->CellWBal[LandUse][2] = it->second->CellBalance[LandUse].Data[6][ParSet.TimeSteps-1]
                											+ it->second->CellBalance[LandUse].Data[7][ParSet.TimeSteps-1]
                											+ it->second->CellBalance[LandUse].Data[24][ParSet.TimeSteps-1];

                // Retention store = Retention volume
                it->second->CellWBal[LandUse][3] = it->second->CellInit.Data[LandUse][NUM_CELL_EQN];

                // Initial soil = soil water initial volume
                it->second->CellWBal[LandUse][4] = it->second->CellInit.Data[LandUse][6];

                // Initial groundwater = groundwater initial volume
                it->second->CellWBal[LandUse][5] = it->second->CellInit.Data[LandUse][7];

                // Initial direct = initial direct runoff water volume change
                it->second->CellWBal[LandUse][6] = it->second->CellInit.Data[LandUse][24];

                // Balance = total water input - total water output
                //              - drainage store
                //              + initial soil + initial groundwater
                //              + initial direct
                it->second->CellWBal[LandUse][7] = it->second->CellWBal[LandUse][0]
                                            - it->second->CellWBal[LandUse][1]
                                            - it->second->CellWBal[LandUse][2]
//                                            - it->second->CellWBal[LandUse][3]
                                            + it->second->CellWBal[LandUse][4]
                                            + it->second->CellWBal[LandUse][5]
                                            + it->second->CellWBal[LandUse][6];



                // Input = Total NO3 input + Total NH4 input
                //          + NH4 mineralisation + fixation
                it->second->CellBal[LandUse][0] = (it->second->CellBalance[LandUse].Data[10][ParSet.TimeSteps-1]
                                            + it->second->CellBalance[LandUse].Data[11][ParSet.TimeSteps-1]
                                            + it->second->CellBalance[LandUse].Data[16][ParSet.TimeSteps-1]
                                            + it->second->CellBalance[LandUse].Data[19][ParSet.TimeSteps-1]) / 100.0;

                // Output =
                it->second->CellBal[LandUse][1] = (it->second->CellBalance[LandUse].Data[12][ParSet.TimeSteps-1]
                                           + it->second->CellBalance[LandUse].Data[13][ParSet.TimeSteps-1]
                                            + it->second->CellBalance[LandUse].Data[14][ParSet.TimeSteps-1]
                                            + it->second->CellBalance[LandUse].Data[15][ParSet.TimeSteps-1]
                                            + it->second->CellBalance[LandUse].Data[18][ParSet.TimeSteps-1]
                                            + it->second->CellBalance[LandUse].Data[20][ParSet.TimeSteps-1]) / 100.0;

                // Storage =
                it->second->CellBal[LandUse][2] = (it->second->CellBalance[LandUse].Data[2][ParSet.TimeSteps-1]
                                                + it->second->CellBalance[LandUse].Data[3][ParSet.TimeSteps-1]
                                                + it->second->CellBalance[LandUse].Data[4][ParSet.TimeSteps-1]
                                                + it->second->CellBalance[LandUse].Data[5][ParSet.TimeSteps-1]
                                                + it->second->CellBalance[LandUse].Data[22][ParSet.TimeSteps-1]
                                                + it->second->CellBalance[LandUse].Data[23][ParSet.TimeSteps-1])
                                                / 100.0;

                // Initial =
                it->second->CellBal[LandUse][3] = (it->second->CellInit.Data[LandUse][2]
                                                            + it->second->CellInit.Data[LandUse][3]
                                                            + it->second->CellInit.Data[LandUse][4]
                                                            + it->second->CellInit.Data[LandUse][5]
                                                            + it->second->CellInit.Data[LandUse][22]
                                                            + it->second->CellInit.Data[LandUse][23])
                                                            / 100.0;

                // Balance = Input - Output - Storage + Initial
                it->second->CellBal[LandUse][4] = it->second->CellBal[LandUse][0]
                                            - it->second->CellBal[LandUse][1]
                                            - it->second->CellBal[LandUse][2]
                                            + it->second->CellBal[LandUse][3];
            }
            else
            {
                it->second->CellBal[LandUse][0] = 0.0;
                it->second->CellBal[LandUse][1] = 0.0;
                it->second->CellBal[LandUse][2] = 0.0;
                it->second->CellBal[LandUse][3] = 0.0;
                it->second->CellBal[LandUse][4] = 0.0;
                it->second->CellBal[LandUse][5] = 0.0;
                it->second->CellBal[LandUse][6] = 0.0;
                it->second->CellBal[LandUse][7] = 0.0;
            }
        }
    }
}
//---------------------------------------------------------------------------
void CResultsGroup::ResetErrorMatrix(void)
{
    for (resultsIter it=SubCatchRes.begin(); it != SubCatchRes.end(); ++it)
    {
        it->second->ResetErrorMatrix();
    }
}
//---------------------------------------------------------------------------
void CResultsGroup::CalcRiverBal(CParSet& ParSet)
{
    for (resultsIter it=SubCatchRes.begin(); it != SubCatchRes.end(); ++it)
    {
        // Water input
        it->second->RiverWBal[0] = it->second->RiverBalance[0].Data[8][ParSet.TimeSteps-1];

        // Water output
        it->second->RiverWBal[1] = it->second->RiverBalance[0].Data[9][ParSet.TimeSteps-1];

        // Water storage
        it->second->RiverWBal[2] = it->second->RiverBalance[0].Data[7][ParSet.TimeSteps-1];

        // Water store initial
        it->second->RiverWBal[3] = it->second->ReachInit[7];

        // Water balance
        it->second->RiverWBal[4] = it->second->RiverWBal[0]
                                            - it->second->RiverWBal[1]
                                            - it->second->RiverWBal[2]
                                            + it->second->RiverWBal[3];



        // Nitrate input
        it->second->RiverNO3Bal[0] = it->second->RiverBalance[0].Data[3][ParSet.TimeSteps-1]
                                                + it->second->RiverBalance[0].Data[11][ParSet.TimeSteps-1];

        // Nitrate output
        it->second->RiverNO3Bal[1] = it->second->RiverBalance[0].Data[4][ParSet.TimeSteps-1]
                                                + it->second->RiverBalance[0].Data[10][ParSet.TimeSteps-1];

        // Nitrate storage
        it->second->RiverNO3Bal[2] = it->second->RiverBalance[0].Data[1][ParSet.TimeSteps-1];

        // Nitrate store initial
        it->second->RiverNO3Bal[3] = it->second->ReachInit[1];

        // Nitrate balance
        it->second->RiverNO3Bal[4] = it->second->RiverNO3Bal[0]
                                            - it->second->RiverNO3Bal[1]
                                            - it->second->RiverNO3Bal[2]
                                            + it->second->RiverNO3Bal[3];



        // Ammonium input
        it->second->RiverNH4Bal[0] = it->second->RiverBalance[0].Data[5][ParSet.TimeSteps-1];

        // Ammonium output
        it->second->RiverNH4Bal[1] = it->second->RiverBalance[0].Data[6][ParSet.TimeSteps-1]
                                                + it->second->RiverBalance[0].Data[11][ParSet.TimeSteps-1];

        // Ammonium storage
        it->second->RiverNH4Bal[2] = it->second->RiverBalance[0].Data[2][ParSet.TimeSteps-1];

        // Ammonium store initial
        it->second->RiverNH4Bal[3] = it->second->ReachInit[2];

        // Ammonium balance
        it->second->RiverNH4Bal[4] = it->second->RiverNH4Bal[0]
                                            - it->second->RiverNH4Bal[1]
                                            - it->second->RiverNH4Bal[2]
                                            + it->second->RiverNH4Bal[3];
    }
}
//---------------------------------------------------------------------------

void CResultsGroup::CalcErrors(CParSet* ParSet,
                                int option,
                                bool calcMeans,
								CObsContainer& ObsSources)
{
 //   dateCl::Date start(ParSet->StartDate.NYear4(),
 //                       ParSet->StartDate.NMonth(),
 //                       ParSet->StartDate.Day());

    double o, p;
    CObsSet ErrorObsSet;

    if (option == -1)
    {
        ErrorObsSet = ObsSources[0];

        if (ObsSources.size() > 1)
        {
            for (CObsContainer::iterator it=ObsSources.begin(); it!=ObsSources.end(); ++it)
            {
                ErrorObsSet.Append(it->second);
            }

            ErrorObsSet.MergeTypes();

            if (calcMeans)
            {
                ErrorObsSet.CalcMeans();
            }
        }
    }
    else
    {
        ErrorObsSet = ObsSources[option];
    }

    ResetErrorMatrix();

    for (std::size_t i=0; i<ErrorObsSet.ObsList.size(); i++)
    {
        CObsItem *series = reinterpret_cast<CObsItem *>(ErrorObsSet.ObsList[i]);

        double oSum = 0.0, pSum = 0.0, oDiffSum = 0.0, pDiffSum = 0.0;
        double oDivSum = 0.0, xySum = 0.0, x2Sum = 0.0, y2Sum = 0.0;
        double oDiff, pDiff;

        std::string id = series->GetReach();
        int type = series->GetType();

        resultsIter it = SubCatchRes.find(id);

        if (it != SubCatchRes.end())
        {

        switch (type)
        {
            case 1 :   type = 0;
                        break;
            case 2 :   type = 1;
                        break;
            case 3 :   type = 2;
                        break;
            case 5 :   type = 4;
                        break;
            default :   type = -9;
        }

        if (type > -9)
        {
            unsigned int count = series->GetCount();
            unsigned int trueCount = 0;

            for (unsigned int i=0; i<count; ++i)
            {
                if (series->Data[i] > 0.0)
                {
                    std::size_t pIndex = (long)series->Date[i].julDate() - (long)ParSet->StartDate.julDate();

                    if (pIndex >= 0 && pIndex < it->second->Daily[0].Data[type].size())
                    {
                        o = series->Data[i];
                        p = it->second->Daily[0].Data[type][pIndex];

                        oSum += o;
                        oDiff = o - p;
                        pSum += p;
                        pDiff = p - o;
                        oDiffSum += std::pow(oDiff, 2.0);
                        pDiffSum += std::pow(pDiff, 2.0);
                        oDivSum += oDiff / o;

                        xySum += o * p;
                        x2Sum += std::pow(o, 2.0);
                        y2Sum += std::pow(p, 2.0);

                        ++trueCount;
                    }
                }
            }

            if (trueCount > 4)
            {
                double oMean = oSum / trueCount;
                double pMean = pSum / trueCount;
                double oDiffMean, oDiffMeanSum = 0.0;

                for (unsigned int i=0; i<count; ++i)
                {
                    if (series->Data[i] > 0.0)
                    {
                        std::size_t pIndex = (long)series->Date[i].julDate() - (long)ParSet->StartDate.julDate();

                        if (pIndex >= 0 && pIndex < it->second->Daily[0].Data[type].size())
                        {
                            oDiffMean = series->Data[i] - oMean;
                            oDiffMeanSum += std::pow(oDiffMean, 2.0);
                        }
                    }
                }

                double Cxy = xySum - trueCount * oMean * pMean;
                double Cxx = x2Sum - trueCount * std::pow(oMean, 2.0);
                double Cyy = y2Sum - trueCount * std::pow(pMean, 2.0);

                if (oDiffMeanSum > 0.0)
                {
                    it->second->ns[type] = (oDiffMeanSum - oDiffSum) / oDiffMeanSum;
                }
                else
                {
                    it->second->ns[type] = 0.0;
                }
                it->second->r2[type] = std::pow((Cxy / std::sqrt(Cxx * Cyy)), 2.0);
                it->second->rmse[type] = (100.0 / oMean) * std::sqrt(pDiffSum / trueCount);
                it->second->re[type] = (100.0 / trueCount) * oDivSum;
                it->second->n[type] = trueCount;

            }
        }
        }
    }
}
//---------------------------------------------------------------------------
CErrorMatrix CResultsGroup::GetErrorMatrix(void)
{
    resultsIter it=SubCatchRes.begin();

    int numItems = it->second->ItemCount;
    CErrorMatrix err(Reaches, numItems);

    int i = 0;

    structure->SetDirection(CStructure::MainStemLast);
    structure->Reset();

    do
    {
        CStream stream = structure->GetCurrentStream();

        do
        {
            resultsIter it = SubCatchRes.find(stream.GetCurrentReachID());

            for (int j=0; j<numItems; ++j)
            {
                err(i,j) = it->second->r2[j];
            }

            ++i;
        }
        while (stream.NextReach());
    }
    while (structure->NextStream());

    return err;
}
//---------------------------------------------------------------------------
std::size_t CResultsGroup::countColumns(std::string& rec)
{
    std::vector<std::string> cols;
    std::istringstream istr(rec);
    std::string str;

    cols.clear();

    while (!istr.eof())
    {
        istr >> str;
        cols.push_back(str);
    }

    return cols.size();
}
//---------------------------------------------------------------------------
void CResultsGroup::CalcStats(void)
{
    for (resultsIter it=SubCatchRes.begin(); it != SubCatchRes.end(); ++it)
    {
        it->second->CalcStats();
    }
}
//---------------------------------------------------------------------------
void CResultsGroup::OpenFile(const char* FileName,
	std::string title,
	std::string newext,
	std::string subType,
	std::string stream,
	std::string reach,
	int i)
{
	//    char *drive, *dir, *file, *ext, *fname;
	std::string fullTitle;
	std::string fname(FileName);

	utils::remove_extension(fname);

	if (stream.size() > 0)
	{
		fname += " (stream ";
		fname += stream.c_str();
		fname += ")";
	}

	if (reach.size() > 0)
	{
		fname += " (reach ";
		fname += reach.c_str();
		fname += ")";
	}

	utils::add_extension(fname, newext.c_str(), i + 1);

	outfile.open(fname.c_str());

	if (header)
	{
		fullTitle = "Branching INCA-N v" + VERSION + " - " + title.c_str();
		outfile << fullTitle.c_str();
		if (subType.size() > 0) outfile << " (" << subType.c_str() << ")";
		outfile << endl;
		outfile << time.c_str() << endl;
		outfile << parfile.c_str() << endl << endl;
	}
}
//---------------------------------------------------------------------------



