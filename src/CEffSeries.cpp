//---------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <sstream>
#include "utils.h"

#include "CEffSeries.h"

//---------------------------------------------------------------------------
CEffSeries::CEffSeries(int _index, int _items) :
    Count(_index),
    ItemCount(_items)
{
}
//---------------------------------------------------------------------------
CEffSeries::~CEffSeries()
{
    for (effConstIter it=Effluent.begin(); it!=Effluent.end(); ++it)
    {
        delete it->second;
    }

    Effluent.clear();
}
//---------------------------------------------------------------------------
bool CEffSeries::LoadFromFile(std::string const& FileName)
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

        CBaseSeries *eff = new CBaseSeries(ItemCount, Count);

        for (int j=0; j<Count; j++)
        {
            getline(infile, row);
            std::istringstream is2(row);

            for (int k=0; k<ItemCount; k++)
            {
                is2 >> eff->Data[k][j];
            }
        }

        std::string idClean = utils::trim(id);

        eff->Loaded(true);
        Effluent[idClean] = eff;
    }

    return true;
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const CEffSeries& s)
{
    os << s.Effluent.size() << std::endl;

    for (effConstIter it=s.Effluent.begin(); it!=s.Effluent.end(); ++it)
    {
        os << it->first << std::endl;
        os << *(it->second);
    }

    return os;
}


