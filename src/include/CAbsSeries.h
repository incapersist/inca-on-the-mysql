//---------------------------------------------------------------------------
#ifndef CAbsSeriesH
#define CAbsSeriesH
//---------------------------------------------------------------------------
//#include <string>
#include <map>

#include "CBaseSeries.h"

typedef std::map<std::string, CBaseSeries> absType;

typedef absType::iterator absIter;
typedef absType::const_iterator absConstIter;

class CAbsSeries
{
public:
    CAbsSeries(int _index);

    bool LoadFromFile(const char* FileName);
    std::string& Filename() { return filename; }

    absType Effluent;

private:
    int Count;
    std::string filename;
};
#endif
