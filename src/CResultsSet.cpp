//---------------------------------------------------------------------------
#include <stdio.h>
#include <algorithm>
#include <cmath>

#include "CResultsSet.h"
#include "Constant.h"

//---------------------------------------------------------------------------
CResultsSet::CResultsSet()
{
}
//---------------------------------------------------------------------------
CResultsSet::CResultsSet(unsigned int _index, CParSet *ParSet, std::vector<unsigned int>& yl, int _item, int _land, std::string _name, int _grain) :
    Count(_index),
    ItemCount(_item),
    LandCount(_land),
    Name(_name),
    GrainCount(_grain)
{
    yearsList = yl;
    startDate = ParSet->StartDate;
    statsColumnCount = 5;

    Setup(Count, LandCount, Name, GrainCount);
}
//---------------------------------------------------------------------------
void CResultsSet::Setup(unsigned int Count, int LandCount, std::string& Name, int GrainCount)
{
    pcents.clear();

    pcents.push_back(99);
    pcents.push_back(95);
    pcents.push_back(90);
    pcents.push_back(80);
    pcents.push_back(70);
    pcents.push_back(60);
    pcents.push_back(50);
    pcents.push_back(40);
    pcents.push_back(30);
    pcents.push_back(20);
    pcents.push_back(10);
    pcents.push_back(5);
    pcents.push_back(1);

    names.clear();
    names.resize(ItemCount);

    names[0] = "Flow";
    names[1] = "Nitrate";
    names[2] = "Ammonium";
    names[3] = "Volume";

    r2.resize(41);
    rmse.resize(41);
    re.resize(41);
    ns.resize(41);
    n.resize(41);

    CellBal.resize(LandCount);
    CellWBal.resize(LandCount);
    RiverWBal.resize(5);
    RiverNO3Bal.resize(5);
    RiverNH4Bal.resize(5);

    Loads.Setup(LandCount, 22, Name);

    ReachInit.resize(NUM_RIVER_EQN);
    CellInit.Setup(LandCount, (NUM_CELL_EQN+1), Name);

    Land.clear();
    CellBalance.clear();


    for (int i=0; i<LandCount; i++)
    {
        CBaseSeries cb1(20, Count, Name);
        Land.push_back(cb1);

        CBaseSeries cb2(NUM_CELL_EQN, Count, Name);
        CellBalance.push_back(cb2);

        CellBal[i].resize(8);
        CellWBal[i].resize(10);
    }

    Direct.clear();
    Soil.clear();
    Ground.clear();

    for (int i=0; i<5; i++)
    {
        CBaseSeries cb(LandCount, Count, Name);
        Direct.push_back(cb);
    }

    for (int i=0; i<12; i++)
    {
        CBaseSeries cb1(LandCount, Count, Name);
        Soil.push_back(cb1);
    }

    for (int i=0; i<5; i++)
    {
        CBaseSeries cb2(LandCount, Count, Name);
        Ground.push_back(cb2);
    }

    for (int i=0; i<10; i++)
    {
        CBaseSeries cb(LandCount, Count, Name);
        Hydrology.push_back(cb);
    }

    RiverBalance.clear();
    Daily.clear();

    for (int i=0; i<GrainCount+1; i++)
    {
        CBaseSeries cb1(NUM_RIVER_EQN, Count, Name);
        RiverBalance.push_back(cb1);

        CBaseSeries cb2(12, Count, Name);
        Daily.push_back(cb2);
    }

    ResetErrorMatrix();

//    FindYears();
    unsigned int statsSize = yearsList.size() + 13;

    mean.resize(statsSize);
    stddev.resize(statsSize);
    dist.resize(statsSize);
    min.resize(statsSize);
    max.resize(statsSize);
    sum.resize(statsSize);
    square.resize(statsSize);
    range.resize(statsSize);
    variance.resize(statsSize);
    stderror.resize(statsSize);
    p95.resize(statsSize);
    median.resize(statsSize);

    for (unsigned int i=0; i<statsSize; ++i)
    {
        mean[i].resize(statsColumnCount);
        stddev[i].resize(statsColumnCount);
        dist[i].resize(statsColumnCount);
        min[i].resize(statsColumnCount);
        max[i].resize(statsColumnCount);
        sum[i].resize(statsColumnCount);
        square[i].resize(statsColumnCount);
        range[i].resize(statsColumnCount);
        variance[i].resize(statsColumnCount);
        stderror[i].resize(statsColumnCount);
        p95[i].resize(statsColumnCount);
        median[i].resize(statsColumnCount);
    }
}
//---------------------------------------------------------------------------
void CResultsSet::ResetErrorMatrix(void)
{
    for (int i=0; i<41; i++)
    {
        r2[i] = -999.0;
        rmse[i] = -999.0;
        re[i] = -999.0;
        ns[i] = -999.0;
        n[i] = -999.0;
    }
}
//---------------------------------------------------------------------------
void CResultsSet::Init(double val)
{
    for (int i=0; i<GrainCount+1; i++)
    {
        Daily[i].Init(val);
        RiverBalance[i].Init(val);
    }

    Loads.Init(val);

    for (int i=0; i<LandCount; i++)
    {
        Land[i].Init(val);
        CellBalance[i].Init(val);
    }

    for (int i=0; i<ItemCount; i++)
    {
        Direct[i].Init(val);
    }

    for (int i=0; i<6; i++)
    {
        Soil[i].Init(val);
    }

    for (int i=0; i<5; i++)
    {
        Ground[i].Init(val);
    }

    for (int i=0; i<10; i++)
    {
        Hydrology[i].Init(val);
    }
}
//---------------------------------------------------------------------------
//CResultsSet::~CResultsSet()
//{
//}
//---------------------------------------------------------------------------
std::vector<std::string> CResultsSet::TimeSeriesNames(void)
{
    return names;
}
//---------------------------------------------------------------------------
void CResultsSet::CalcStats(void)
{
    CalcPeriodStats();
    CalcMonthlyStats();
    CalcAnnualStats();
    CalcP();
}
//---------------------------------------------------------------------------
void CResultsSet::CalcPeriodStats(void)
{
    Stats(Daily[0].Data, 0);
}
//---------------------------------------------------------------------------
void CResultsSet::CalcMonthlyStats(void)
{
    std::vector<std::vector<float> > vec;

    vec.resize(statsColumnCount);

    for (unsigned short month=1; month<13; ++month)
    {
        dateCl::Date d = startDate;

        for (std::size_t i=0; i<statsColumnCount; ++i)
        {
            vec[i].clear();
        }

        for (unsigned int step=0; step<Count; ++step)
        {
            if (month == d.NMonth())
            {
                for (std::size_t k=0; k<statsColumnCount; ++k)
                {
                    vec[k].push_back(Daily[0].Data[k][step]);
                }
            }

            ++d;
        }

        Stats(vec, month);
    }
}
//---------------------------------------------------------------------------
void CResultsSet::CalcAnnualStats(void)
{
    std::vector<std::vector<float> > vec;
    vec.resize(statsColumnCount);

    unsigned int start = 0;
    unsigned int end = DaysInYear(startDate.NYear4()) - startDate.DOY();

    for (unsigned int year=0; year<yearsList.size(); ++year)
    {
        for (std::size_t i=0; i<statsColumnCount; ++i)
        {
            vec[i].clear();
        }

        for (unsigned int step=start; step<=end; ++step)
        {
            for (std::size_t k=0; k<statsColumnCount; ++k)
            {
                vec[k].push_back(Daily[NUM_GRAIN_SIZES].Data[k][step]);
            }
        }

        Stats(vec, year+13);

        start = end + 1;
        if ((year+1) < yearsList.size())
        {
            end += DaysInYear(yearsList[year+1]);
            if (end >= Count) end = (Count - 1);
        }
    }
}
//---------------------------------------------------------------------------
/*void CResultsSet::GetStartEnd(unsigned int year, int& start, int& end)
{
    int startYear = startDate.NYear4();
    int extraYearCount = year - startYear;

    start = 0;
    end = startDate.EOY() - startDate + 1;

    if (year != startYear)
    {
        for (int yr = 0; yr < extraYearCount; yr++)
        {
            end += DaysInYear(++startYear);
        }

        start = end - DaysInYear(year);
    }
}*/
//---------------------------------------------------------------------------
int CResultsSet::DaysInYear(int year)
{
    dateCl::Date date(1, 1, year);
    int days = date.EOY() - date.BOY() + 1;

    return days;
}
//---------------------------------------------------------------------------
void CResultsSet::Stats(std::vector<std::vector<float> > vec, unsigned int index)
{
    for (std::size_t j=0; j<statsColumnCount; j++)
    {
        min[index][j] = 9999.0;
        max[index][j] = 0.0;
        sum[index][j] = 0.0;
        square[index][j] = 0.0;

        if (vec[j].size() > statsColumnCount)
        {
        for (unsigned int i=0; i<vec[j].size(); i++)
        {
            if (vec[j][i] < min[index][j])
            {
                min[index][j] = vec[j][i];
            }
            if (vec[j][i] > max[index][j])
            {
                max[index][j] = vec[j][i];
            }

            sum[index][j] += vec[j][i];
            square[index][j] += vec[j][i] * vec[j][i];
        }

            if (sum[index][j] > 0)
            {
                range[index][j] = max[index][j] - min[index][j];
                mean[index][j] = sum[index][j] / vec[j].size();

                if (square[index][j] > 0)
                {
	                variance[index][j] = (square[index][j] - vec[j].size() * mean[index][j] * mean[index][j]) / (vec[j].size() - 1);
	                stddev[index][j] = std::sqrt(std::fabs(variance[index][j]));
		            stderror[index][j] = stddev[index][j] / std::sqrt(vec[j].size());
                    p95[index][j] = mean[index][j] + 1.96 * stddev[index][j];
                }
            }

        std::vector<float> med = vec[j];
        std::sort(med.begin(), med.end());

        unsigned int num = med.size() / 2;

        if (med.size() % 2 == 0)
        {
		    median[index][j] = (med[num - 1] + med[num]) / 2.0;
        }
        else
        {
		    median[index][j] = med[num];
        }
        }
    }
}
//---------------------------------------------------------------------------
void CResultsSet::CalcP(void)
{
    for (unsigned int i=0; i<dist.size(); ++i)
    {
        dist[i].clear();
    }
    dist.clear();

    dist.resize(pcents.size());
    for (unsigned int i=0; i<pcents.size(); ++i)
    {
        dist[i].resize(statsColumnCount);
    }

    std::vector< std::vector<double> > vec;
    vec.resize(statsColumnCount);

    for (std::size_t j=0; j<statsColumnCount; ++j)
    {
        vec[j].resize(Count);

        for (unsigned int k=0; k<Count; ++k)
        {
            vec[j][k] = Daily[0].Data[j][k];
        }

        std::sort(vec[j].begin(), vec[j].end());

        for (unsigned int p=0; p<pcents.size(); ++p)
        {
            dist[p][j] = CalcPercentile(vec[j], pcents[p]);
        }
    }
}
//---------------------------------------------------------------------------
double CResultsSet::CalcPercentile(const std::vector<double>& v,
                                            unsigned int p)
{
    double i = (double(v.size()) * double(p) / 100.0 + 0.5) - 1.0;

    if (int(i * 10.0) > (int(i) * 10))
    {
        int k = int(i);
        double f = i - double(k);

        return (1.0 - f) * v[k] + f * v[k+1];
    }

    return v[int(i)];
}
//---------------------------------------------------------------------------


