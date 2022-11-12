//---------------------------------------------------------------------------
#ifndef CFertSeriesH
#define CFertSeriesH

#include <string>
#include <map>

#include "CBaseSeries.h"
//---------------------------------------------------------------------------

typedef std::map<std::string, CBaseSeries*> fertType;

typedef fertType::iterator fertIter;
typedef fertType::const_iterator fertConstIter;

class CFertSeries
{
    friend std::ostream& operator<<(std::ostream&, const CFertSeries&);

public:
	CFertSeries() {}
	CFertSeries(int _index, int _land);
    ~CFertSeries();

	void Init(int _index, int _land);
    bool LoadFromFile(std::string const& FileName);
    std::string& Filename() { return filename; }

    fertType Fert;

private:
    int Count, Land;
    std::string filename;
};
#endif
