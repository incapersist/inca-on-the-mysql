//---------------------------------------------------------------------------
#include <fstream>

#include "CLandSeries.h"
#include "CLandItem.h"
#include "Constant.h"
#include "utils.h"

//---------------------------------------------------------------------------
CLandSeries::CLandSeries(const char* file, unsigned int _landuse)
{
    std::string fname(file);
    filename = fname;

    std::ifstream infile(file);
    std::string reachID;
    unsigned int numPeriods;

    while (infile >> reachID)
    {
        infile >> numPeriods;
        std::vector<CLandItem> period;

        for (unsigned int i=0; i<numPeriods; ++i)
        {
            CLandItem item(_landuse);
            infile >> item;
            period.push_back(item);
        }

        std::string idClean = utils::trim(reachID);

        series[idClean] = period;
    }
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const CLandSeries& s)
{
    for (std::map<std::string, std::vector<CLandItem> >::const_iterator it=s.series.begin(); it!=s.series.end(); ++it)
    {
        os << it->first << std::endl;
        os << it->second.size() << std::endl;

        for (unsigned int i=0;i<it->second.size(); ++i)
        {
            os << it->second[i] << std::endl;
        }
    }

    return os;
}
//---------------------------------------------------------------------------
std::vector<double>& CLandSeries::GetLandUse(dateCl::Date& date, std::string& reachID)
{
    std::map<std::string, std::vector<CLandItem> >::iterator it = series.find(reachID);

    for (unsigned int i=0; i<it->second.size(); ++i)
    {
        if (it->second[i].HasDates(date))
        {
            return it->second[i].GetLandUse();
        }
    }

    throw;
}
//---------------------------------------------------------------------------
std::vector<CLandItem>& CLandSeries::GetReachPeriods(std::string& reachID)
{
    std::map<std::string, std::vector<CLandItem> >::iterator it = series.find(reachID);

    if (it != series.end())
    {
        return it->second;
    }

    throw;
}
//---------------------------------------------------------------------------
bool CLandSeries::HasReach(std::string& reachID)
{
    std::map<std::string, std::vector<CLandItem> >::iterator it = series.find(reachID);

    return (it != series.end());
}
