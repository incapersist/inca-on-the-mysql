//---------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <sstream>
#include "utils.h"

#include "CDepSeries.h"

//---------------------------------------------------------------------------
CDepSeries::CDepSeries(int _index, int _items)
{
	Init(_index, _items);
}
//---------------------------------------------------------------------------
CDepSeries::~CDepSeries()
{
    for (depConstIter it=Deposition.begin(); it!=Deposition.end(); ++it)
    {
        delete it->second;
    }

    Deposition.clear();
}
//---------------------------------------------------------------------------
void CDepSeries::Init(int _index, int _land)
{
	Count = _index;
	ItemCount = _land;
}
//---------------------------------------------------------------------------
bool CDepSeries::LoadFromFile(std::string const& FileName)
{
    std::string fname(FileName.c_str());
    filename = fname;

    int ReachCount;
    std::string row, id;

    std::ifstream infile(FileName.c_str());

    getline(infile, row);
    std::istringstream is(row);
    is >> ReachCount;

    for (int i=0; i<ReachCount; i++)
    {
        getline(infile, id);

        CBaseSeries *dep = new CBaseSeries(ItemCount, Count);

        for (int j=0; j<Count; j++)
        {
            getline(infile, row);
            std::istringstream is2(row);

            for (int k=0; k<ItemCount; k++)
            {
                is2 >> dep->Data[k][j];
            }
        }

		std::string idClean = utils::trim(id);

        dep->Loaded(true);
        Deposition[idClean] = dep;
    }

    return true;
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const CDepSeries& s)
{
    os << s.Deposition.size() << std::endl;

    for (depConstIter it=s.Deposition.begin(); it!=s.Deposition.end(); ++it)
    {
        os << it->first << std::endl;
        os << *(it->second);
    }

    return os;
}
