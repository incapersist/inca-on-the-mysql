#ifndef LandModelH
#define LandModelH

#define CS 1.1
#define LD 0.2

#include <vector>

#include "CParSet.h"
#include "CResultsGroup.h"
#include "CInputSeries.h"
#include "CFertSeries.h"
#include "CDepSeries.h"
#include "CGrowthSeries.h"
#include "CLandSeries.h"

class LandModel
{
public:
    LandModel(CParSet&,
                CSubPar&,
                CReachPar&,
                CResultsSet *);

    bool Run(CInputSeries*,
                    CBaseSeries *,
                    CBaseSeries *,
                    CBaseSeries *,
                    CGrowthSeriesItem*,
                    bool PrecipOption,
                    std::vector<float>&,
                    std::vector<float>&,
                    std::vector<float>&,
                    std::vector<float>&,
                    std::vector<float>&,
                    std::vector<CLandItem>&,
                    std::vector< std::vector<float> >&,
                    bool,
                    bool useSnowPack,
                    bool useCoup,
                    bool useNo3Fert,
                    bool useNh4Fert,
                    bool useDonFert,
                    bool useGrowth,
					void(* updateLandModelProgress)(float),
					unsigned int
    );


private:
    double VrCalc(double, int);
    bool NewYear(dateCl::Date&);
    double DaysInYear(dateCl::Date&);
    void SnowMelt(double&, double&, double, double, double, bool&);
    void DailyLoadCalc(CResultsSet *, int, int);
    double CalcSoilMass(CCellPar& pars);
void CalcPersist(CInputSeries* InputSeries,
                            CCellPar& pars,
                            int TimeStep,
                            int LandUse);
    CParSet ParSet;
    CSubPar subPar;
    CReachPar reachPar;
    CResultsSet *results;
    std::vector< std::vector<double> > luinit;
    unsigned int numLand;
    int reaches;

    std::vector<double> MaxUptake, snow, melt, mSoil;

    std::vector<double> lastStore;

    double SecondsInDay;

    CCellPar pars;

    double precSD, precFE, rain, rainAsCumecsPerm;

    std::vector<double> cinit, cinit2;

    std::vector<double> wk1, wk2;

    std::vector<double> Inputs, Conc, Outputs;

};


#endif