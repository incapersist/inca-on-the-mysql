//---------------------------------------------------------------------------
#ifndef CDepSeriesH
#define CDepSeriesH
//---------------------------------------------------------------------------
#include <string>
#include <map>

#include "CBaseSeries.h"

typedef std::map<std::string, CBaseSeries*> depType;

typedef depType::iterator depIter;
typedef depType::const_iterator depConstIter;

class CDepSeries
{
    friend std::ostream& operator<<(std::ostream&, const CDepSeries&);

public:
	CDepSeries() {}
    CDepSeries(int _index, int _items);
    ~CDepSeries();

	void Init(int _index, int _land);
	bool LoadFromFile(std::string const& FileName);
    std::string& Filename() { return filename; }

    depType Deposition;

private:
    int Count, ItemCount;
    std::string filename;
};
#endif
 