//---------------------------------------------------------------------------
#include "CLandItem.h"
#include "Constant.h"

//---------------------------------------------------------------------------
CLandItem::CLandItem(unsigned int _landuse) : numLand(_landuse)
{
    landUse.resize(numLand);
}

//---------------------------------------------------------------------------
std::vector<double>& CLandItem::GetLandUse(void)
{
    return landUse;
}

//---------------------------------------------------------------------------
bool CLandItem::HasDates(dateCl::Date& date)
{
    return (date >= start && date <= end);
}

//---------------------------------------------------------------------------
std::istream& operator>>(std::istream& file, CLandItem& l)
{
    int year, month, day;
    char c;

    file >> day >> c >> month >> c >> year;
    l.start = dateCl::Date(month, day, year);

    file >> day >> c >> month >> c >> year;
    l.end = dateCl::Date(month, day, year);

    for (int land=0; land<l.numLand; ++land)
    {
        file >> l.landUse[land];
    }

    return file;
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const CLandItem& s)
{
    os << s.start;
    os << "\t";
    os << s.end;
    os << "\t";

    for (int land=0; land<s.numLand; ++land)
    {
        os << s.landUse[land] << "\t";
    }

    return os;
}
//---------------------------------------------------------------------------

