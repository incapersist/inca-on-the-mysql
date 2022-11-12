#define _USE_MATH_DEFINES

#include <algorithm>
#include <cmath>

#include "LandModel.h"
#include "Constant.h"
#include "Eqn.h"
#include "Integrat.h"

//---------------------------------------------------------------------------
LandModel::LandModel(CParSet& parset,
                        CSubPar& SubPar,
                        CReachPar& ReachPar,
                        CResultsSet *Results)
{
    ParSet = parset;
    subPar = SubPar;
    reachPar = ReachPar;
    results = Results;

    numLand = ParSet.LandCount();
    reaches = ParSet.Reaches;

    luinit.resize(numLand);


    lastStore.resize(numLand);
    MaxUptake.resize(numLand);
    snow.resize(numLand);

    melt.resize(numLand);
    mSoil.resize(numLand);

    Inputs.resize(22);
    Outputs.resize(17);
    Conc.resize(3);

    wk1.resize(1000);

    cinit.resize(NUM_CELL_EQN);


    for (unsigned int Landuse=0; Landuse<numLand; Landuse++)
    {
        luinit[Landuse].resize(NUM_CELL_EQN);
    }
}

//---------------------------------------------------------------------------
bool LandModel::Run(CInputSeries* InputSeries,
                    CBaseSeries *no3Fert,
                    CBaseSeries *nh4Fert,
                    CBaseSeries *donFert,
                    CGrowthSeriesItem* growth,
                    bool usePrecip,
                    std::vector<float>& flow,
                    std::vector<float>& volume,
                    std::vector<float>& diffuseNO3,
                    std::vector<float>& diffuseNH4,
                    std::vector<float>& diffuseDON,
                    std::vector<CLandItem>& landInput,
                    std::vector< std::vector<float> >& depInput,
                    bool depLoaded,
                    bool useSnowPack,
                    bool useCoup,
                    bool useNo3Fert,
                    bool useNh4Fert,
                    bool useDonFert,
                    bool useGrowth,
					void(* updateLandModelProgress)(float),
					unsigned int reachCount
                    )
{
    double totRain, Period, yrsz, VrInitial;
    int StartDay;
    dateCl::Date CurrentDate, date;
    double nDryStorage, nDryInput, aDryStorage, aDryInput;
    double ndrydep, nwetdep, adrydep, awetdep;
    double tDiff, soiltemp, snowdepth, Da;
    std::vector<double> Land;
    bool InGrowthPeriod, useParameterFileLand;
    bool useNewHydrology = InputSeries->useNewHydrology();

    Land.resize(numLand);
    date = ParSet.StartDate;

    // Calculate start day (julian day) from start date
    dateCl::Date yearStart = date.BOY();
    StartDay = (int)(date.julDate() - yearStart.julDate());

    // Get the number of days in the year
    yrsz = DaysInYear(ParSet.StartDate);

    // Calculate a useful constant
    SecondsInDay = (3600.0 * 24.0);


        // Initialise total rainfall sum
        totRain = 0.0;

        // Set Period to be the whole year until we find out otherwise
        Period = yrsz;

        // Find out if the start date is the 1st January. If not,
        // deposition calculations for the first year in the run
        // period must be based on a part year.
        if (StartDay != 1)
        {
            Period -= StartDay + 1;
        }

        // Find out if the run period is shorter than one year.
        // If so, deposition calculations must be based on a part year.
        if (ParSet.TimeSteps < yrsz)
        {
            Period = ParSet.TimeSteps;
        }

        // Calculate total rainfall input
        for (int TimeStep=0; TimeStep<Period; TimeStep++)
        {
            totRain += InputSeries->Data[3][TimeStep];
        }

        // Set up initial conditions for the land phase model
        for (unsigned int LandUse=0; LandUse<numLand; LandUse++)
        {
            for (int i=0; i<NUM_CELL_EQN; i++)
            {
//                luinit[LandUse][i] = ParSet.CellPar[LandUse].Initial[i];
                luinit[LandUse][i] = 0.0;
            }


            double directDrainageVolume = ParSet.CellPar[LandUse].Initial[0]
                                            * ParSet.CellPar[LandUse].TC[0]
                                            * 86400.0;

            double soilDrainageVolume = 0.0;
            if (useNewHydrology)
            {
                soilDrainageVolume = 1000.0 * ParSet.CellPar[LandUse].Initial[9];
                VrInitial = 0.0;
            }
            else
            {
                soilDrainageVolume = ParSet.CellPar[LandUse].Initial[1]
                                        * ParSet.CellPar[LandUse].TC[1]
                                        * 86400.0;

                // Nitrogen initial conditions are expressed as concentrations
                // for user convenience, so convert these to loads.
                // Surface nitrate and ammonium first:
                VrInitial = VrCalc(InputSeries->Data[0][0], LandUse);
            }

            double groundDrainageVolume = subPar.Par[3]
                                            * subPar.Par[4]
                                            * 1000000.0;

            mSoil[LandUse] = CalcSoilMass(ParSet.CellPar[LandUse]);

            // direct runoff nitrate and ammonium
//            luinit[LandUse][22] = (ParSet->CellPar[LandUse].Initial[3]
//                                    * ParSet->CellPar[LandUse].Initial[9])
//                                    / 1000.0;
//            luinit[LandUse][23] = (ParSet->CellPar[LandUse].Initial[6]
//                                    * ParSet->CellPar[LandUse].Initial[9])
//                                    / 1000.0;
            luinit[LandUse][22] = (ParSet.CellPar[LandUse].Initial[2]
                                    * directDrainageVolume)
                                    / 1000.0;
            luinit[LandUse][23] = (ParSet.CellPar[LandUse].Initial[4]
                                    * directDrainageVolume)
                                    / 1000.0;
            luinit[LandUse][34] = (ParSet.CellPar[LandUse].Initial[8]
                                    * directDrainageVolume)
                                    / 1000.0;

            // surface nitrate and ammonium
            luinit[LandUse][2] = ParSet.CellPar[LandUse].Initial[3]
                                    * ((VrInitial
                                    + soilDrainageVolume)
                                    / 1000.0);
            luinit[LandUse][4] = ParSet.CellPar[LandUse].Initial[5]
                                    * ((VrInitial
                                    + soilDrainageVolume)
                                    / 1000.0);
            // ... then sub-surface nitrate and ammonium
            luinit[LandUse][3] = (subPar.Par[7]
                                    * groundDrainageVolume)
                                    / 1000.0;
            luinit[LandUse][5] = (subPar.Par[8]
                                    * groundDrainageVolume)
                                    / 1000.0;

            // Organic N
            luinit[LandUse][25] = ParSet.CellPar[LandUse].Initial[6]
                                    * ((VrInitial + soilDrainageVolume)
                                    / 1000.0);
            luinit[LandUse][26] = ParSet.CellPar[LandUse].Initial[7]
                                    * mSoil[LandUse]
                                    * 1.0e-6;
            luinit[LandUse][27] = (subPar.Par[11]
                                    * groundDrainageVolume)
                                    / 1000.0;

            luinit[LandUse][0] = ParSet.CellPar[LandUse].Initial[1];
            luinit[LandUse][1] = subPar.Par[6];
            luinit[LandUse][6] = soilDrainageVolume;
            luinit[LandUse][7] = groundDrainageVolume;
            luinit[LandUse][21] = ParSet.CellPar[LandUse].Initial[0];
            luinit[LandUse][24] = directDrainageVolume;

            luinit[LandUse][35] = ParSet.CellPar[LandUse].Initial[9];
   //         luinit[LandUse][36] = ParSet.CellPar[LandUse].Initial[1];
                                  //  * 86.4;

            for (int i=8; i<21; i++)
            {
                luinit[LandUse][i] = 0.0;
            }

            // Initial snowpack depth as water equivalent
            snow[LandUse] = ParSet.CellPar[LandUse].Par[24];
            melt[LandUse] = 0.0;

            for (int i=0; i<NUM_CELL_EQN; i++)
            {
                results->CellInit.Data[LandUse][i] = luinit[LandUse][i];
            }

            results->CellInit.Data[LandUse][NUM_CELL_EQN] = VrInitial;
        }

        // Dry deposition is stored on vegetation until a rainfall event.
        // Reset this store at the first time step for each new sub-catchment.
        nDryStorage = 0.0;
        aDryStorage = 0.0;

        // Reset the P uptake sum as the maximum P uptake parameter
        // should apply to each sub-catchment individually.
        for (unsigned int i=0; i<ParSet.LandCount(); i++)
        {
            MaxUptake[i] = 0.0;
        }

        /**********************************************************
        *                 START OF TIME STEP LOOP                 *
        **********************************************************/
        for (int TimeStep=0; TimeStep<ParSet.TimeSteps; TimeStep++)
        {
            // Work out the date from the current time step and the start date
            CurrentDate = ParSet.StartDate + TimeStep;

            if (NewYear(CurrentDate))
            {
                // Get the number of days in the year again.
                yrsz = DaysInYear(CurrentDate);

                // Set Period to be the whole year until we find out otherwise.
                Period = yrsz;

                // Initialise total rainfall sum.
                totRain = 0.0;

                // Find out if the remaining run period is shorter
                // than one year. If so, deposition calculations
                // must be based on a part year.
                if ((ParSet.TimeSteps - (TimeStep + (int)yrsz)) < 0)
                {
                    Period = ParSet.TimeSteps - TimeStep + 1;
                }

                // Calculate total rainfall input
                for (int day=TimeStep; day<TimeStep+Period; day++)
                {
                    totRain += InputSeries->Data[3][day];
                }

                // Reset the N uptake sum as the maximum N uptake parameter
                // is an annual (calendar) value.
                for (unsigned int i=0; i<ParSet.LandCount(); i++)
                {
                    MaxUptake[i] = 0.0;
                }

            }

            flow[TimeStep] = 0.0;
            volume[TimeStep] = 0.0;
            diffuseNO3[TimeStep] = 0.0;
            diffuseNH4[TimeStep] = 0.0;
            diffuseDON[TimeStep] = 0.0;

            // Daily nitrate and ammonium dry deposition inputs
            // These are annual loads, so divide by the number of days
            // in a year to get the daily rate.
            ndrydep = 0.0;
            adrydep = 0.0;

            if (subPar.Dep[0] > 0.0)
            {
            	ndrydep = subPar.Dep[0] * (Period / yrsz) / yrsz;
			}

			if (subPar.Dep[2] > 0.0)
			{
            	adrydep = subPar.Dep[2] * (Period / yrsz) / yrsz;
			}

            // Daily nitrate and ammonium wet deposition inputs
            // These are annual loads so apportion according to rainfall
            nwetdep = 0.0;
            awetdep = 0.0;

            if (subPar.Dep[1] > 0.0)
            {
            	nwetdep = ((subPar.Dep[1] * (Period / yrsz))
                        	/ totRain) * InputSeries->Data[3][TimeStep];
			}

			if (subPar.Dep[3] > 0.0)
			{
            	awetdep = ((subPar.Dep[3] * (Period / yrsz))
                        	/ totRain) * InputSeries->Data[3][TimeStep];
			}

            // Add the daily dry deposition loads to the stores
            nDryStorage += ndrydep;
            aDryStorage += adrydep;

            // If there's no rainfall on this time step then we don't add
            // any dry deposition, we just leave it in the stores.
            if (InputSeries->Data[3][TimeStep] < 0.0001)
            {
                nDryInput = 0.0;
                aDryInput = 0.0;
            }
            // Otherwise, use the contents of the dry deposition stores
            // and reset the stores
            else
            {
                nDryInput = nDryStorage;
                aDryInput = aDryStorage;
                nDryStorage = 0.0;
                aDryStorage = 0.0;
            }

            // Convert runoff from mm/day to cumecs
            Inputs[0] = InputSeries->Data[1][TimeStep] * 1000.0 / SecondsInDay;


            useParameterFileLand = true;

            if (landInput.size() > 0)
            {
                for (unsigned int i=0; i<landInput.size(); ++i)
                {
                    if (landInput[i].HasDates(CurrentDate))
                    {
                        Land = landInput[i].GetLandUse();
                        useParameterFileLand = false;
                    }
                }
            }

            if (useParameterFileLand)
            {
                for (unsigned int LandUse=0; LandUse<ParSet.LandCount(); LandUse++)
                {
                    Land[LandUse] = subPar.Land[LandUse];
                }
            }


            /**********************************************************
            *                 START OF LAND USE LOOP                  *
            **********************************************************/
            for (unsigned int LandUse=0; LandUse<ParSet.LandCount(); LandUse++)
            {
                // Only run the cell model if the current land use
                // is present in this sub-catchement.
                if (Land[LandUse] > 0)
                {
                    // Copy the parameters for this land use to a
                    // temporary parameter set.  Do this because the
                    // model may need to change a parameter (i.e. uptake)
                    // during the course of a model run, but we want
                    // the parameters the user sees (for editing) to stay
                    // the same.
                    pars.Par = ParSet.CellPar[LandUse].Par;
                    pars.TC = ParSet.CellPar[LandUse].TC;
                    pars.Initial = ParSet.CellPar[LandUse].Initial;

					useNewHydrology = true;




                    if (useNewHydrology)
                    {
                        CalcPersist(InputSeries, pars, TimeStep, LandUse);

                        // Potential evapotranspiration
                        Inputs[18] = results->Hydrology[5].Data[LandUse][TimeStep];
                        // Soilwater retained water depth 'c12'
                        Inputs[19] = (1.0 - 1.0 / pars.TC[2]) * pars.Par[14];
                        // Soilwater freely draining depth 'c13'
                        Inputs[20] = pars.Par[14] / pars.TC[2];
                    }







                    if (!useNewHydrology)
                    {
                        // Store the correct VrMax in the Inputs array for
                        // convenience.
                        Inputs[7] = VrCalc(InputSeries->Data[0][TimeStep], LandUse);
                    }

                    bool precAsRain = false;

                    // Calculate temperature effect.
	                tDiff = pars.Par[15] * sin(3.0 * M_PI_2 * CurrentDate.DOY() / yrsz);
	                soiltemp = InputSeries->Data[2][TimeStep] - tDiff;

                    if (TimeStep > 0)
                    {
                        if (results->Soil[8].Data[LandUse][TimeStep - 1] > 0.0)
//                        if (soiltemp > 0.0)
                        {
                            Da = pars.Par[28] / (CS * 1000000.0);
                        }
                        else
                        {
                            Da = pars.Par[28] / (1000000.0 * (CS + pars.Par[29]));
                        }

                        soiltemp = results->Soil[8].Data[LandUse][TimeStep - 1]
                                        + 86400.0 * Da / pow((2.0 * LD), 2.0)
                                        * (InputSeries->Data[2][TimeStep] - results->Soil[8].Data[LandUse][TimeStep - 1]);
                    }

                    if (useSnowPack)
                    {
                        // Run snow pack accumulation model.
                        SnowMelt(snow[LandUse], melt[LandUse], InputSeries->Data[2][TimeStep],
                                    InputSeries->Data[3][TimeStep],
                                    pars.Par[25], precAsRain);

                        if (useNewHydrology)
                        {
                            // Convert snowpack depth in mm to water equivalents
                            results->Hydrology[9].Data[LandUse][TimeStep] = results->Hydrology[3].Data[LandUse][TimeStep]
                                                                            * pars.Par[26];
                            // Convert snowpack depth in mm to snowpack depth in cm
                            snowdepth = results->Hydrology[3].Data[LandUse][TimeStep] / 10.0;
                        }
                        else
                        {
                            results->Hydrology[9].Data[LandUse][TimeStep] = snow[LandUse];

                            // Convert snow pack depth in water equivalents
                            // to actual snow depth in centimetres.
                            snowdepth = snow[LandUse] / 10.0 / pars.Par[26];
                        }

                        // Calculate soil temperature from snow pack depth
                        // using empirical relationship.
                        soiltemp = soiltemp * exp(pars.Par[27] * snowdepth);

           //             results->SnowDepth.Data[LandUse][TimeStep] = snowdepth;
                    }

                    results->Soil[8].Data[LandUse][TimeStep] = soiltemp;

                    if (useCoup)
                    {
	                    Inputs[3] = pow(pars.Par[22], ((soiltemp - pars.Par[23]) / 10.0));
	                    Inputs[10] = pow(pars.Par[32], ((soiltemp - pars.Par[33]) / 10.0));
	                    Inputs[11] = pow(pars.Par[34], ((soiltemp - pars.Par[35]) / 10.0));
	                    Inputs[12] = pow(pars.Par[36], ((soiltemp - pars.Par[37]) / 10.0));
	                    Inputs[13] = pow(pars.Par[38], ((soiltemp - pars.Par[39]) / 10.0));
	                    Inputs[14] = pow(pars.Par[40], ((soiltemp - pars.Par[41]) / 10.0));
	                    Inputs[15] = pow(pars.Par[42], ((soiltemp - pars.Par[43]) / 10.0));
                    }
                    else
                    {
  	                    Inputs[3] = pow(1.047, (soiltemp - 20.0));
  	                    Inputs[10] = pow(1.047, (soiltemp - 20.0));
  	                    Inputs[11] = pow(1.047, (soiltemp - 20.0));
  	                    Inputs[12] = pow(1.047, (soiltemp - 20.0));
  	                    Inputs[13] = pow(1.047, (soiltemp - 20.0));
  	                    Inputs[14] = pow(1.047, (soiltemp - 20.0));
  	                    Inputs[15] = pow(1.047, (soiltemp - 20.0));
                    }

                    Inputs[16] = pow(1.047, (soiltemp - 20.0));

                    double smd = 0.0;

                    if (useNewHydrology)
                    {
                        if (TimeStep > 0)
                        {
                            smd = results->Soil[11].Data[LandUse][TimeStep - 1];
                        }
                    }
                    else
                    {
                        smd = InputSeries->Data[0][TimeStep];
                    }

                    // Calculate dryness factor for plant uptake, nitrification
                    // mineralisation and immobilisation processes
	                if (smd >= pars.Par[14])
                    {
                        Inputs[5] = 0;
                    }
	                else
                    {
                        Inputs[5] = (pars.Par[14] - smd) / pars.Par[14];
                    }

                    Inputs[1] = 0.0;
                    Inputs[2] = 0.0;

                    // If a deposition time series has been loaded then use
                    // that, otherwise use the daily inputs calculated from
                    // the annual loads in the parameter file.
                    if (depLoaded)
                    {
                        Inputs[1] = depInput[0][TimeStep];
                        Inputs[2] = depInput[1][TimeStep];
                    }
                    else
                    {
	                    Inputs[1] = nDryInput + nwetdep;
	                    Inputs[2] = aDryInput + awetdep;
                    }

                    // If a solid fertiliser application time series has
                    // been loaded then use that, otherwise use the daily
                    // input rate from the parameter file.

                    if (no3Fert && useNo3Fert && no3Fert->Loaded())
                    {
                        Inputs[1] += no3Fert->Data[LandUse][TimeStep];
                    }
                    else
                    {

                        // Only add the daily rate from the parameter file
                        // if we're in the fertiliser application period.
	                    if (CurrentDate.DOY() >= pars.Par[12]
		                    && CurrentDate.DOY() < (pars.Par[12] + pars.Par[13]))
                        {
                            Inputs[1] += pars.Par[4];
                        }
                    }

                    // If a liquid fertiliser application time series has
                    // been loaded then use that, otherwise use the daily
                    // input rate from the parameter file.

                    if (nh4Fert && useNh4Fert && nh4Fert->Loaded())
                    {
                        Inputs[2] += nh4Fert->Data[LandUse][TimeStep];
                    }
                    else
                    {

                        // Only add the daily rate from the parameter file
                        // if we're in the fertiliser application period.
 	                    if (CurrentDate.DOY() >= pars.Par[12]
		                    && CurrentDate.DOY() < (pars.Par[12] + pars.Par[13]))
                        {
                            Inputs[2] += pars.Par[8];
                        }
                    }

                    // If a liquid fertiliser application time series has
                    // been loaded then use that, otherwise use the daily
                    // input rate from the parameter file.

                    if (donFert && useDonFert && donFert->Loaded())
                    {
                        Inputs[17] += donFert->Data[LandUse][TimeStep];
                    }
                    else
                    {

                        // Only add the daily rate from the parameter file
                        // if we're in the fertiliser application period.
 	                    if (CurrentDate.DOY() >= pars.Par[12]
		                    && CurrentDate.DOY() < (pars.Par[12] + pars.Par[13]))
                        {
                            Inputs[17] += pars.Par[47];
                        }
                    }

                    // Convert phosphorus total inputs from kg/ha/day to kg/km2/day
                    Inputs[1] *= 100.0;
                    Inputs[2] *= 100.0;
             //       Inputs[17] *= 100.0;

//                    if (Inputs[1] < 0.0000001) Inputs[1] = 0.0;
//                    if (Inputs[2] < 0.0000001) Inputs[2] = 0.0;

                    InGrowthPeriod = false;

                    // Check to see if we are in a growth period.
                    // If multiple growth periods are loaded ...

                    if (growth && useGrowth && growth->PeriodCount[LandUse])
                    {
						// Loop through the loaded growth periods ...
                        for (int m=0; m<growth->PeriodCount[LandUse]; m++)
                        {
                            int currentDay = CurrentDate.DOY();
                            int startDay = dateCl::Date(growth->StartDate.Data[LandUse][m]).DOY();
                            int endDay = dateCl::Date(growth->EndDate.Data[LandUse][m]).DOY();

                            // and check to see if we're in the middle of one.
                            if (currentDay >= startDay && currentDay <= endDay)
                            {
                                InGrowthPeriod = true;

                                if (useNewHydrology)
                                {
                                    pars.Par[2] = growth->Offset.Data[LandUse][m];
                                    pars.Par[9] = growth->Amplitude.Data[LandUse][m];
                                }
                                else
                                {
                                    pars.Par[30] = growth->Offset.Data[LandUse][m];
                                    pars.Par[31] = growth->Amplitude.Data[LandUse][m];
                                }
                            }
                        }
                    }
                    else
                    {

                        // Otherwise see if we're in the growth period
                        // defined in the parameter file.
                        if (CurrentDate.DOY() >= pars.Par[10]
                            && CurrentDate.DOY() < (pars.Par[10] + pars.Par[11]))
                        {
                            InGrowthPeriod = true;
                        }
                    }

                    // Calculate biomass seasonal growth factor
	                Inputs[6] = pars.Par[30] + pars.Par[31] * sin(2.0 * M_PI
                                * (CurrentDate.DOY() - pars.Par[10]) / (int)yrsz);

                    // If we're in a growth period and we haven't hit the
                    // uptake limit, then allow uptake.
                    if (InGrowthPeriod && MaxUptake[LandUse] < pars.Par[3])
                    {
                        pars.Par[2] = ParSet.CellPar[LandUse].Par[2];
                        pars.Par[9] = ParSet.CellPar[LandUse].Par[9];
                    }
                    else
                    {
                        pars.Par[2] = 0.0;
                        pars.Par[9] = 0.0;
                    }

                    // If a temperature limit for denitrification has been
                    // set in the options, check to see if this time step's
                    // temperature is below that limit.  If so, stop the
                    // process by setting the parameter to zero.
                    if (pars.Par[16] < 9998.0)
                    {
                        if (soiltemp < pars.Par[16])
                        {
                            pars.Par[0] = 0.0;
                        }
                        else
                        {
                            pars.Par[0] = ParSet.CellPar[LandUse].Par[0];
                        }
                    }

                    // If a temperature limit for nitrification has been
                    // set in the options, check to see if this time step's
                    // temperature is below that limit.  If so, stop the
                    // process by setting the parameter to zero.
                    if (pars.Par[17] < 9998.0)
                    {
                        if (soiltemp < pars.Par[17])
                        {
                            pars.Par[5] = 0.0;
                        }
                        else
                        {
                            pars.Par[5] = ParSet.CellPar[LandUse].Par[5];
                        }
                    }

                    // If a temperature limit for mineralisation has been
                    // set in the options, check to see if this time step's
                    // temperature is below that limit.  If so, stop the
                    // process by setting the parameter to zero.
                    if (pars.Par[18] < 9998.0)
                    {
                        if (soiltemp < pars.Par[18])
                        {
                            pars.Par[6] = 0.0;
                        }
                        else
                        {
                            pars.Par[6] = ParSet.CellPar[LandUse].Par[6];
                        }
                    }

                    // If a temperature limit for immobilisation has been
                    // set in the options, check to see if this time step's
                    // temperature is below that limit.  If so, stop the
                    // process by setting the parameter to zero.
                    if (pars.Par[19] < 9998.0)
                    {
                        if (soiltemp < pars.Par[19])
                        {
                            pars.Par[7] = 0.0;
                        }
                        else
                        {
                            pars.Par[7] = ParSet.CellPar[LandUse].Par[7];
                        }
                    }

                    // Copy initial values for this land use type from the
                    // large 2D array to a small 1D array to make it easier
                    // to handle inside the solver.
	                for (int m=0; m<NUM_CELL_EQN; m++)
                    {
                        cinit[m] = luinit[LandUse][m];
                    }

                    double infiltration = 0.0;
                    double rain = InputSeries->Data[1][TimeStep];

                    double liquidPrecipitation = 0.0;
                    if (InputSeries->Data[2][TimeStep] > 0.0)
                    {
                        liquidPrecipitation = InputSeries->Data[3][TimeStep];
                    }

                    if (usePrecip)
                    {
//                        rain = InputSeries->Data[3][TimeStep];
                        rain = liquidPrecipitation + melt[LandUse];
                    }

                    if (soiltemp > 0.1)
                    {
                        infiltration = (pars.Par[60] / 86.4)
                                    * (1.0 - exp(-((rain / 86.4)
                                    / (pars.Par[60] / 86.4))));
                    }

                    Inputs[9] = (rain / 86.4) - infiltration;



                    Inputs[21] = 0.0;
                    if (soiltemp > pars.Par[61])
                    {
                        Inputs[21] = pars.Par[60]
                                    * (1.0 - exp(-(results->Hydrology[4].Data[LandUse][TimeStep]
                                    + results->Hydrology[2].Data[LandUse][TimeStep])
                                    / pars.Par[60]));
                    }

                    if (useNewHydrology)
                    {
                        rain = results->Hydrology[4].Data[LandUse][TimeStep]
                                    + results->Hydrology[2].Data[LandUse][TimeStep];
                        Inputs[9] = (rain - Inputs[21]) / 86.4;
                    }

                    results->Hydrology[8].Data[LandUse][TimeStep] = Inputs[9];


                    try
                    {
                        dascru(deqn, TimeStep-1, TimeStep, 0.1, NUM_CELL_EQN,
                                cinit, wk1, Inputs, Outputs,
                                pars, subPar, reachPar,
                                Conc,
                                useNewHydrology,
                                mSoil[LandUse]);
                    }
                    catch(EqnException& E)
                    {
                        E.TimeStep(TimeStep+1);
                        throw;
                    }


					// If the miniumum direct runoff flow level is set in
                    // the options and the output flow is lower than this
                    // level, force the output to be this minimum level.
                    if (pars.Par[21] < 9998.0 && cinit[21] < pars.Par[21])
                    {
                        cinit[21] = pars.Par[21];
                    }

                    double Qsw = useNewHydrology ? Outputs[16] : cinit[0];

                    // If the miniumum surface flow level is set in
                    // the options and the output flow is lower than this
                    // level, force the output to be this minimum level.
                    if (pars.Par[20] < 9998.0 && Qsw < pars.Par[20])
                    {
                        Qsw = pars.Par[20];
                    }

                    // If the miniumum sub-surface flow level is set in
                    // the options and the output flow is lower than this
                    // level, force the output to be this minimum level.
                    if (subPar.Par[9] < 9998.0
                        && cinit[1] < subPar.Par[9])
                    {
                        cinit[1] = subPar.Par[9];
                    }

                    // As long as we're not on the final time step,
                    // store the results from this time step as the initial
                    // conditions for the next time step.
                    for (int m=0; m<NUM_CELL_EQN; m++)
                    {
                        luinit[LandUse][m] = cinit[m];
                    }

                    // Store all cell model results (21 equations) to
                    // write out with the results.  These data may then
                    // be used to calculate the cell water and nitrogen
                    // balance.
                        for (int m=0; m<NUM_CELL_EQN; m++)
                        {
                            results->CellBalance[LandUse].Data[m][TimeStep] = cinit[m];
                        }


                    // Calculate and store surface and sub-surface flow and
                    // nitrate and ammonium concentrations.
                        results->Direct[0].Data[LandUse][TimeStep] = cinit[21];

                        if (cinit[24] > 0.0)
                        {
                            results->Direct[1].Data[LandUse][TimeStep] = (cinit[22] * 1000.0) / cinit[24];
                            results->Direct[2].Data[LandUse][TimeStep] = (cinit[23] * 1000.0) / cinit[24];
                            results->Direct[3].Data[LandUse][TimeStep] = cinit[24];
                            results->Direct[4].Data[LandUse][TimeStep] = (cinit[34] * 1000.0) / cinit[24];
                        }

                        double Vsw = cinit[6];
                        if (!useNewHydrology)
                        {
                            Vsw += Inputs[7];
                        }

                        // Discharge
                        results->Soil[0].Data[LandUse][TimeStep] = Qsw;
						// Nitrate as Nitrogen
                        results->Soil[1].Data[LandUse][TimeStep] = (cinit[2] * 1000.0) / Vsw;
						// Ammonium as Nitrogen
                        results->Soil[2].Data[LandUse][TimeStep] = (cinit[4] * 1000.0) / Vsw;
						// Total volume (drainage + retention
                        results->Soil[3].Data[LandUse][TimeStep] = Vsw;
						// Dissolved Organic Nitrogen
                        results->Soil[4].Data[LandUse][TimeStep] = (cinit[25] * 1000.0) / Vsw;
						// Soil Solid Organic Nitrogen
                        results->Soil[5].Data[LandUse][TimeStep] = cinit[26]
                                                                    / (mSoil[LandUse] * 1.0e-6);
						if (useNewHydrology)
                        {
                            // Drainage volume
                            results->Soil[6].Data[LandUse][TimeStep] = 1000.0 * (std::max)(0.0, (cinit[35] - Inputs[19] - pars.Par[62]));
                            // Retention volume
                            results->Soil[7].Data[LandUse][TimeStep] = 1000.0 * (std::min)(Inputs[19], (cinit[35] - pars.Par[62]));
                        }
                        else
                        {
                            // Drainage volume
                            results->Soil[6].Data[LandUse][TimeStep] = cinit[6];
                            // Retention volume
                            results->Soil[7].Data[LandUse][TimeStep] = Inputs[7];
                        }

                        results->Ground[0].Data[LandUse][TimeStep] = cinit[1];
                        results->Ground[1].Data[LandUse][TimeStep] = (cinit[3] * 1000.0) / cinit[7];
                        results->Ground[2].Data[LandUse][TimeStep] = (cinit[5] * 1000.0) / cinit[7];
                        results->Ground[3].Data[LandUse][TimeStep] = cinit[7];
                        results->Ground[4].Data[LandUse][TimeStep] = (cinit[27] * 1000.0) / cinit[7];

                        // Actual evapotranspiration
                        results->Hydrology[6].Data[LandUse][TimeStep] = Outputs[15];
                        // Depth of water in the soil box
                        results->Soil[10].Data[LandUse][TimeStep] = cinit[35];

                        // Soil moisture deficit
                        if (cinit[35] > (Inputs[19] + Inputs[20] + pars.Par[62] - pars.Par[57]))
                        {
                            results->Soil[11].Data[LandUse][TimeStep] = 0.0;
                        }
                        else
                        {
                            results->Soil[11].Data[LandUse][TimeStep] = (Inputs[19] + Inputs[20] + pars.Par[62])
                                                                            - cinit[35]
                                                                            - pars.Par[57];
                        }

                        // Depth of rain added to land + melt
                        results->Hydrology[7].Data[LandUse][TimeStep] = (results->Hydrology[4].Data[LandUse][TimeStep]
                                    + results->Hydrology[2].Data[LandUse][TimeStep]);
                        // New Q
                        results->Soil[9].Data[LandUse][TimeStep] = Inputs[21];

                    // Calculate daily loads
                    DailyLoadCalc(results, TimeStep, LandUse);

                    // Calculate how much total P uptake there
                    // has been so far in this simulation
                    MaxUptake[LandUse] += (results->Land[LandUse].Data[4][TimeStep]
                                            + results->Land[LandUse].Data[5][TimeStep]);

                    // Sum the flow results from each land use group in
                    // this catchment, apportioning amount according to
                    // percentage of this land use in this sub-catchment
                    // and sub-catchment area.

                    double soilWaterFlow;

                    if (Qsw > pars.Par[58])
                    {
                        soilWaterFlow = (1.0 - subPar.Par[1])
                                        * pars.Par[58];
                    }
                    else
                    {
                        soilWaterFlow = (1.0 - subPar.Par[1])
                                        * Qsw;
                    }

                    flow[TimeStep] += (soilWaterFlow + cinit[1] + cinit[21])
                                * subPar.Par[0]
                                * (Land[LandUse] / 100.0);

                    if (TimeStep == 0)
                    {
                        volume[TimeStep] += cinit[9]
                                * subPar.Par[0]
                                * (Land[LandUse] / 100.0);

                        // Sum the total phosphorus results from each
                        // land use group in this catchment, apportioning
                        // amount according to percentage of this land use
                        // in this sub-catchment and sub-catchment area.
                        // Results as concentrations are used for the
                        // calculation, then converted to mass
                        diffuseNO3[TimeStep] += cinit[12]
                                                * subPar.Par[0]
                                                * (Land[LandUse] / 100.0);
                        diffuseNH4[TimeStep] += cinit[13]
                                                * subPar.Par[0]
                                                * (Land[LandUse] / 100.0);
                        diffuseDON[TimeStep] += cinit[28]
                                                * subPar.Par[0]
                                                * (Land[LandUse] / 100.0);
                    }
                    else
                    {
                        volume[TimeStep] += (cinit[9] - results->CellBalance[LandUse].Data[9][TimeStep-1])
                                    * subPar.Par[0]
                                    * (Land[LandUse] / 100.0);

                         // Sum the total phosphorus results from each
                        // land use group in this catchment, apportioning
                        // amount according to percentage of this land use
                        // in this sub-catchment and sub-catchment area.
                        // Results as concentrations are used for the
                        // calculation, then converted to mass
                        diffuseNO3[TimeStep] += (cinit[12] - results->CellBalance[LandUse].Data[12][TimeStep-1])
                                                * subPar.Par[0]
                                                * (Land[LandUse] / 100.0);
                        diffuseNH4[TimeStep] += (cinit[13] - results->CellBalance[LandUse].Data[13][TimeStep-1])
                                                * subPar.Par[0]
                                                * (Land[LandUse] / 100.0);
                        diffuseDON[TimeStep] += (cinit[28] - results->CellBalance[LandUse].Data[28][TimeStep-1])
                                                * subPar.Par[0]
                                                * (Land[LandUse] / 100.0);
                    }



                // End land use 'if' statement
	            }


            // End land use loop
	        }


            updateLandModelProgress(TimeStep * (reachCount + 1) + 1);

        // End timestep loop for land phase
        }

		return true;
}
//---------------------------------------------------------------------------
double LandModel::VrCalc(double SMD, int LandUse)
{
    // Calculate the constants for the retention volume.
    // The VrMax in the parameter set isn't really the
    // VrMax we want, so calculate it here rather than
    // make the user do it.

    double Vr;

    Vr = ParSet.CellPar[LandUse].TC[2]
            * (ParSet.CellPar[LandUse].Par[14] - SMD)
            * 1000.0;

    return Vr;
}
//---------------------------------------------------------------------------
double LandModel::DaysInYear(dateCl::Date& date)
{
    dateCl::Date start = date.BOY();
    dateCl::Date end = date.EOY();
    long days = end - start + 1;

    return (double) days;
}
//---------------------------------------------------------------------------

bool LandModel::NewYear(dateCl::Date& Today)
{
    dateCl::Date Yesterday;

    Yesterday = Today - 1;

    return ((Today.NYear4() - Yesterday.NYear4()) != 0);
}
//---------------------------------------------------------------------------
void LandModel::SnowMelt(double& snow, double& xmelt, double temp, double prec, double fmelt, bool& rain)
{
    const double cps = 1.23;    // Correction factor for solid precipitation
    const double cpl = 1.08;    // Correction factor for liquid precipitation
    const double ts = -1.5;     // Temperature, below which only snow
    const double tl = 1.0;      // Temperature, below which only rain
    const double t0 = 0.5;      // Temperature, at which snow melts
    const double esnow0 = 0.09; // mm/day evaporation from snow

    xmelt = 0.0;

// Split precip into solid and liquid:
    double psol = 0.0;

    if (temp <= ts)
    {
        psol = prec;
    }

    if (temp > ts && temp < tl)
    {
        psol = prec * (tl - temp) / (tl - ts);
    }

    rain = (psol < 0.00001);

    double pliq = prec - psol;

// Correct precipitation measurements:
    psol = cps * psol;
    pliq = cpl * pliq;

// Increase snowpack:
    snow = snow + psol;

    if (temp > t0)
    {
        double xmelt = fmelt * (temp - t0);
        if (xmelt > snow) xmelt = snow;
        snow = snow - xmelt;
    }

// Evaporation from snow:
    double esnow = (std::min)(snow, esnow0);
    snow = snow - esnow;
}
//---------------------------------------------------------------------------
double LandModel::CalcSoilMass(CCellPar& pars)
{
    return (2650.0
            * pars.Par[14]
            * (1.0 - pars.Par[48]) /  pars.Par[48]
            * 1000.0);
}
//---------------------------------------------------------------------------
void LandModel::DailyLoadCalc(CResultsSet *res,
                            int TimeStep,
                            int LandUse)
{
    for (int i=10; i<21; i++)
    {
        if (TimeStep == 0)
        {
            res->Land[LandUse].Data[i-10][TimeStep] = res->CellBalance[LandUse].Data[i][TimeStep] / 100.0;
        }
        else
        {
            res->Land[LandUse].Data[i-10][TimeStep] = res->CellBalance[LandUse].Data[i][TimeStep] / 100.0
                                                - res->CellBalance[LandUse].Data[i][TimeStep-1] / 100.0;
        }
    }
    for (int i=28; i<NUM_CELL_EQN; i++)
    {
        if (TimeStep == 0)
        {
            res->Land[LandUse].Data[i-17][TimeStep] = res->CellBalance[LandUse].Data[i][TimeStep] / 100.0;
        }
        else
        {
            res->Land[LandUse].Data[i-17][TimeStep] = res->CellBalance[LandUse].Data[i][TimeStep] / 100.0
                                                - res->CellBalance[LandUse].Data[i][TimeStep-1] / 100.0;
        }
    }

    res->Land[LandUse].Data[12][TimeStep] = res->CellBalance[LandUse].Data[29][TimeStep] / 100.0;
}
//---------------------------------------------------------------------------
void LandModel::CalcPersist(CInputSeries* InputSeries,
                            CCellPar& pars,
                            int TimeStep,
                            int LandUse)
{
    if (InputSeries->Data[2][TimeStep] > subPar.Par[15])
    {
        // Rain
        results->Hydrology[0].Data[LandUse][TimeStep] = subPar.Par[13]
                                                        * pars.Par[49]
                                                        * InputSeries->Data[3][TimeStep];
        // Snow
        results->Hydrology[1].Data[LandUse][TimeStep] = 0.0;
    }
    else
    {
        // Rain
        results->Hydrology[0].Data[LandUse][TimeStep] = 0.0;
        // Snow
        results->Hydrology[1].Data[LandUse][TimeStep] = subPar.Par[14]
                                                        * pars.Par[50]
                                                        * InputSeries->Data[3][TimeStep];
    }

    double lastDepth = (TimeStep > 0)
                        ? (double)results->Hydrology[3].Data[LandUse][TimeStep-1]
                        : pars.Par[24];

    if (InputSeries->Data[2][TimeStep] < (subPar.Par[16] + pars.Par[51]))
    {
        // Melt
        results->Hydrology[2].Data[LandUse][TimeStep] = 0.0;
    }
    else
    {
        // Melt
        double m = pars.Par[25] * (InputSeries->Data[2][TimeStep] - (subPar.Par[16] + pars.Par[51]));
        results->Hydrology[2].Data[LandUse][TimeStep] = (std::min)(m, lastDepth);
    }

    // Snowpack depth
    results->Hydrology[3].Data[LandUse][TimeStep] = lastDepth
                        + results->Hydrology[1].Data[LandUse][TimeStep]
                        - results->Hydrology[2].Data[LandUse][TimeStep];

    double rainAndMelt = results->Hydrology[0].Data[LandUse][TimeStep]
                            + results->Hydrology[2].Data[LandUse][TimeStep];

    if (rainAndMelt > pars.Par[52])
    {
        // Depth of precipitation added to land
        results->Hydrology[4].Data[LandUse][TimeStep] = rainAndMelt
                                                        - pars.Par[52];
    }
    else
    {
        // Depth of precipitation added to land
        results->Hydrology[4].Data[LandUse][TimeStep] = 0.0;
    }

    if ((InputSeries->Data[2][TimeStep] + pars.Par[53]) > 0.0)
    {
        // Potential evapotranspiration
        results->Hydrology[5].Data[LandUse][TimeStep] = ((InputSeries->Data[4][TimeStep] * 0.0864) / (2.45 * 1000.0))
                            * (((InputSeries->Data[2][TimeStep] + pars.Par[53]) / pars.Par[54])
                            * 1000.0);
    }
    else
    {
        // Potential evapotranspiration
        results->Hydrology[5].Data[LandUse][TimeStep] = 0.0;
    }
}
//---------------------------------------------------------------------------



