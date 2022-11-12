//---------------------------------------------------------------------------
#ifndef CLandSeriesH
#define CLandSeriesH
//---------------------------------------------------------------------------

#include <vector>
#include <map>
#include <string>

#include "CLandItem.h"

class CLandSeries
{
    friend std::ostream& operator<<(std::ostream&, const CLandSeries&);

public:
	CLandSeries() {}
    CLandSeries(const char* filename,  unsigned int _landuse);

    std::vector<CLandItem>& GetReachPeriods(std::string&);
    std::vector<double>& GetLandUse(dateCl::Date&, std::string&);
    std::string& Filename() { return filename; }
    bool HasReach(std::string& reachID);

private:
    std::map<std::string, std::vector<CLandItem> > series;
    std::string filename;
};
#endif
