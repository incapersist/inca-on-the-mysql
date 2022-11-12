//---------------------------------------------------------------------------
#include <algorithm>
#include <exception>
#include <cmath>

#include "Eqn.h"

//---------------------------------------------------------------------------
using std::max;

void deqn(std::vector<double>& x0,
            std::vector<double>& xp,
            std::vector<double>& Inputs,
            std::vector<double>& Outputs,
            CCellPar& pars,
            CSubPar& subPar,
            CReachPar& reachPar,
            std::vector<double>& Conc,
            bool& useNewHydrology,
            double Msoil)
{

int Reach = 0;
//bool useNewHydrology = true;

double Qsw = x0[0];

double Vsw = x0[6];
if (!useNewHydrology)
{
    Vsw += Inputs[7];
}

double thresholdQ = pars.Par[58] + subPar.Par[17];
double rainfallExcess = pars.Par[59] + subPar.Par[18];

if (useNewHydrology)
{
    if (x0[35] < (Inputs[19] + pars.Par[62]))
    {
        Qsw = 0.0;
    }
    else
    {
        Qsw = (1.0 / 86.4) * ((x0[35] - Inputs[19] - pars.Par[62]) / pars.TC[1]);
    }

    // Actual evapotranspiration
    try
    {
        Outputs[15] = Inputs[18];
        if (x0[35] < (Inputs[19] + pars.Par[62]))
        {
            Outputs[15] = (std::min)((x0[35] - pars.Par[62]), (std::pow(((x0[35] - pars.Par[62]) / Inputs[19]), pars.Par[56]) * Inputs[18]));
        }
    }
    catch (std::exception& E)
    {
        throw EqnException(101, Reach, "land", "actual evapotranspiration",
                            x0[1], x0[7], subPar.Par[5], subPar.Par[1] * Qsw, x0[1]);
    }

    // Soil water depth
    try
    {
        xp[35] = Inputs[21] - (Qsw * 86.4) - Outputs[15];
    }
    catch (std::exception& E)
    {
        throw EqnException(101, Reach, "land", "soil water depth",
                            x0[1], x0[7], subPar.Par[5], subPar.Par[1] * Qsw, x0[1]);
    }
}

    try
    {
        if (useNewHydrology)
        {
            xp[0] = 0.0;
        }
        else
        {
            xp[0] = (Inputs[0] - Qsw) / pars.TC[1];
        }
    }
    catch (std::exception& E)
    {
        throw EqnException(101, Reach, "land", "soil water flow",
                            x0[1], x0[7], subPar.Par[5], subPar.Par[1] * Qsw, x0[1]);
    }

    try
    {
        if (Qsw > thresholdQ)
        {
            // Groundwater box: Flow
            xp[1] = (subPar.Par[1]
                    * (Qsw - (Qsw - pars.Par[58])) - x0[1])
                    / subPar.Par[5];
        }
        else
        {
            // Groundwater box: Flow
            xp[1] = (subPar.Par[1] * Qsw - x0[1])
                    / subPar.Par[5];
        }
    }
    catch (std::exception& E)
    {
        throw EqnException(101, Reach, "land", "groundwater flow",
                            x0[1], x0[7], subPar.Par[5], subPar.Par[1] * Qsw, x0[1]);
    }


 /***** direct runoff box *********/

    if (Qsw > thresholdQ)
    {
        // Direct runoff
        try
        {
            // Both saturation excess and infiltration excess
            xp[21] =  (Qsw - thresholdQ
                        + rainfallExcess * Inputs[9] - x0[21])
                        / pars.TC[0];
        }
        catch (std::exception& e)
        {
            throw EqnException(100, Reach+1, "land", "direct runoff",
                            x0[21], x0[12], pars.TC[0]);
        }

        try
        {
            // Direct runoff water volume change
            xp[24] = (((Qsw - thresholdQ)
                        + rainfallExcess * Inputs[9] - x0[21]))
                        * 86400.0;
        }
        catch (std::exception& e)
        {
            throw EqnException(113, Reach+1, "land", "Direct runoff water volume change",
                            Qsw, x0[12], Inputs[0]);
        }

            try
            {
                // Direct runoff nitrate
                xp[22] = ((Qsw - thresholdQ)
                            * x0[2] * 86400.0)
                            / Vsw
                            - (x0[22] * x0[21] * 86400.0) / x0[24];
            }
            catch (std::exception& e)
            {
                throw EqnException(100, Reach+1, "land", "direct runoff",
                            x0[21], x0[12], pars.TC[0]);
            }

            try
            {
                // Direct runoff ammonium
                xp[23] = ((Qsw - thresholdQ)
                            * x0[4] * 86400.0)
                            / Vsw
                            - (x0[23] * x0[21] * 86400.0) / x0[24];
            }
            catch (std::exception& e)
            {
                throw EqnException(100, Reach+1, "land", "direct runoff",
                            x0[21], x0[12], pars.TC[0]);
            }

            try
            {
                // Direct runoff DON
                xp[34] = ((Qsw - thresholdQ)
                            * x0[25] * 86400.0)
                            / Vsw
                            - (x0[34] * x0[21] * 86400.0) / x0[24];
            }
            catch (std::exception& e)
            {
                throw EqnException(100, Reach+1, "land", "direct runoff",
                            x0[21], x0[12], pars.TC[0]);
            }
    }
    else
    {
        // Direct runoff
        try
        {
            // Infiltration excess only
            xp[21] =  (rainfallExcess * Inputs[9] - x0[21])
                        / pars.TC[0];
        }
        catch (std::exception& e)
        {
            throw EqnException(100, Reach+1, "land", "direct runoff",
                            x0[21], x0[12], pars.TC[0]);
        }

        try
        {
            // Direct runoff water volume change
            xp[24] = (rainfallExcess * Inputs[9] - x0[21])
                        * 86400.0;
        }
        catch (std::exception& e)
        {
            throw EqnException(113, Reach+1, "land", "Direct runoff water volume change",
                            Qsw, x0[12], Inputs[0]);
        }

        if (x0[24] > 0.0)
        {
            try
            {
                // Direct runoff nitrate
                xp[22] = -(x0[22] * x0[21] * 86400.0) / x0[24];
            }
            catch (std::exception& e)
            {
                throw EqnException(100, Reach+1, "land", "direct runoff",
                            x0[21], x0[12], pars.TC[0]);
            }

            try
            {
                // Direct runoff ammonium
                xp[23] = -(x0[23] * x0[21] * 86400.0) / x0[24];
            }
            catch (std::exception& e)
            {
                throw EqnException(100, Reach+1, "land", "direct runoff",
                            x0[21], x0[12], pars.TC[0]);
            }

            try
            {
                // Direct runoff DON
                xp[34] = -(x0[34] * x0[21] * 86400.0) / x0[24];
            }
            catch (std::exception& e)
            {
                throw EqnException(100, Reach+1, "land", "direct runoff",
                            x0[21], x0[12], pars.TC[0]);
            }
        }
        else
        {
            xp[22] = 0.0;
            xp[23] = 0.0;
            xp[34] = 0.0;
        }
    }

    Conc[0] = (Qsw > 0.0) ? ((x0[2] * Qsw * 86400.0)) / Vsw : 0.0;

    double no3uptake = (pars.Par[2] * Inputs[14] * x0[2] * Inputs[5] * Inputs[6]);

    if (useNewHydrology)
    {
        no3uptake = (Conc[0] > 0.0) ? (x0[2] * ((pars.Par[2] * Conc[0]) / (pars.Par[30] * Conc[0])) * Outputs[15]
                / Vsw * 1000000.0) : 0.0;
    }

    try
    {
        // Soil box: Nitrate
        xp[2] = Inputs[1] - Conc[0]
                // Uptake
                - no3uptake
                // Denitrification
                - (pars.Par[0] * Inputs[3] * x0[2] * Inputs[5])
                / Vsw * 1000000.0
                // Nitrification
                + (pars.Par[5] * Inputs[11] * x0[4] * Inputs[5])
                / Vsw * 1000000.0
                // Fixation
                + pars.Par[1] * Inputs[10] * 100.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(102, Reach, "land", "soil water nitrate",
                            Qsw, Vsw, pars.TC[1], Inputs[1], x0[2]);
    }

    try
    {
        if (Qsw > thresholdQ)
        {
            // Groundwater box: Nitrate
            xp[3] = (subPar.Par[1]
                    * (Qsw - (Qsw - thresholdQ))
                    * x0[2] * 86400.0)
                    / Vsw
                    - (x0[3] * x0[1] * 86400.0) / x0[7]
                    // Denitrification
                    - (subPar.Par[2] * x0[3] * Inputs[16]) / x0[7] * 1000000
                    // Nitrification
                    + (subPar.Par[12] * Inputs[16] * x0[27]) / x0[7] * 1000000;
        }
        else
        {
            double conc  = (Qsw > 0.0) ? ((Qsw * x0[25] * 86400.0) / Vsw) : 0.0;
            // Groundwater box: Nitrate
            xp[3] = conc
                    - (x0[3] * x0[1] * 86400.0) / x0[7]
                    // Denitrification
                    - (subPar.Par[2] * x0[3] * Inputs[16]) / x0[7] * 1000000
                    // Nitrification
                    + (subPar.Par[12] * Inputs[16] * x0[5]) / x0[7] * 1000000;
        }
    }
    catch (std::exception& E)
    {
        throw EqnException(103, Reach, "land", "groundwater nitrate",
                            x0[1], x0[7], subPar.Par[5], subPar.Par[1] * x0[2], x0[3]);
    }

    Conc[1] = (Qsw > 0.0) ? ((x0[4] * Qsw * 86400.0) / Vsw) : 0.0;

    double nh4uptake = (pars.Par[9] * Inputs[15] * x0[4] * Inputs[5] * Inputs[6]);

    if (useNewHydrology)
    {
        nh4uptake = (Conc[1] > 0.0) ? (x0[4] * ((pars.Par[9] * Conc[1]) / (pars.Par[31] * Conc[1])) * Outputs[15]
                / Vsw * 1000000.0) : 0.0;
    }

    try
    {
        // Soil box: Ammonium
        xp[4] = Inputs[2] - Conc[1]
                // Uptake
                - nh4uptake
                // Nitrification
                - (pars.Par[5] * Inputs[11] * x0[4] * Inputs[5])
                / Vsw * 1000000.0
                // Immobilisation
                - (pars.Par[7] * Inputs[13] * x0[4] * Inputs[5])
                / Vsw * 1000000.0
                // Mineralisation
                + pars.Par[6] * Inputs[12] * Inputs[5] * x0[26] * 100.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(104, Reach, "land", "soil water ammonium",
                            Qsw, Vsw, pars.TC[1], Inputs[2], x0[4]);
    }

    try
    {
        // Groundwater box: Ammonium
        if (Qsw > thresholdQ)
        {
            xp[5] = (subPar.Par[1]
                    * (Qsw - (Qsw - thresholdQ))
                    * x0[4] * 86400.0)
                    / Vsw
                    - (x0[5] * x0[1] * 86400.0) / x0[7]
                    // Mineralisation
                    + (subPar.Par[10] * Inputs[16] * x0[27]) / x0[7] * 1000000
                    // Nitrification
                    - (subPar.Par[12] * Inputs[16] * x0[5]) / x0[7] * 1000000;
        }
        else
        {
            double conc = (Qsw > 0.0) ? ((subPar.Par[1] * x0[4] * Qsw * 86400.0) / Vsw) : 0.0;
            xp[5] = conc
                    - (x0[5] * x0[1] * 86400.0) / x0[7]
                    // Mineralisation
                    + (subPar.Par[10] * Inputs[16] * x0[27]) / x0[7] * 1000000
                    // Nitrification
                    - (subPar.Par[12] * Inputs[16] * x0[5]) / x0[7] * 1000000;
        }
    }
    catch (std::exception& E)
    {
        throw EqnException(105, Reach, "land", "groundwater ammonium",
                            x0[1], x0[7], subPar.Par[5], subPar.Par[1] * x0[4], x0[5]);
    }

    try
    {
        // Soil box: Water volume
        if (useNewHydrology)
        {
            xp[6] = 1000.0 * xp[35];
  //          xp[6] = xp[0] * 86400.0;
        }
        else
        {
            xp[6] = (Inputs[0] - Qsw) * 86400.0;
        }
    }
    catch (std::exception& E)
    {
        throw EqnException(106, Reach, "land", "soil water volume",
                            Qsw, Vsw, pars.TC[1]);
    }

    try
    {
        // Groundwater box: Water volume
        if (Qsw > thresholdQ)
        {
            xp[7] = (subPar.Par[1]
                    * thresholdQ - x0[1])
                    * 86400.0;
        }
        else
        {
            xp[7] = (subPar.Par[1] * Qsw - x0[1]) * 86400.0;
        }
    }
    catch (std::exception& E)
    {
        throw EqnException(107, Reach, "land", "groundwater flow",
                            x0[1], x0[7], subPar.Par[5]);
    }

    try
    {
        // Total water input
        xp[8] = (Inputs[0] + rainfallExcess * Inputs[9])
                * 86400.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(108, Reach, "land", "total water input",
                            Qsw, Vsw, pars.TC[1]);
    }

    try
    {
        // Total water output
        if (Qsw > thresholdQ)
        {
            xp[9] = ((1.0 - subPar.Par[1])
                        * thresholdQ
                        + x0[1] + x0[21]) * 86400.0;
        }
        else
        {
            xp[9] = ((1.0 - subPar.Par[1])
                        * Qsw
                        + x0[1] + x0[21]) * 86400.0;
        }
    }
    catch (std::exception& E)
    {
        throw EqnException(109, Reach, "land", "total water output",
                            Qsw, Vsw, pars.TC[1]);
    }

    try
    {
        // Total NO3 input
        xp[10] = Inputs[1];
    }
    catch (std::exception& E)
    {
        throw EqnException(110, Reach, "land", "total nitrate input",
                            Qsw, Vsw, pars.TC[1]);
    }

    try
    {
        // Total NH4 input
        xp[11] = Inputs[2];
    }
    catch (std::exception& E)
    {
        throw EqnException(111, Reach, "land", "total ammonium input",
                            Qsw, Vsw, pars.TC[1]);
    }

    if (Qsw > thresholdQ)
    {
        if (x0[24] > 0)
        {
            try
            {
                // Total NO3 output
                xp[12] = ((1.0 - subPar.Par[1])
                            * thresholdQ * x0[2] * 86400.0) / Vsw
                            + (x0[1] * x0[3] * 86400.0) / x0[7]
                            + (x0[21] * x0[22] * 86400.0) / x0[24];
            }
            catch (std::exception& E)
            {
                throw EqnException(112, Reach, "land", "total nitrate output",
                            Qsw, Vsw, pars.TC[1]);
            }

            try
            {
                // Total NH4 output
                xp[13] = ((1.0 - subPar.Par[1])
                        * thresholdQ * x0[4] * 86400.0) / Vsw
                        + (x0[1] * x0[5] * 86400.0) / x0[7]
                        + (x0[21] * x0[23] * 86400.0) / x0[24];
            }
            catch (std::exception& E)
            {
                throw EqnException(113, Reach, "land", "total ammonium output",
                            Qsw, Vsw, pars.TC[1]);
            }

            try
            {
                // Total DON output
                xp[28] = ((1.0 - subPar.Par[1])
                            * thresholdQ * x0[25] * 86400.0) / Vsw
                            + (x0[1] * x0[27] * 86400.0) / x0[7]
                            + (x0[21] * x0[34] * 86400.0) / x0[24];
            }
            catch (std::exception& E)
            {
                throw EqnException(113, Reach, "land", "total DON output",
                            Qsw, Vsw, pars.TC[1]);
            }
        }
        else
        {
            try
            {
                // Total NO3 output
                xp[12] = ((1.0 - subPar.Par[1])
                            * thresholdQ
                            * x0[2] * 86400.0) / Vsw
                            + (x0[1] * x0[3] * 86400.0) / x0[7];
            }
            catch (std::exception& E)
            {
                throw EqnException(112, Reach, "land", "total nitrate output",
                            Qsw, Vsw, pars.TC[1]);
            }

            try
            {
                // Total NH4 output
                xp[13] = ((1.0 - subPar.Par[1])
                        * thresholdQ
                        * x0[4] * 86400.0) / Vsw
                        + (x0[1] * x0[5] * 86400.0) / x0[7];
            }
            catch (std::exception& E)
            {
                throw EqnException(113, Reach, "land", "total ammonium output",
                            Qsw, Vsw, pars.TC[1]);
            }

            try
            {
                // Total DON output
                xp[28] = ((1.0 - subPar.Par[1])
                            * thresholdQ * x0[25] * 86400.0) / Vsw
                            + (x0[1] * x0[27] * 86400.0) / x0[7];
            }
            catch (std::exception& E)
            {
                throw EqnException(113, Reach, "land", "total DON output",
                            Qsw, Vsw, pars.TC[1]);
            }
        }
    }
    else
    {
        if (x0[24] > 0)
        {
            try
            {
                double conc = (Qsw > 0.0) ? (((1.0 - subPar.Par[1])
                            * Qsw
                            * x0[2] * 86400.0) / Vsw) : 0.0;
                // Total NO3 output
                xp[12] = conc
                            + (x0[1] * x0[3] * 86400.0) / x0[7]
                            + (x0[21] * x0[22] * 86400.0) / x0[24];
            }
            catch (std::exception& E)
            {
                throw EqnException(112, Reach, "land", "total nitrate output",
                            Qsw, Vsw, pars.TC[1]);
            }

            try
            {
                double conc = (Qsw > 0.0) ? (((1.0 - subPar.Par[1])
                        * Qsw * x0[4] * 86400.0) / Vsw) : 0.0;
                // Total NH4 output
                xp[13] = conc
                        + (x0[1] * x0[5] * 86400.0) / x0[7]
                        + (x0[21] * x0[23] * 86400.0) / x0[24];
            }
            catch (std::exception& E)
            {
                throw EqnException(113, Reach, "land", "total ammonium output",
                            Qsw, Vsw, pars.TC[1]);
            }

            try
            {
                double conc = (Qsw > 0.0) ? (((1.0 - subPar.Par[1])
                            * Qsw * x0[25] * 86400.0) / Vsw) : 0.0;
                // Total DON output
                xp[28] = conc
                            + (x0[1] * x0[27] * 86400.0) / x0[7]
                            + (x0[21] * x0[34] * 86400.0) / x0[24];
            }
            catch (std::exception& E)
            {
                throw EqnException(113, Reach, "land", "total DON output",
                            Qsw, Vsw, pars.TC[1]);
            }
        }
        else
        {
            try
            {
                double conc = (Qsw > 0.0) ? (((1.0 - subPar.Par[1])
                            * Qsw
                            * x0[2] * 86400.0) / Vsw) : 0.0;
                // Total NO3 output
                xp[12] = conc
                            + (x0[1] * x0[3] * 86400.0) / x0[7];
            }
            catch (std::exception& E)
            {
                throw EqnException(112, Reach, "land", "total nitrate output",
                            Qsw, Vsw, pars.TC[1]);
            }

            try
            {
                double conc = (Qsw > 0.0) ? (((1.0 - subPar.Par[1])
                        * Qsw * x0[4] * 86400.0) / Vsw) : 0.0;
                // Total NH4 output
                xp[13] = conc
                        + (x0[1] * x0[5] * 86400.0) / x0[7];
            }
            catch (std::exception& E)
            {
                throw EqnException(113, Reach, "land", "total ammonium output",
                            Qsw, Vsw, pars.TC[1]);
            }

            try
            {
                double conc = (Qsw > 0.0) ? (((1.0 - subPar.Par[1])
                            * Qsw * x0[25] * 86400.0) / Vsw) : 0.0;
                // Total DON output
                xp[28] = conc
                            + (x0[1] * x0[27] * 86400.0) / x0[7];
            }
            catch (std::exception& E)
            {
                throw EqnException(113, Reach, "land", "total DON output",
                            Qsw, Vsw, pars.TC[1]);
            }
        }
    }

    try
    {
        // Nitrate uptake
          xp[14] = no3uptake;
    }
    catch (std::exception& E)
    {
        throw EqnException(114, Reach, "land", "nitrate uptake",
                            Qsw, Vsw, pars.TC[1]);
    }

    try
    {
        // Ammonium uptake
        xp[15] = nh4uptake;
    }
    catch (std::exception& E)
    {
        throw EqnException(115, Reach, "land", "ammonium uptake",
                            Qsw, Vsw, pars.TC[1]);
    }

    try
    {
        // Mineralisation
        xp[16] = pars.Par[6] * Inputs[12] * Inputs[5] * x0[26] * 100.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(116, Reach, "land", "mineralisation",
                            Qsw, Vsw, pars.TC[1]);
    }

    try
    {
        // Soil Water Nitrification
        xp[17] = (pars.Par[5] * Inputs[11] * x0[4] * Inputs[5])
                    / Vsw * 1000000.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(117, Reach, "land", "nitrification",
                            Qsw, Vsw, pars.TC[1]);
    }

    try
    {
        // Soil Water Denitrification
        xp[18] = ((pars.Par[0] * Inputs[3] * x0[2] * Inputs[5])
                    / Vsw
                    + (subPar.Par[2] * x0[3] * Inputs[16]) / x0[7])
                    * 1000000.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(118, Reach, "land", "denitrification",
                            Qsw, Vsw, pars.TC[1]);
    }

    try
    {
        // Fixation
        xp[19] = pars.Par[1] * Inputs[10] * 100.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(119, Reach, "land", "fixation",
                            Qsw, Vsw, pars.TC[1]);
    }

    try
    {
        // Immobilisation
        xp[20] = (pars.Par[7] * Inputs[13] * x0[4] * Inputs[5])
                    / Vsw * 1000000.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(120, Reach, "land", "immobilisation",
                            Qsw, Vsw, pars.TC[1]);
    }


/************
* ORGANIC N *
************/

    double don = (x0[25] * 1000.0) / Vsw;
    double epc0 = don;

    try
    {
        if (Msoil > 0.0 && x0[26] > 0.0 && pars.Par[46] > 0.0)
        {
            epc0 = std::pow(((1000000.0 / pars.Par[46]) * (x0[26] / Msoil)), pars.Par[45]);
        }
    }
    catch (std::exception& E)
    {
        throw EqnException(120, Reach, "land", "epc0",
                            Qsw, Vsw, pars.TC[1]);
    }

    double one_over_n = 1.0 / pars.Par[45];
    double sorption = 0.0;

    try
    {
        sorption = pars.Par[44] / 1000.0
                    * (std::pow(don, one_over_n) - std::pow(epc0, one_over_n))
                    * Vsw;
    }
    catch (std::exception& E)
    {
        throw EqnException(120, Reach, "land", "sorption",
                            Qsw, Vsw, pars.TC[1]);
    }

    Conc[2] = (Qsw > 0.0) ? ((Qsw * x0[25] * 86400.0) / Vsw) : 0.0;

    // Change in Soilwater Dissolved Organic Nitrogen
    try
    {
        xp[25] = -Conc[2]  // dissolved flux out
                // sorption isotherm
                - sorption;
    }
    catch (std::exception& E)
    {
        throw EqnException(120, Reach, "land", "soil water DON",
                            Qsw, Vsw, pars.TC[1]);
    }

    // Change in Soil Solid Organic Nitrogen
    try
    {
        xp[26] = Inputs[17] * 100.0  // solid organic N input
                // sorption isotherm
                + sorption
                // mineralisation
                - pars.Par[6] * Inputs[12] * Inputs[5] * x0[26] * 100.0
                // immobilization
                + (pars.Par[7] * Inputs[13] * x0[4] * Inputs[5]) / Vsw * 1000000.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(120, Reach, "land", "soil solid ON",
                            Qsw, Vsw, pars.TC[1]);
    }

    // Change in Groundwater Dissolved Organic N
    try
    {
        if (Qsw > thresholdQ)
        {
                    // input from soil layer
            xp[27] = (subPar.Par[1]
                    * (Qsw - (Qsw - thresholdQ))
                    * Qsw * x0[25] * 86400.0) / Vsw
                    // outflow to reach
                    - (x0[1] * x0[27] * 86400.0) / x0[7]
                    // mineralisation
                    - (subPar.Par[10] * Inputs[16] * x0[27]) / x0[7] * 1000000;
        }
        else
        {
            double conc = (Qsw > 0.0) ? ((subPar.Par[1] * Qsw * x0[25] * 86400.0) / Vsw) : 0.0;
                    // input from soil layer
            xp[27] = conc
                    // outflow to reach
                    - (x0[1] * x0[27] * 86400.0) / x0[7]
                    // mineralisation
                    - (subPar.Par[10] * Inputs[16] * x0[27]) / x0[7] * 1000000;
        }
    }
    catch (std::exception& E)
    {
        throw EqnException(120, Reach, "land", "groundwater DON",
                            Qsw, Vsw, pars.TC[1]);
    }

    try
    {
        // Organic Nitrogen sorption isotherm
        xp[29] = sorption;
    }
    catch (std::exception& E)
    {
        throw EqnException(120, Reach, "land", "sorption",
                            Qsw, Vsw, pars.TC[1]);
    }

    try
    {
        // Groundwater mineralisation
        xp[30] = (subPar.Par[10] * Inputs[16] * x0[27]) / x0[7] * 1000000;
    }
    catch (std::exception& E)
    {
        throw EqnException(120, Reach, "land", "groundwater mineralisation",
                            Qsw, Vsw, pars.TC[1]);
    }

    try
    {
        // Groundwater nitrification
        xp[31] = (subPar.Par[12] * Inputs[16] * x0[5]) / x0[7] * 1000000;
    }
    catch (std::exception& E)
    {
        throw EqnException(120, Reach, "land", "groundwater mineralisation",
                            Qsw, Vsw, pars.TC[1]);
    }

    try
    {
        // Groundwater denitrification
        xp[32] = (subPar.Par[2] * x0[3] * Inputs[16]) / x0[7] * 1000000;
    }
    catch (std::exception& E)
    {
        throw EqnException(120, Reach, "land", "groundwater mineralisation",
                            Qsw, Vsw, pars.TC[1]);
    }

    // Total ON input
    try
    {
        xp[33] = Inputs[17] * 100.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(120, Reach, "land", "immobilisation",
                            Qsw, Vsw, pars.TC[1]);
    }

    Outputs[16] = Qsw;
}

//---------------------------------------------------------------------------
void reqn(std::vector<double>& x0,
            std::vector<double>& xp,
            std::vector<double>& Inputs,
            std::vector<double>& Outputs,
            CCellPar& pars,
            CSubPar& subPar,
            CReachPar& reachPar,
            std::vector<double>& Conc,
            bool& Abstracted,
            double Msoil)
{

int Reach = 0;

            Conc[0] = x0[1] * 1000.0 / Inputs[9];
            Conc[1] = x0[2] * 1000.0 / Inputs[9];
            Conc[2] = x0[14] * 1000.0 / Inputs[9];

    double AbstractionFlow = Inputs[8];
    double AbstractionNO3Mass = AbstractionFlow * Conc[0] * 86.4;
    double AbstractionNH4Mass = AbstractionFlow * Conc[1] * 86.4;
    double AbstractionDONMass = AbstractionFlow * Conc[2] * 86.4;

    double flow = (Inputs[0] + x0[0]);
    double no3 = (Inputs[1] + x0[1]);
    double nh4 = (Inputs[2] + x0[2]);
    double don = (Inputs[12] + x0[14]);



    if (AbstractionFlow < 0.0000001
            || AbstractionFlow >= flow
            || AbstractionNO3Mass >= no3
            || AbstractionNH4Mass >= nh4
            || AbstractionDONMass >= don)
    {
        AbstractionFlow = 0.0;
        AbstractionNO3Mass = 0.0;
        AbstractionNH4Mass = 0.0;
        AbstractionDONMass = 0.0;
        Abstracted = false;
    }
    else
    {
        Abstracted = true;
    }

    flow -= AbstractionFlow;
    no3 -= AbstractionNO3Mass;
    nh4 -= AbstractionNH4Mass;
    don -= AbstractionDONMass;

    double inputFlow = Inputs[0];// - AbstractionFlow;
    inputFlow *= 86400.0;


/*    try
    {
        Inputs[4] = x0[7] / (x0[0] * 86400.0);
    }
    catch (std::exception& E)
    {
        throw EqnException(199, Reach+1, "instream", "volume",
                            x0[0], x0[7], Inputs[4], Inputs[0], x0[0]);
    }

    try
    {
        // Instream flow
        xp[0] = (inputFlow - x0[0])
                / (Inputs[4] * (1.0 - reachPar.Par[2]));

//        if (x0[0] < 0.000001) xp[0] = std::max(double(0.0), xp[0]);
    }
    catch (std::exception& E)
    {
        throw EqnException(200, Reach+1, "instream", "flow",
                            x0[0], x0[7], Inputs[4], Inputs[0], x0[0]);
    }
 */


//    Inputs[9]     // Volume
//    Inputs[10]    // Velocity


    double ReachVolume = 0.0;

    if (Inputs[10] > 0.0)
    {
        double temp = std::exp( -86400.0 / ( reachPar.Par[0] / Inputs[10] ) );

        ReachVolume = Inputs[9] * temp
                        + ( reachPar.Par[0] / ( 86400.0 * Inputs[10] ) )
                            * inputFlow * ( 1.0 - temp );
    }
    else
    {
        ReachVolume = 1.0;
    }

    double ReachFlow = ( Inputs[9] + inputFlow - ReachVolume ) / 86400.0;

    double ReachWidth = reachPar.Par[1] * std::pow( ReachFlow, reachPar.Par[2] );

    double ReachDepth = reachPar.Par[11] * std::pow( ReachFlow, reachPar.Par[12] );

    double ReachArea = ReachDepth * ( ( ReachWidth + reachPar.Par[13] ) / 2.0 );

    double ReachPerimeter = reachPar.Par[13]
                                + 2.0 * std::pow( ( std::pow( ( ( ReachWidth - reachPar.Par[13] ) / 2.0 ), 2.0 ) + std::pow( ReachDepth, 2.0 ) ), 0.5 );

    double ReachRadius = ReachArea / ReachPerimeter;

    double ReachVelocity = 1.0 / reachPar.Par[14]
                        * std::pow( ReachRadius, ( 2.0 / 3.0 ) )
                        * std::pow( reachPar.Par[15], 0.5 );

    double ReachResidenceTime = reachPar.Par[0] / ( 86400.0 * ReachVelocity );






    try
    {
        // Instream nitrate
        xp[1] = Inputs[1] - (x0[1] * ReachFlow * 86400.0) / ReachVolume
                - reachPar.Par[3] * Conc[0] * Inputs[3] * ReachVolume / 1000.0
                + reachPar.Par[4] * Conc[1] * Inputs[3] * ReachVolume / 1000.0
                - AbstractionNO3Mass;
    }
    catch (std::exception& E)
    {
        throw EqnException(201, Reach+1, "instream", "nitrate",
                            ReachFlow, Inputs[9], ReachResidenceTime, Inputs[1], x0[1]);
    }

    try
    {
        // Instream ammonium
        xp[2] = Inputs[2] - (x0[2] * ReachFlow * 86400.0) / ReachVolume
                // nitrification
                - reachPar.Par[4] * Conc[1] * Inputs[3] * ReachVolume / 1000.0
                // mineralization
                + reachPar.Par[16] * Conc[2] * Inputs[3] * ReachVolume / 1000.0
                // immobilization
                - reachPar.Par[17] * Conc[1] * Inputs[3] * ReachVolume / 1000.0
                - AbstractionNH4Mass;
    }
    catch (std::exception& E)
    {
        throw EqnException(202, Reach+1, "instream", "ammonium",
                            ReachFlow, ReachVolume, ReachResidenceTime, Inputs[2], x0[2]);
    }

    try
    {
        // Total NO3 input
        xp[3] = Inputs[1];
    }
    catch (std::exception& E)
    {
        throw EqnException(203, Reach+1, "instream", "total nitrate input",
                            ReachFlow, ReachVolume, ReachResidenceTime);
    }

    try
    {
        // Total NO3 output
        xp[4] = (x0[1] * ReachFlow * 86400.0) / ReachVolume;// + AbstractionNO3Mass;
    }
    catch (std::exception& E)
    {
        throw EqnException(204, Reach+1, "instream", "total nitrate output",
                            ReachFlow, ReachVolume, ReachResidenceTime);
    }

    try
    {
        // Total NH4 input
        xp[5] = Inputs[2];
    }
    catch (std::exception& E)
    {
        throw EqnException(205, Reach+1, "instream", "total ammonium input",
                            ReachFlow, ReachVolume, ReachResidenceTime);
    }

    try
    {
        // Total NH4 output
        xp[6] = (x0[2] * ReachFlow * 86400.0) / ReachVolume;// + AbstractionNH4Mass;
    }
    catch (std::exception& E)
    {
        throw EqnException(206, Reach+1, "instream", "total ammonium output",
                            ReachFlow, ReachVolume, ReachResidenceTime);
    }

//    try
 //   {
        // Reach volume
 //       xp[7] = (inputFlow - ReachFlow) * 86400.0;
 //   }
//    catch (std::exception& E)
//    {
//        throw EqnException(207, Reach+1, "instream", "volume",
//                            ReachFlow, ReachVolume, ReachResidenceTime);
//    }

    try
    {
        // Total water input
        xp[8] = inputFlow * 86400.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(208, Reach+1, "instream", "total water input",
                            ReachFlow, ReachVolume, ReachResidenceTime);
    }

    try
    {
        // Total water output
        xp[9] = ReachFlow * 86400.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(209, Reach+1, "instream", "total water output",
                            ReachFlow, ReachVolume, ReachResidenceTime);
    }

    try
    {
        // Denitrification
        xp[10] = reachPar.Par[3] * Conc[0] * Inputs[3] * ReachVolume / 1000.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(210, Reach+1, "instream", "denitrification",
                            ReachFlow, ReachVolume, ReachResidenceTime);
    }

    try
    {
        // Nitrification
        xp[11] = reachPar.Par[4] * Conc[1] * Inputs[3] * ReachVolume / 1000.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(211, Reach+1, "instream", "nitrification",
                            ReachFlow, ReachVolume, ReachResidenceTime);
    }

    try
    {
        // Nitrate storage
        xp[12] = (xp[1] * ReachVolume + xp[7] * x0[1]) / 1000.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(212, Reach+1, "instream", "nitrate storage",
                            ReachFlow, ReachVolume, ReachResidenceTime);
    }

    try
    {
        // Ammonium storage
        xp[13] = (xp[2] * ReachVolume + xp[7] * x0[2]) / 1000.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(213, Reach+1, "instream", "ammonium storage",
                            ReachFlow, ReachVolume, ReachResidenceTime);
    }

    // Change in Stream Dissolved Organic Nitrogen
    xp[14] = Inputs[12]
                // reach outflow
		        - (ReachFlow * x0[14] * 86400) / ReachVolume
                // mineralization
                - reachPar.Par[16] * Conc[2] * Inputs[3] * ReachVolume / 1000.0
                // immobilization
                + reachPar.Par[17] * Conc[1] * Inputs[3] * ReachVolume / 1000.0
                - AbstractionDONMass;

    try
    {
        // Total DON input
        xp[15] = Inputs[12];
    }
    catch (std::exception& E)
    {
        throw EqnException(205, Reach+1, "instream", "total DON input",
                            ReachFlow, ReachVolume, ReachResidenceTime);
    }

    try
    {
        // Total DON output
        xp[16] = (ReachFlow * x0[14] * 86400) / ReachVolume;
    }
    catch (std::exception& E)
    {
        throw EqnException(206, Reach+1, "instream", "total DON output",
                            ReachFlow, ReachVolume, ReachResidenceTime);
    }

    try
    {
        // DON storage
        xp[17] = (xp[14] * ReachVolume + xp[7] * x0[14]) / 1000.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(213, Reach+1, "instream", "DON storage",
                            ReachFlow, ReachVolume, ReachResidenceTime);
    }

    try
    {
        // mineralization
        xp[18] = reachPar.Par[16] * Conc[2] * Inputs[3] * ReachVolume / 1000.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(210, Reach+1, "instream", "mineralization",
                            ReachFlow, ReachVolume, ReachResidenceTime);
    }

    try
    {
        // immobilization
        xp[19] = reachPar.Par[17] * Conc[1] * Inputs[3] * ReachVolume / 1000.0;
    }
    catch (std::exception& E)
    {
        throw EqnException(211, Reach+1, "instream", "immobilization",
                            ReachFlow, ReachVolume, ReachResidenceTime);
    }

    Inputs[9] = ReachVolume;
    Inputs[10] = ReachVelocity;
    Inputs[11] = ReachFlow;

    Outputs[2] = ReachWidth;
    Outputs[3] = ReachDepth;
    Outputs[4] = ReachArea;
    Outputs[5] = ReachPerimeter;
    Outputs[6] = ReachRadius;
    Outputs[7] = ReachResidenceTime;
}

//---------------------------------------------------------------------------
std::ostream& EqnException::print(std::ostream& os) const
{
    os << "INCA-ON (THE) failed on sub-catchment/reach " << reach << ", timestep " << timestep << std::endl;
    os << "with error code " << code << " (" << type << " phase, " << eqn << " equation)\n\n";
    os << "Status:\n\n";
    os << "Flow\tVolume\tTime Constant\tInput\tValue\n";
    os << "----\t------\t-------------\t-----\t-----\n";
    os << flow << "\t" << volume << "\t" << tc << "\t" << input << "\t" << value << std::endl;

    return os;
}

