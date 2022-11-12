//---------------------------------------------------------------------------
#include "CBaseSeries.h"
#include "CRecord.h"

#include <stdio.h>
#include <fstream>
#include <exception>

//---------------------------------------------------------------------------
//#pragma package(smart_init)
//---------------------------------------------------------------------------
CBaseSeries::CBaseSeries()
{
//    Data = NULL;
}
//---------------------------------------------------------------------------
CBaseSeries::CBaseSeries(unsigned int _item, unsigned int _index,
                            std::vector<std::string>& strs, std::string _name)
{
//    Data = NULL;

    Setup(_item, _index, _name);
    names = strs;
}
//---------------------------------------------------------------------------
CBaseSeries::CBaseSeries(unsigned int _item, unsigned int _index,
                            std::string _name)
{
//    Data = NULL;

    Setup(_item, _index, _name);
}
/*//---------------------------------------------------------------------------
CBaseSeries::~CBaseSeries()
{
    clear();
}
//---------------------------------------------------------------------------
CBaseSeries::CBaseSeries(const CBaseSeries& rhs)
{
//    clear();
    copy(rhs);
}
//---------------------------------------------------------------------------
CBaseSeries& CBaseSeries::operator=(const CBaseSeries& rhs)
{
    if (&rhs != this)
    {
//        clear();
        copy(rhs);
    }

    return *this;
}
//---------------------------------------------------------------------------
void CBaseSeries::clear(void)
{
    for (unsigned int i=0; i<ItemCount; i++)
    {
        delete[] Data[i];
    }
    delete[] Data;

    Data = NULL;
}
//---------------------------------------------------------------------------
void CBaseSeries::copy(const CBaseSeries& rhs)
{
    Setup(rhs.ItemCount, rhs.count, rhs.name);

    for (unsigned int i=0; i<ItemCount; i++)
    {
        for (unsigned int j=0; j<count; j++)
        {
            Data[i][j] = rhs.Data[i][j];
        }
    }
} */
//---------------------------------------------------------------------------
void CBaseSeries::Setup(unsigned int _item, unsigned int _index,
                            std::string _name)
{
    ItemCount = _item;
    count = _index;
    name = _name;

    missing = false;
    loaded = false;

    try
    {
 //       Data = new float *[ItemCount];
        Data.resize(ItemCount);

        for (unsigned int i=0; i<ItemCount; i++)
        {
//            Data[i] = new float[count];
            Data[i].resize(count);
        }
    }
    catch (...)
    {
        exit(-1);
    }
}
//---------------------------------------------------------------------------
void CBaseSeries::Init(double val)
{
    for (unsigned int i=0; i<ItemCount; i++)
    {
        for (unsigned int j=0; j<count; j++)
        {
            Data[i][j] = val;
        }
    }
}
/*
//---------------------------------------------------------------------------
CBaseSeries::CBaseSeries(const CBaseSeries& rhs)
{
    count = rhs.count;
    ItemCount = rhs.ItemCount;
    name = rhs.name;

    Data = rhs.Data;
}
//---------------------------------------------------------------------------
CBaseSeries& CBaseSeries::operator=(const CBaseSeries& rhs)
{
    if (&rhs != this)
    {
        count = rhs.count;
        ItemCount = rhs.ItemCount;
        name = rhs.name;

    Data = rhs.Data;
    }

    return *this;
}     */
//---------------------------------------------------------------------------
bool CBaseSeries::LoadFromFile(const char* FileName)
{
    std::string fname(FileName);
    filename = fname;

    std::string row;
    unsigned int cols = 0;

    std::ifstream infile(FileName);

        for (unsigned int j=0; j<count; j++)
        {
            getline(infile, row);
            std::istringstream is2(row);

//            for (int k=0; k<ItemCount; k++)
            while (!is2.eof())
            {
                is2 >> Data[cols][j];
                ++cols;
            }
        }

    missing = (cols != ItemCount);

    return true;
}
//---------------------------------------------------------------------------
/*bool CBaseSeries::LoadFromFile(const char* FileName)
{
    std::string fname(FileName);
    filename = fname;

    std::ifstream in(FileName);
    CRecord rec(in);

    float val;

    unsigned int cols = rec.count();

    for (unsigned int j=0; j<count; ++j)
    {
        std::istringstream istr(rec.str());

        for (unsigned int i=0; i<cols; ++i)
        {
              istr >> val;
              Data[i][j] = val;
        }

        if (j<count-1) rec.read(in);
    }

    missing = (cols != ItemCount);

    return true;
}    */
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const CBaseSeries& s)
{
    for (unsigned int j=0; j<s.count; ++j)
    {
        for (unsigned int i=0; i<s.ItemCount; ++i)
        {
              os << s.Data[i][j] << "\t";
        }

        os << std::endl;
    }

    return os;
}
//---------------------------------------------------------------------------
std::vector<std::vector<double> > CBaseSeries::GetVector(void)
{
    std::vector<std::vector<double> > vec;

    vec.resize(ItemCount);

    for (unsigned int i=0; i<ItemCount; ++i)
    {
        vec[i].resize(count);
    }

    for (unsigned int i=0; i<ItemCount; ++i)
    {
        for (unsigned int j=0; j<count; ++j)
        {
            vec[i][j] = Data[i][j];
        }
    }

    return vec;
}
//---------------------------------------------------------------------------
/*DataCollection CBaseSeries::GetSection(unsigned int start, unsigned int period)
{
    DataCollection section;

    section.resize(ItemCount);

    for (unsigned int i=0; i<ItemCount; ++i)
    {
        section[i].resize(period);
    }

    for (unsigned int i=0; i<ItemCount; ++i)
    {
        for (unsigned int j=0; j<period; ++j)
        {
            section[i][j] = Data[i][j+start];
        }
    }

    return section;
}
 */
