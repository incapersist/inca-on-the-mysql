//---------------------------------------------------------------------------
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

#include "CGrowthSeriesItem.h"

CGrowthSeriesItem::CGrowthSeriesItem()
{
}

//---------------------------------------------------------------------------
CGrowthSeriesItem::CGrowthSeriesItem(unsigned int _land) :
    landCount(_land)
{
    std::string name = "";

    StartDate.Setup(landCount, 50, name);
    EndDate.Setup(landCount, 50, name);
    Offset.Setup(landCount, 50, name);
    Amplitude.Setup(landCount, 50, name);
    V.Setup(landCount, 50, name);

    PeriodCount.resize(landCount);

    for (unsigned int i=0; i<landCount; i++)
    {
        PeriodCount[i] = 0;
    }
}
//---------------------------------------------------------------------------
bool CGrowthSeriesItem::LoadFromFile(std::istream& infile)
{
    int LandNum, Num;
    double off, amp, v;
    std::string row, sDate, eDate;

    getline(infile, row);
    std::istringstream is(row);
    is >> LandNum;

    getline(infile, row);
    is.str(row);
    is >> Num;

    PeriodCount[LandNum-1] = Num;

    for (int i=0; i<Num; i++)
    {
        getline(infile, row);
        is.str(row);
        is >> sDate;
        dateCl::Date start = ReadDate(sDate);

        getline(infile, row);
        is.str(row);
        is >> eDate;
        dateCl::Date end = ReadDate(eDate);

        getline(infile, row);
        is.str(row);
        is >> off >> amp >> v;

        StartDate.Data[LandNum-1][i] = start.julDate();
        EndDate.Data[LandNum-1][i] = end.julDate();
        Offset.Data[LandNum-1][i] = off;
        Amplitude.Data[LandNum-1][i] = amp;
        V.Data[LandNum-1][i] = v;
    }

    return true;
}
//---------------------------------------------------------------------------
dateCl::Date CGrowthSeriesItem::ReadDate(std::string& dateStr)
{
    char x[10] = {0};
    std::istringstream is(dateStr);
    short num;
    std::vector<short> fields;

    while (is.get(x, '/'))
    {
        std::string field(x);
        std::istringstream is2(field);
        is2 >> num;

        fields.push_back(num);
    }

    return dateCl::Date(fields[1], fields[0], fields[2]);
}
//---------------------------------------------------------------------------
/*
std::ostream& operator<<(std::ofstream& out, const CGrowthSeriesItem& gsi)
{
	for (unsigned int land = 0; land < gsi.landCount; ++land)
	{
		for (int g = 0; g < gsi.PeriodCount[land]; ++g)
		{
			out << unsigned int(land + 1) << std::endl;
			out << dateCl::Date(gsi.StartDate.Data[land][g]) << "\t";
			out << dateCl::Date(gsi.EndDate.Data[land][g]) << "\t";
			out << gsi.Offset.Data[land][g] << "\t";
			out << gsi.Amplitude.Data[land][g] << std::endl;
			out << gsi.V.Data[land][g] << std::endl;
		}
	}

	return out;
}
*/