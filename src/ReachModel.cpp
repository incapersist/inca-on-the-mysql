#include "ReachModel.h"
#include "Constant.h"
#include "Eqn.h"
#include "Integrat.h"

#include <sstream>
#include <cmath>

//---------------------------------------------------------------------------
ReachModel::ReachModel(CParSet& parset,
                        CSubPar& SubPar,
                        CReachPar& ReachPar,
                        CRiverPar& RiverPar,
                        CResultsSet *Results)
{
    ParSet = parset;
    subPar = SubPar;
    reachPar = ReachPar;
    riverPar = RiverPar;
    results = Results;

    reaches = ParSet.Reaches;

    Inputs.resize(13);
    Outputs.resize(8);
    Conc.resize(3);

    rinit.resize(NUM_RIVER_EQN);
    wk.resize(100);
}

//---------------------------------------------------------------------------
bool ReachModel::Run(CInputSeries* InputSeries,
                    std::vector<float>& abstractions,
                    std::vector< std::vector<float> >& upstreamInputs,
                    std::vector<float>& init,
                    void(* updateReachModelProgress)(float),
                    std::vector<std::string>& log,
                    bool isTopReach,
					unsigned int reachCount)
{
    bool Abstracted;
    CCellPar pars;

    dateCl::Date CurrentDate;
    CurrentDate.setFormat(dateCl::Date::DMY);

        for (TimeStep=0; TimeStep<ParSet.TimeSteps; TimeStep++)
        {
            // Work out the date from the current time step and the start date
            CurrentDate = ParSet.StartDate + TimeStep;

            Inputs[0] = upstreamInputs[0][TimeStep];
            Inputs[1] = upstreamInputs[1][TimeStep];
            Inputs[2] = upstreamInputs[2][TimeStep];
            Inputs[12] = upstreamInputs[4][TimeStep];

            if (isTopReach && TimeStep == 0)

            {
                Inputs[0] += riverPar.Initial[0];
            }

            Inputs[8] = abstractions[TimeStep];

            // Calculate instream temperature response function
            double waterTemp;

            if (InputSeries->Data[2][TimeStep] > riverPar.Pars[0])
            {
                waterTemp = InputSeries->Data[2][TimeStep];
            }
            else
            {
                waterTemp = riverPar.Pars[0];
            }

	        Inputs[3] = std::pow(1.047, (waterTemp - 20.0));

            if (TimeStep == 0)
            {
                // For the first reach the initial conditions are as defined
                // in the parameter set, otherwise they're the results from
                // the first day, upstream reach.
                rinit[0] = Inputs[0];

                // Calculate the reach time constant (L / (aQ**b))
                // and store in a convenient place.
//                Inputs[4] = CalcTimeConstant(rinit[0]);

                // Calculate initial volume in the reach
//                rinit[7] = rinit[0] * Inputs[4] * 86400.0;

                double initialReachWidth = reachPar.Par[1]
                                            * std::pow( riverPar.Initial[0],
                                                        reachPar.Par[2] );

                double initialReachDepth = reachPar.Par[11]
                                            * std::pow( riverPar.Initial[0], reachPar.Par[12] );

                double ReachArea = initialReachDepth
                                    * ( ( initialReachWidth
                                            + reachPar.Par[13] ) / 2.0 );

                // Calculate initial volume in the reach
                Inputs[9] = ReachArea * reachPar.Par[0];

                double ReachPerimeter = reachPar.Par[13]
                                + 2.0 * std::pow( ( std::pow( ( ( initialReachWidth - reachPar.Par[13] ) / 2.0 ), 2.0 ) + std::pow( initialReachDepth, 2.0 ) ), 0.5 );

                double ReachRadius = ReachArea / ReachPerimeter;

                // Initial velocity in the reach
                Inputs[10] = 1.0 / reachPar.Par[14]
                        * std::pow( ReachRadius, ( 2.0 / 3.0 ) )
                        * std::pow( reachPar.Par[15], 0.5 );

                if (isTopReach)
                {
//                    rinit[1] = riverPar.Initial[1] * rinit[7] / 1000.0;
//                    rinit[2] = riverPar.Initial[2] * rinit[7] / 1000.0;
                    rinit[1] = riverPar.Initial[1] * Inputs[9] / 1000.0;
                    rinit[2] = riverPar.Initial[2] * Inputs[9] / 1000.0;
                    rinit[14] = riverPar.Initial[3] * Inputs[9] / 1000.0;
                }
                else
                {
//                    rinit[1] = init[1] * rinit[7] / 1000.0;
//                    rinit[2] = init[2] * rinit[7] / 1000.0;
                    rinit[1] = init[1] * Inputs[9] / 1000.0;
                    rinit[2] = init[2] * Inputs[9] / 1000.0;
                    rinit[14] = init[3] * Inputs[9] / 1000.0;
                }

                // Copy the initial conditions for use in the instream balance
                // calculation, shown on the time series charts.
                for (unsigned int j=0; j<NUM_RIVER_EQN; j++)
                {
                    results->ReachInit[j] = rinit[j];
                }
            }
            else
            {
                Inputs[9] = results->Daily[0].Data[3][TimeStep-1];
                Inputs[11] = results->Daily[0].Data[0][TimeStep-1];
            }

            CalcRiverConc(Inputs[9]);

            // Run the solver with the river model equations and
            // attempt to trap any exceptions.  Only a partial
            // solution, should be improved.
            try
            {
                dascru(reqn, TimeStep, TimeStep+1, 0.01, NUM_RIVER_EQN,
                            rinit, wk, Inputs, Outputs,
                            pars, subPar, reachPar,
                            Conc,
                            Abstracted,
                            0.0);



            }
            catch(EqnException& E)
            {
                E.TimeStep(TimeStep+1);
                throw;
            }


            // Increment the time step progress bar
            updateReachModelProgress(TimeStep * (reachCount + 1) + 1);

            if (!Abstracted && abstractions[TimeStep] > 0.0)
            {
                std::ostringstream entry;
                entry << "Abstraction could not be performed on ";
                entry << CurrentDate;
                entry << " in reach ";

                log.push_back(entry.str());
            }

            for (int j=0; j<NUM_RIVER_EQN; j++)
            {
                results->RiverBalance[0].Data[j][TimeStep] = rinit[j];
            }

            results->RiverBalance[0].Data[0][TimeStep] = Inputs[11];
            results->RiverBalance[0].Data[3][TimeStep] = Inputs[9];

            StoreRiverResults(Inputs[9]);

        // End time step loop
        }

		return true;
}
//---------------------------------------------------------------------------

void ReachModel::CalcRiverConc(double volume)
{
    Conc[0] = rinit[1] * 1000.0 / volume;
    Conc[1] = rinit[2] * 1000.0 / volume;
    Conc[2] = rinit[14] * 1000.0 / volume;
}
//---------------------------------------------------------------------------

void ReachModel::StoreRiverResults(double volume)
{
    // Calculate river model concentrations
    CalcRiverConc(volume);

    // Flow
    results->Daily[0].Data[0][TimeStep] = Inputs[11];

    // Nitrate
    results->Daily[0].Data[1][TimeStep] = Conc[0];

    // Ammonium
    results->Daily[0].Data[2][TimeStep] = Conc[1];

    // Volume
    results->Daily[0].Data[3][TimeStep] = Inputs[9];

    // Organic N
    results->Daily[0].Data[4][TimeStep] = Conc[2];

    // Velocity
    results->Daily[0].Data[5][TimeStep] = Inputs[10];

    // Width
    results->Daily[0].Data[6][TimeStep] = Outputs[2];

    // Depth
    results->Daily[0].Data[7][TimeStep] = Outputs[3];

    // Area
    results->Daily[0].Data[8][TimeStep] = Outputs[4];

    // Perimeter
    results->Daily[0].Data[9][TimeStep] = Outputs[5];

    // Radius
    results->Daily[0].Data[10][TimeStep] = Outputs[6];

    // Residence time
    results->Daily[0].Data[11][TimeStep] = Outputs[7];
}
//---------------------------------------------------------------------------
double ReachModel::CalcTimeConstant(double Q)
{
    return (reachPar.Par[0] / (reachPar.Par[1]
            * std::pow(Q, reachPar.Par[2]) * 86400.0));
}
//---------------------------------------------------------------------------

