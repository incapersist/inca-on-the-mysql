//---------------------------------------------------------------------------
#include <sstream>

#include "CRecord.h"

//---------------------------------------------------------------------------

CRecord::CRecord()
{
    SetComment('%');
}
//---------------------------------------------------------------------------

CRecord::CRecord(std::istream& infile)
{
    SetComment('%');
    read(infile);
}
//---------------------------------------------------------------------------

std::istream& CRecord::read(std::istream& file)
{
    rec.clear();

    while (getline(file, rec))
    {
        if (rec[0] != commentChar && rec != "")
        {
            splitColumns();
            break;
        }
    }

    return file;
}
//---------------------------------------------------------------------------

std::string& CRecord::str(void)
{
    return rec;
}
//---------------------------------------------------------------------------

void CRecord::SetComment(char ch)
{
    commentChar = ch;
}

//---------------------------------------------------------------------------
void CRecord::extract(double& num)
{
    std::istringstream istr(rec);
    istr >> num;
}

//---------------------------------------------------------------------------
double CRecord::extractCol(int col)
{
    double num;

    std::istringstream istr(cols[col-1]);
    istr >> num;

    return num;
}

//---------------------------------------------------------------------------
void CRecord::extract(int& num)
{
    std::istringstream istr(rec);
    istr >> num;
}

//---------------------------------------------------------------------------
void CRecord::extract(unsigned int& num)
{
    std::istringstream istr(rec);
    istr >> num;
}

//---------------------------------------------------------------------------
void CRecord::extract(unsigned short& num)
{
    std::istringstream istr(rec);
    istr >> num;
}

//---------------------------------------------------------------------------
void CRecord::extract(bool& num)
{
    std::istringstream istr(rec);
    istr >> num;
}
//---------------------------------------------------------------------------
void CRecord::splitColumns(void)
{
    std::istringstream istr(rec);
    std::string str;

    cols.clear();

    while (!istr.eof())
    {
        istr >> str;
        cols.push_back(str);
    }
}
//---------------------------------------------------------------------------
unsigned int CRecord::count(void)
{
    if (cols.size() == 0)
    {
        splitColumns();
    }

    return cols.size();
}
//---------------------------------------------------------------------------


