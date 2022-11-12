//---------------------------------------------------------------------------
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "CAbsSeries.h"

//---------------------------------------------------------------------------

CAbsSeries::CAbsSeries(int _index) : Count(_index)
{
}
//---------------------------------------------------------------------------
bool CAbsSeries::LoadFromFile(const char* FileName)
{
    std::string fname(FileName);
    filename = fname;

    int ReachCount;
    std::string row, id;

    std::ifstream infile(FileName);

    getline(infile, row);
    std::istringstream is(row);
    is >> ReachCount;

    for (int i=0; i<ReachCount; i++)
    {
        getline(infile, id);

        CBaseSeries abs(1, Count);

        for (int j=0; j<Count; j++)
        {
            getline(infile, row);
            std::istringstream is2(row);

            for (int k=0; k<1; k++)
            {
                is2 >> abs.Data[k][j];
            }
        }

        Effluent[id] = abs;
    }

    return true;
}



