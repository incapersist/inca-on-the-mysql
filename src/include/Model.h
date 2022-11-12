#ifndef CModelH
#define CModelH

#include <vector>

#include "CParSet.h"
#include "CResultsGroup.h"
#include "CEffSeries.h"
#include "CFertSeries.h"
#include "CDepSeries.h"
#include "CLandSeries.h"
#include "CGrowthSeries.h"
#include "CInputSeries.h"
#include "CAbsSeries.h"
#include "CStructure.h"
#include "CInputSeriesContainer.h"


class CModel
{
public:
    CModel(CParSet&);

    bool Run(CResultsGroup *,
                    CFertSeries *,
                    CFertSeries *,
                    CFertSeries *,
                    CDepSeries *,
            std::vector<CEffSeries*>& EffluentSeriesList,
            std::vector<CAbsSeries*>& AbstractionSeriesList,
            bool&, CInputSeriesContainer&,
            CLandSeries* LandPeriods, CGrowthSeries* Growth,
            CStructure* structure,
            void(* resetModelProgress)(),
            void(* updateLandModelProgress)(float),
            void(* updateReachModelProgress)(float),
            bool precipOption,
            bool useSnowPack,
            bool useCoup,
            bool useNo3Fert,
            bool useNh4Fert,
            bool useDonFert,
            bool useGrowth,
            bool useLand,
            bool useDep,
            bool useEff,
            bool useAbs,
            std::vector<std::string>& reachLog,
            int run);


private:
typedef void (*CallbackType)();
    void LoadCalc(CResultsSet&);
    void StoreUpstreamInputs(CResultsGroup*);
    void CalcReachInputs(std::vector<CEffSeries*>&, bool useEff);
    void CalcReachOutputs(std::vector<CAbsSeries*>&, bool useAbs);
    void CalcLandInputs(CLandSeries*, bool useLand);
    void CalcDepInputs(CDepSeries*, bool useDep);
    void DiffuseInputs(void);
    void EffluentInputs(std::vector<CEffSeries*>&, bool useEff);

    std::vector<float> diffuseNO3, diffuseNH4, diffuseDON, flow, volume, init, abstractions;
    std::vector< std::vector<float> > reachInputs, depInput;

    std::vector<CLandItem> landInput;

    unsigned int numLand;
    std::string reachKey;
    std::map<std::string, int> SpatialReaches;
    bool depLoaded;

    std::ofstream out;

    CParSet ParSet;
    CStream stream;


};


#endif
