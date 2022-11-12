//---------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <sstream>
#include "utils.h"

#include "CFertSeries.h"

//---------------------------------------------------------------------------
CFertSeries::CFertSeries(int _index, int _land)
{
	Init(_index, _land);
}
//---------------------------------------------------------------------------
CFertSeries::~CFertSeries()
{
    for (fertConstIter it=Fert.begin(); it!=Fert.end(); ++it)
    {
        delete it->second;
    }

    Fert.clear();
}
//---------------------------------------------------------------------------
void CFertSeries::Init(int _index, int _land)
{
	Count = _index;
	Land = _land;
}
//---------------------------------------------------------------------------
bool CFertSeries::LoadFromFile(std::string const& FileName)
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

        CBaseSeries *fert = new CBaseSeries(Land, Count);

        for (int j=0; j<Count; j++)
        {
            getline(infile, row);
            std::istringstream is2(row);

            for (int k=0; k<Land; k++)
            {
                is2 >> fert->Data[k][j];
            }
        }

        std::string idClean = utils::trim(id);

        fert->Loaded(true);
        Fert[idClean] = fert;
    }

    return true;
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const CFertSeries& s)
{
    os << s.Fert.size() << std::endl;

    for (fertConstIter it=s.Fert.begin(); it!=s.Fert.end(); ++it)
    {
        os << it->first << std::endl;
        os << *(it->second);
    }

    return os;
}














