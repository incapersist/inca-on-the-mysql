//---------------------------------------------------------------------------
#ifndef CLandItemH
#define CLandItemH
//---------------------------------------------------------------------------

#include <vector>
#include <istream>

#include "DATECL.h"

class CLandItem
{
    friend std::istream& operator>>(std::istream&, CLandItem&);
    friend std::ostream& operator<<(std::ostream&, const CLandItem&);

public:
        CLandItem(unsigned int _landuse);

        std::vector<double>& GetLandUse(void);
        bool HasDates(dateCl::Date& date);

private:
        dateCl::Date start, end;
        std::vector<double> landUse;
        int numLand;
};
#endif