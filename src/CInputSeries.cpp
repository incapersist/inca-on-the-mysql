//---------------------------------------------------------------------------
#include "CInputSeries.h"

#define _USE_MATH_DEFINES

#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <sstream>

//---------------------------------------------------------------------------
CInputSeries::CInputSeries(unsigned int _index, unsigned int _item, int _reaches, std::string _name) :
    ReachCount(_reaches)
{
    Setup(_item, _index, _name);
}
/*
//---------------------------------------------------------------------------
CInputSeries::~CInputSeries()
{
    clear();
}
//---------------------------------------------------------------------------
CInputSeries::CInputSeries(const CInputSeries& rhs)
{
    copy(rhs);
}
//---------------------------------------------------------------------------
CInputSeries& CInputSeries::operator=(const CInputSeries& rhs)
{
    if (&rhs != this)
    {
        copy(rhs);
    }

    return *this;
}
//---------------------------------------------------------------------------
void CInputSeries::clear(void)
{
    for (unsigned int i=0; i<ItemCount; i++)
    {
        delete[] Data[i];
    }
    delete[] Data;

    Data = NULL;
}
//---------------------------------------------------------------------------
void CInputSeries::copy(const CInputSeries& rhs)
{
    Setup(rhs.ItemCount, rhs.count, rhs.name);

    for (unsigned int i=0; i<ItemCount; i++)
    {
        for (unsigned int j=0; j<count; j++)
        {
            Data[i][j] = rhs.Data[i][j];
        }
    }
}  */
//---------------------------------------------------------------------------
void CInputSeries::Setup(unsigned int _item, unsigned int _index,
                            std::string _name)
{
    ItemCount = _item;
    count = _index;
    name = _name;

    missing = false;
    loaded = false;

    try
    {
//        Data = new float *[ItemCount];
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
void CInputSeries::Init(double val)
{
    for (unsigned int i=0; i<ItemCount; i++)
    {
        for (unsigned int j=0; j<count; j++)
        {
            Data[i][j] = val;
        }
    }
}
//---------------------------------------------------------------------------
bool CInputSeries::LoadFromFile(const char* FileName)
{
    std::string fname(FileName);
    filename = fname;

    unsigned int cols = 0;

    std::ifstream infile(FileName);

    if (!infile.good()) return false;

    std::vector<std::string> fileStrings;
    std::string row;

    while (getline(infile, row))
    {
        fileStrings.push_back(row);
    }

    if (fileStrings.size() < count) return false;

    cols = GetWords(fileStrings[0]).size();

    if (cols == 2 || cols == 3)
    {
        loadNewHydrology(cols, fileStrings);
        _useNewHydrology = true;
    }
    else if (cols == 4 || cols == 5)
    {
        loadOldHydrology(cols, fileStrings);
        _useNewHydrology = false;
    }
    else
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool CInputSeries::LoadFromStrings(const std::vector<std::string>& fileStrings,
                                    unsigned int offset)
{
    unsigned int cols = GetWords(fileStrings[offset]).size();

    if (cols == 2 || cols == 3)
    {
        loadNewHydrology(cols, fileStrings, offset);
        _useNewHydrology = true;
    }
    else if (cols == 4 || cols == 5)
    {
        loadOldHydrology(cols, fileStrings, offset);
        _useNewHydrology = false;
    }
    else
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
void CInputSeries::loadOldHydrology(unsigned int cols,
                                    const std::vector<std::string>& fileStrings,
                                    unsigned int offset)
{
    for (unsigned int j=0; j<count; ++j)
    {
        std::vector<float> words = GetWords(fileStrings[j+offset]);

        if (words.size() <= ItemCount)
        {
            for (unsigned int column=0; column<words.size(); ++column)
            {
                Data[column][j] = words[column];
            }
        }
    }

    missing = (cols != ItemCount);
}
//---------------------------------------------------------------------------
void CInputSeries::loadNewHydrology(unsigned int cols,
                                    const std::vector<std::string>& fileStrings,
                                    unsigned int offset)
{
    for (unsigned int j=0; j<count; ++j)
    {
        std::vector<float> words = GetWords(fileStrings[j+offset]);

        if (words.size() <= 3)
        {
            for (unsigned int column=0; column<words.size(); ++column)
            {
                Data[column][j] = words[column];
                switch(column)
                {
                    case 0: Data[3][j] = words[column];
                            break;
                    case 1: Data[2][j] = words[column];
                            break;
                    case 2: Data[4][j] = words[column];
                            break;
                }
            }
        }
    }

    missing = (cols != 3);
}
//---------------------------------------------------------------------------
std::vector<float> CInputSeries::GetWords(const std::string& row)
{
    std::vector<float> words;
    float value;

    std::istringstream iss(row);

    while(iss >> value)
    {
        words.push_back(value);
    }

    return words;
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const CInputSeries& s)
{
    for (unsigned int j=0; j<s.count; ++j)
    {
        if (s._useNewHydrology)
        {
            os << s.Data[3][j] << "\t";
            os << s.Data[2][j] << "\t";

            for (unsigned int i=4; i<s.ItemCount; ++i)
            {
                os << s.Data[i][j] << "\t";
            }
        }
        else
        {
            for (unsigned int i=0; i<s.ItemCount; ++i)
            {
                os << s.Data[i][j] << "\t";
            }
        }

        os << std::endl;
    }

    return os;
}
//---------------------------------------------------------------------------
/*void CInputSeries::InsertRunupPeriod(int period)
{
    DataCollection section = GetSection(1, period);

    for (unsigned int i=0; i<ItemCount; ++i)
    {
        for (unsigned int j=0; j<section[i].size(); ++j)
        {
            Data[i].insert(Data[i].begin(), section[i][j]);
        }

    }
}           */
//---------------------------------------------------------------------------
std::string CInputSeries::SpatialReaches(void)
{
    std::ostringstream str;

    for (unsigned int i=0; i<Reaches.size(); ++i)
    {
        str << Reaches[i];
        if (i < Reaches.size() - 1) str << ",";
    }

    return str.str();
}
//---------------------------------------------------------------------------
void CInputSeries::SetReaches(std::string list)
{
    char ctmp[255];
    unsigned int index = 0;
    bool inDelimit = false;

    for (unsigned int j=0; j<list.size(); ++j)
    {
        if(list[j] != '\"' && list[j] != '\n')
        {
            if (list[j] != ',' || inDelimit)
            {
                ctmp[index] = list[j];
                ++index;
            }
            else
            {
                std::string id;
                id.assign(ctmp, index);
                Reaches.push_back(id);

                index = 0;
                inDelimit = false;
            }
        }
        else
        {
            inDelimit = !inDelimit;
        }
    }

    std::string id;
    id.assign(ctmp, index);
    Reaches.push_back(id);
}
//---------------------------------------------------------------------------

void CInputSeries::CalcSolar(double latitude, double longitude, dateCl::Date date)
{
    for (unsigned int i=0; i<count; ++i)
    {
        double solar = 0.0;
        double count = 0.0;

        for (int j=0; j<48; ++j)
        {
            solar += SolarRad(latitude, longitude, date, count);
            date += (1.0 / 48.0);
        }

        Data[4][i] = solar / count;
//        Solar[i] = solar / 48.0;
    }
}
//---------------------------------------------------------------------------

double CInputSeries::SolarRad(double latitude, double longitude,
                                     dateCl::Date& date, double& count)
{
    SolarNoon(latitude, longitude, date);

    int hour, min, sec;
    date.decodeTime(hour, min, sec);
    double time = hour * 60.0 + min;            // time fraction of date;
    double julianDay = date.DOY() - 1;     // Julian Day of date

    double fact = 0.0;
    if (time > sunrise && time < sunset)
    {
        fact = 1.0;
        ++count;
    }

    double houra = 360.0 * time / (24.0 * 60.0);

    double dec2 = 360.0 * julianDay / DaysInYear(date);
    double dec = 0.39637 - 22.9133 * cosd(dec2) + 4.02543 * sind(dec2)
                    - 0.3872 * cosd(2.0 * dec2) + 0.052 * sind(2.0 * dec2);
    double solelv = sind(latitude) * sind(dec) - cosd(latitude)
                    * cosd(dec) * cosd(houra);

    if (solelv <= 0.005) solelv = 0.005;

// reflection coefficient calculated from fig 5.1 (Monteith 1973), and
// expression for clear sky radiation calculated from Bras (pg 35, 1990)
// - note require solelv in degrees not radians

//    double refl = 0.3703 * exp(-0.0875 * solelv) + 0.05;

    double am = 1.0 / solelv;
    double aa = 0.128 - 0.054 * log10(am);
    double ae = exp(-3.0 * aa * am);
//    double visfrac = 0.5;
//    double srad = 1378.0 * (1.0 - refl) * visfrac * solelv * ae * fact;
    double srad = 1378.0 * solelv * ae * fact;

    return srad;
}
//---------------------------------------------------------------------------

bool CInputSeries::SolarNoon(double latitude, double longitude,
                        dateCl::Date& date)
{
    int daysInYear = DaysInYear(date);
    double julianDay = date.DOY() - 1;     // Julian Day of date

    double gamma = 2.0 * M_PI * (julianDay - 1.0) / daysInYear;

    double eqtime = 229.18 * (0.000075 + 0.001868 * cos(gamma)
                    - 0.032077 * sin(gamma) - 0.014615 * cos(2.0 * gamma)
                    - 0.040849 * sin(2.0 * gamma));

    double decl = 0.006918 - 0.399912 * cos(gamma) + 0.070257 * sin(gamma)
                    - 0.006758 * cos(2.0 * gamma) + 0.000907 * sin(2.0 * gamma)
                    - 0.002697 * cos(3.0 * gamma) + 0.00148 * sin (3.0 * gamma);

    double tmp = (cosd(90.833) / (cosd(latitude)
                    * cos(decl))) - tand(latitude)
                    * tan(decl);

    // At higher latitudes, tmp can be < -1, which would be outside the
    // allowable range for ACOS
    tmp = (std::max)(tmp, -1.0);

    double ha =(acos(tmp)) * (180.0 / M_PI);

    // Calculate true dawn and sunset
    sunrise = 720.0 + 4.0 * (longitude - ha) - eqtime;
    sunset = 720.0 + 4.0 * (longitude + ha) - eqtime;

    return true;
}
//---------------------------------------------------------------------------
double CInputSeries::cosd(double degrees)
{
    double radians = degrees * M_PI / 180.0;
    return cos(radians);
}
//---------------------------------------------------------------------------

double CInputSeries::sind(double degrees)
{
    double radians = degrees * M_PI / 180.0;
    return sin(radians);
}
//---------------------------------------------------------------------------
double CInputSeries::tand(double degrees)
{
    double radians = degrees * M_PI / 180.0;
    return tan(radians);
}
//---------------------------------------------------------------------------
double CInputSeries::asind(double num)
{
    return asin(num) * 180.0 / M_PI;
}
//---------------------------------------------------------------------------

double CInputSeries::DaysInYear(dateCl::Date& date)
{
    dateCl::Date start = date.BOY();
    dateCl::Date end = date.EOY();
    long days = end - start + 1;

    return (double) days;
}

//---------------------------------------------------------------------------

