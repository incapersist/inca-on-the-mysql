//---------------------------------------------------------------------------
#include <stdio.h>
#include <sstream>

#include "CGrowthSeries.h"
#include "DATECL.h"
#include "utils.h"

//---------------------------------------------------------------------------
CGrowthSeries::CGrowthSeries(unsigned int _land)
{
	Init(_land);
}

//---------------------------------------------------------------------------
void CGrowthSeries::Init(int _land)
{
	landCount = _land;
}
//---------------------------------------------------------------------------

bool CGrowthSeries::LoadFromFile(const char* FileName)
{
    std::string fname(FileName);
    filename = fname;

    std::ifstream infile(FileName);

    while (!infile.eof())
    {
        std::string id;
        getline(infile, id);
        id = utils::trim( id );

        CGrowthSeriesItem item(landCount);
        item.LoadFromFile(infile);

        Growth[id] = item;
    }

    return true;
}
//---------------------------------------------------------------------------
/*
std::ofstream& CGrowthSeries::serialize(std::ofstream& out)
{
    for (growIter it=Growth.begin(); it != Growth.end(); ++it)
    {
        out << it->second;
    }

    return out;
}
*/
//---------------------------------------------------------------------------

//std::ofstream& operator<<(std::ofstream& out, CGrowthSeries& g)
//{
//	g.serialize(out);
//	return out;
//}
