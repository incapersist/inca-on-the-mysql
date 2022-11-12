//---------------------------------------------------------------------------
#ifndef CEffSeriesH
#define CEffSeriesH
//---------------------------------------------------------------------------
#include <string>
#include <map>

#include "CBaseSeries.h"

typedef std::map<std::string, CBaseSeries*> effType;

typedef effType::iterator effIter;
typedef effType::const_iterator effConstIter;

class CEffSeries
{
    friend std::ostream& operator<<(std::ostream&, const CEffSeries&);

public:
    CEffSeries(int _index, int _items);
    ~CEffSeries();

    bool LoadFromFile(std::string const& FileName);
//    void SaveToFile(const std::string& fname);
    std::string& Filename() { return filename; }

    effType Effluent;

private:
    int Count, ItemCount;
    std::string filename;
};
#endif
