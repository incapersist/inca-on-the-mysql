//---------------------------------------------------------------------------
#ifndef CGrowthSeriesItemH
#define CGrowthSeriesItemH
//---------------------------------------------------------------------------
#include "DATECL.h"
#include <string>
#include <fstream>

#include "CBaseSeries.h"

class CGrowthSeriesItem
{
//	friend std::ostream& operator<<(std::ofstream&, const CGrowthSeriesItem&);

public:
    CGrowthSeriesItem();
    CGrowthSeriesItem(unsigned int _land);

    bool LoadFromFile(std::istream&);
    std::string& Filename() { return filename; }

    CBaseSeries StartDate, EndDate, Offset, Amplitude, V;
    std::vector<int> PeriodCount;
private:
    dateCl::Date ReadDate(std::string&);

    std::string filename;
    unsigned int landCount;
};

#endif
