//---------------------------------------------------------------------------
#include <math.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>

#include "Model.h"
#include "Constant.h"
#include "Eqn.h"
#include "Integrat.h"
#include "DATECL.h"
#include "LandModel.h"
#include "ReachModel.h"

//---------------------------------------------------------------------------
CModel::CModel(CParSet& parset)
{
    ParSet = parset;

    numLand = ParSet.LandCount();

    init.resize(4);

    flow.resize(ParSet.TimeSteps);
    volume.resize(ParSet.TimeSteps);
    diffuseNO3.resize(ParSet.TimeSteps);
    diffuseNH4.resize(ParSet.TimeSteps);
    diffuseDON.resize(ParSet.TimeSteps);
    abstractions.resize(ParSet.TimeSteps);

    reachInputs.resize(5);

    for (unsigned int i=0; i<5; i++)
    {
        reachInputs[i].resize(ParSet.TimeSteps);
    }

    depInput.resize(2);

    for (int i=0; i<2; i++)
    {
        depInput[i].resize(ParSet.TimeSteps);
    }
}
//---------------------------------------------------------------------------
bool CModel::Run(CResultsGroup *Results,
                    CFertSeries *NO3Fert,
                    CFertSeries *NH4Fert,
                    CFertSeries *DONFert,
                    CDepSeries *Dep,
                    std::vector<CEffSeries*>& EffluentSeriesList,
                    std::vector<CAbsSeries*>& AbstractionSeriesList,
                    bool& stopRun,
                    CInputSeriesContainer& InputSeriesList,
                    CLandSeries* LandPeriods,
                    CGrowthSeries* Growth,
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
                    int run)
{

    CBaseSeries *nFertPtr, *aFertPtr, *oFertPtr;
    CGrowthSeriesItem *growthPtr;

    // Sort out spatial reaches

    SpatialReaches.clear();

    if (InputSeriesList.size() > 1)
    {
        for (unsigned int i=0; i<InputSeriesList.size(); i++)
        {
            CInputSeries* InputSeries = InputSeriesList[i];

            for (unsigned int j=0; j<InputSeries->Reaches.size(); j++)
            {
                SpatialReaches[InputSeries->Reaches[j]] = i;
            }
        }
    }
    else
    {
        structure->SetDirection(CStructure::MainStemLast);
        structure->Reset();

        do
        {
            stream = structure->GetCurrentStream();

            do
            {
                SpatialReaches[stream.GetCurrentReachID()] = 0;
            }
            while (stream.NextReach());
        }
        while (structure->NextStream());
    }

	unsigned int reachCount = 0;

    structure->SetDirection(CStructure::MainStemLast);
    structure->Reset();

    do
    {
        stream = structure->GetCurrentStream();

        bool isTopReach = true;



        /**********************************************************
        *           START OF SUB-CATCHMENT / REACH LOOP           *
        **********************************************************/
        do
        {
            resetModelProgress();




            reachKey = stream.GetCurrentReachID();



            // Get the correct time series for this reach (allows
            // use of spatially distributed inputs).
            CInputSeries *InputSeries = InputSeriesList[SpatialReaches[reachKey]];
//            CInputSeries *InputSeries = InputSeriesList[reachKey];

            landInput.clear();
            CalcLandInputs(LandPeriods, useLand);

            depLoaded = false;
            CalcDepInputs(Dep, useDep);

            LandModel land(ParSet,
                            ParSet.SubPar[reachKey],
                            ParSet.ReachPar[reachKey],
                            Results->SubCatchRes[reachKey]);

            if (NO3Fert == NULL) nFertPtr = NULL;
            else nFertPtr = NO3Fert->Fert[reachKey];

            if (NH4Fert == NULL) aFertPtr = NULL;
            else aFertPtr = NH4Fert->Fert[reachKey];

            if (DONFert == NULL) oFertPtr = NULL;
            else oFertPtr = DONFert->Fert[reachKey];

            if (Growth == NULL) growthPtr = NULL;
            else growthPtr = &Growth->Growth[reachKey];

            land.Run(InputSeries,
                        nFertPtr, aFertPtr, oFertPtr, growthPtr,
                        precipOption,
                        flow, volume, diffuseNO3, diffuseNH4, diffuseDON,
                        landInput, depInput, depLoaded,
                        useSnowPack,
                        useCoup,
                        useNo3Fert,
                        useNh4Fert,
                        useDonFert,
                        useGrowth,
                        updateLandModelProgress,
						reachCount
            );

            // Calculate annual loads
            LoadCalc(*(Results->SubCatchRes[reachKey]));



            CalcReachInputs(EffluentSeriesList, useEff);
            StoreUpstreamInputs(Results);
            CalcReachOutputs(AbstractionSeriesList, useAbs);





 //           CRiverPar rp = stream.Pars();
			CRiverPar rp = ParSet.RiverPar;

            ReachModel reach(ParSet,
                                ParSet.SubPar[reachKey],
                                ParSet.ReachPar[reachKey],
                                rp,
                                Results->SubCatchRes[reachKey]);

			reach.Run(InputSeries,
                        abstractions,
                        reachInputs,
                        init,
                        updateReachModelProgress,
                        reachLog,
                        isTopReach,
						reachCount);


            init[0] = Results->SubCatchRes[reachKey]->Daily[0].Data[0][0];

            init[1] = Results->SubCatchRes[reachKey]->Daily[0].Data[1][0];
            init[2] = Results->SubCatchRes[reachKey]->Daily[0].Data[2][0];
            init[3] = Results->SubCatchRes[reachKey]->Daily[0].Data[4][0];

            isTopReach = false;

			reachCount++;


        // End reach / sub-catchment loop
        }
        while (stream.NextReach());

    }
    while (structure->NextStream());

    // Calculate mass balance check
    Results->CalcCellBal(ParSet);
    Results->CalcRiverBal(ParSet);
    Results->Available(true);

    Results->CalcStats();

    return true;
}

//---------------------------------------------------------------------------
void CModel::LoadCalc(CResultsSet& res)
{
    for (unsigned int LandUse=0; LandUse<numLand; ++LandUse)
    {
        res.Loads.Data[LandUse][0] = res.CellBalance[LandUse].Data[6][ParSet.TimeSteps-1];
        res.Loads.Data[LandUse][1] = res.CellBalance[LandUse].Data[7][ParSet.TimeSteps-1];

        for (int j=0; j<11; j++)
        {
            res.Loads.Data[LandUse][j+2] = res.CellBalance[LandUse].Data[j+10][ParSet.TimeSteps-1] / 100.0;
        }

        for (int j=28; j<NUM_CELL_EQN; j++)
        {
            res.Loads.Data[LandUse][j-15] = res.CellBalance[LandUse].Data[j][ParSet.TimeSteps-1] / 100.0;
        }
    }
}
//---------------------------------------------------------------------------
void CModel::StoreUpstreamInputs(CResultsGroup* res)
{
    std::vector<std::string> inputs = stream.GetCurrentReach().Inputs();
    std::string reach;

//    std::ofstream out("banana.csv");

    for (unsigned int j=0; j<inputs.size(); ++j)
    {
        reach = inputs[j];

//        res->SaveToFile("res", &ParSet, 5, 1, 0);

        CResultsSet* results = res->SubCatchRes[reach];

        for (int i=1; i<ParSet.TimeSteps; ++i)
        {
            reachInputs[0][i] += results->RiverBalance[0].Data[0][i];
//            reachInputs[0][i] += results->Daily[0].Data[0][i];
            reachInputs[1][i] += (results->RiverBalance[0].Data[4][i]
                                - results->RiverBalance[0].Data[4][i-1]);
            reachInputs[2][i] += (results->RiverBalance[0].Data[6][i]
                                - results->RiverBalance[0].Data[6][i-1]);
            reachInputs[4][i] += (results->RiverBalance[0].Data[16][i]
                                - results->RiverBalance[0].Data[16][i-1]);

//if (stream.GetCurrentReach().ID() == "18")
//{
//out << results->RiverBalance[0].Data[0][i] << ",";
//out << reachInputs[0][i] << ",";
//}

        }

//if (stream.GetCurrentReach().ID() == "18")
//{
//    out << std::endl;
//}

        reachInputs[0][0] += results->RiverBalance[0].Data[0][0];
//        reachInputs[0][0] += results->Daily[0].Data[0][0];
        reachInputs[1][0] += results->RiverBalance[0].Data[4][0];
        reachInputs[2][0] += results->RiverBalance[0].Data[6][0];
        reachInputs[4][0] += results->RiverBalance[0].Data[16][0];
    }

    out.close();
}

//---------------------------------------------------------------------------
void CModel::CalcReachInputs(std::vector<CEffSeries*>& EffluentSeriesList, bool useEff)
{
    DiffuseInputs();
    EffluentInputs(EffluentSeriesList, useEff);
}

//---------------------------------------------------------------------------
void CModel::DiffuseInputs(void)
{
    for (int TimeStep=0; TimeStep<ParSet.TimeSteps; TimeStep++)
    {
        reachInputs[0][TimeStep] = flow[TimeStep];
        reachInputs[1][TimeStep] = diffuseNO3[TimeStep];
        reachInputs[2][TimeStep] = diffuseNH4[TimeStep];
        reachInputs[3][TimeStep] = volume[TimeStep];
        reachInputs[4][TimeStep] = diffuseDON[TimeStep];
    }
}
//---------------------------------------------------------------------------
void CModel::EffluentInputs(std::vector<CEffSeries*>& EffluentSeriesList, bool useEff)
{
    bool TimeSeriesPresent = false;

    // If an effluent time series is loaded for this reach,
    // convert the specified concentrations to masses and add
    // to the input for this reach.
    if (EffluentSeriesList.size() && useEff)
    {
        for (unsigned int i=0; i<EffluentSeriesList.size(); ++i)
        {
            CEffSeries* Eff = EffluentSeriesList[i];
            effIter it = Eff->Effluent.find(reachKey);

            if (it != Eff->Effluent.end())
            {
                TimeSeriesPresent = true;

                for (int TimeStep=0; TimeStep<ParSet.TimeSteps; TimeStep++)
                {
                    reachInputs[0][TimeStep] += Eff->Effluent[reachKey]->Data[0][TimeStep];
                    reachInputs[1][TimeStep] += Eff->Effluent[reachKey]->Data[1][TimeStep]
                                                            * Eff->Effluent[reachKey]->Data[0][TimeStep]
                                                            * 86.4;
                    reachInputs[2][TimeStep] += Eff->Effluent[reachKey]->Data[2][TimeStep]
                                                            * Eff->Effluent[reachKey]->Data[0][TimeStep]
                                                            * 86.4;
                    reachInputs[4][TimeStep] += Eff->Effluent[reachKey]->Data[3][TimeStep]
                                                            * Eff->Effluent[reachKey]->Data[0][TimeStep]
                                                            * 86.4;
                }
            }
        }
    }

    // Otherwise just take the concentrations specified in the
    // parameter file, convert them to a mass and add to the input.
    if (!TimeSeriesPresent && ParSet.ReachPar[reachKey].Effluent)
    {
        for (int TimeStep=0; TimeStep<ParSet.TimeSteps; TimeStep++)
        {
            reachInputs[0][TimeStep] += ParSet.ReachPar[reachKey].Par[7];
            reachInputs[1][TimeStep] += ParSet.ReachPar[reachKey].Par[5]
                                        * ParSet.ReachPar[reachKey].Par[7]
                                                * 86.4;
            reachInputs[2][TimeStep] += ParSet.ReachPar[reachKey].Par[6]
                                        * ParSet.ReachPar[reachKey].Par[7]
                                        * 86.4;
            reachInputs[4][TimeStep] += ParSet.ReachPar[reachKey].Par[18]
                                        * ParSet.ReachPar[reachKey].Par[7]
                                        * 86.4;
        }
    }
}
//---------------------------------------------------------------------------
void CModel::CalcReachOutputs(std::vector<CAbsSeries*>& AbstractionSeriesList, bool useAbs)
{
    for (int TimeStep=0; TimeStep<ParSet.TimeSteps; TimeStep++)
    {
        abstractions[TimeStep] = 0.0;
    }

    bool AbsTimeSeriesPresent = false;

    // If an effluent time series is loaded for this reach,
    // convert the specified concentrations to masses and add
    // to the input for this reach.
    if (AbstractionSeriesList.size() > 0 && useAbs)
    {
        for (unsigned int i=0; i<AbstractionSeriesList.size(); ++i)
        {
            CAbsSeries* Abs = AbstractionSeriesList[i];
            absIter it = Abs->Effluent.find(reachKey);

            if (it != Abs->Effluent.end())
            {
                AbsTimeSeriesPresent = true;

                for (int TimeStep=0; TimeStep<ParSet.TimeSteps; TimeStep++)
                {
                    abstractions[TimeStep] += Abs->Effluent[reachKey].Data[0][TimeStep];
                }
            }
        }
    }

    // Otherwise just take the concentrations specified in the
    // parameter file, convert them to a mass and add to the input.
    if (!AbsTimeSeriesPresent && ParSet.ReachPar[reachKey].Par[8] > 0.0)
    {
        for (int TimeStep=0; TimeStep<ParSet.TimeSteps; TimeStep++)
        {
            abstractions[TimeStep] = ParSet.ReachPar[reachKey].Par[8];
        }
    }
}
//---------------------------------------------------------------------------
void CModel::CalcLandInputs(CLandSeries* LandPeriods, bool useLand)
{
    landInput.clear();

    if (LandPeriods != NULL && useLand)
    {
        if (LandPeriods->HasReach(reachKey))
        {
            landInput = LandPeriods->GetReachPeriods(reachKey);
        }
    }
}

//---------------------------------------------------------------------------
void CModel::CalcDepInputs(CDepSeries* Dep, bool useDep)
{
    if (Dep != NULL)
    {
        depIter it = Dep->Deposition.find(reachKey);

        if (it != Dep->Deposition.end() && useDep)
        {
            depLoaded = true;

            for (int TimeStep=0; TimeStep<ParSet.TimeSteps; TimeStep++)
            {
                depInput[0][TimeStep] = Dep->Deposition[reachKey]->Data[0][TimeStep]
                                        + Dep->Deposition[reachKey]->Data[1][TimeStep];
                depInput[1][TimeStep] = Dep->Deposition[reachKey]->Data[2][TimeStep]
                                        + Dep->Deposition[reachKey]->Data[3][TimeStep];
            }
        }
    }
}





