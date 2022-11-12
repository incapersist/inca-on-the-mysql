#ifndef ReachModelH
#define ReachModelH

#include <vector>

#include "CParSet.h"
#include "CResultsGroup.h"
#include "CInputSeries.h"
#include "CEffSeries.h"
#include "CAbsSeries.h"

class ReachModel
{
public:
    ReachModel(CParSet&, CSubPar&, CReachPar&, CRiverPar&, CResultsSet*);

    bool Run(CInputSeries* InputSeries,
                    std::vector<float>& abstractions,
                    std::vector< std::vector<float> >&,
                    std::vector<float>&,
                    void(* updateReachModelProgress)(float),
                    std::vector<std::string>& log,
                    bool,
					unsigned int reachCount);

private:
    double CalcTimeConstant(double);
    void CalcRiverConc(double);
    void StoreRiverResults(double);

    CParSet ParSet;
    CReachPar reachPar;
    CSubPar subPar;
    CRiverPar riverPar;
    int reaches;
    std::vector<double> Inputs, Outputs, Conc;
    std::vector<double> rinit;
    std::vector<double> wk;
    unsigned int runProgressCount;
    int TimeStep;
    CResultsSet *results;

};

#endif