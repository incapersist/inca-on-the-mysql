//---------------------------------------------------------------------------
#ifndef CResultsSetH
#define CResultsSetH

#include <string>
#include <cstddef>

#include "CBaseSeries.h"
#include "CParSet.h"

//---------------------------------------------------------------------------

class CResultsSet
{
public:
    CResultsSet();
    CResultsSet(unsigned int _index,
                CParSet *ParSet,
                std::vector<unsigned int>& yearsList,
                int _item = 4,
                int _land = 6,
                std::string _name = "",
                int _grain = 5);

    std::vector<std::string> TimeSeriesNames(void);
    void Init(double val);
    void CalcStats(void);
    const std::vector<unsigned int>& YearList(void) { return yearsList; }
    void ResetErrorMatrix(void);

	unsigned int Count;
	int ItemCount, LandCount;
	std::string Name;
	int Reaches, GrainCount;
    CBaseSeries Loads;
    CBaseSeries CellInit;

    std::vector<CBaseSeries> Direct;
    std::vector<CBaseSeries> Soil;
    std::vector<CBaseSeries> Ground;
    std::vector<CBaseSeries> Daily;
    std::vector<CBaseSeries> RiverBalance;
    std::vector<CBaseSeries> CellBalance;
    std::vector<CBaseSeries> Land;
    std::vector<CBaseSeries> Hydrology;

    std::vector<unsigned int> pcents;
    std::vector< std::vector<double> > mean, stddev, dist;
    std::vector< std::vector<double> > min, max, sum, square, median;
    std::vector< std::vector<double> > range, variance, stderror, p95;

    std::vector<std::string> names;
    std::vector<double> mn, r2, rmse, re, ns, n;
    std::vector<double> RiverWBal, RiverNO3Bal, RiverNH4Bal;
    std::vector< std::vector<double> > CellBal, CellWBal;
    std::vector<double> ReachInit;

private:
    void Setup(unsigned int, int, std::string&, int);
    void CalcP(void);
    void CalcPeriodStats(void);
    void CalcMonthlyStats(void);
    void CalcAnnualStats(void);
    void Stats(std::vector<std::vector<float> >, unsigned int);
    double CalcPercentile(const std::vector<double>&, unsigned int);
    int DaysInYear(int year);
    std::size_t statsColumnCount;

    dateCl::Date startDate;
    std::vector<unsigned int> yearsList;
};

#endif
