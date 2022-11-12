//---------------------------------------------------------------------------
#include "CCellPar.h"
#include "Constant.h"

//---------------------------------------------------------------------------

CCellPar::CCellPar()
{
//    Initial.Init(NUM_CELL_EQN);
    Initial.Init(NUM_CELL_INITIAL);
    Par.Init(NUM_CELL_PARS);
    TC.Init(NUM_CELL_TC);

    // Set missing value for denitrification temperature threshold
    Par.SetMissingValue(9999.0, 16);
    // Set missing value for nitrification temperature threshold
    Par.SetMissingValue(9999.0, 17);
    // Set missing value for mineralisation temperature threshold
    Par.SetMissingValue(9999.0, 18);
    // Set missing value for immobilisation temperature threshold
    Par.SetMissingValue(9999.0, 19);
    // Set missing value for soil water sustainable flow
    Par.SetMissingValue(9999.0, 20);
    // Set missing value for direct runoff sustainable flow
    Par.SetMissingValue(9999.0, 21);

    // Exclude initial values from LH
/*    Initial.LatinHypercubeExclude(true, 6);
    Initial.LatinHypercubeExclude(true, 7);
    Initial.LatinHypercubeExclude(true, 8);
    Initial.LatinHypercubeExclude(true, 9);
    Initial.LatinHypercubeExclude(true, 10);
    Initial.LatinHypercubeExclude(true, 11);
    Initial.LatinHypercubeExclude(true, 12);
    Initial.LatinHypercubeExclude(true, 13);
    Initial.LatinHypercubeExclude(true, 14);
    Initial.LatinHypercubeExclude(true, 15);
    Initial.LatinHypercubeExclude(true, 16);
    Initial.LatinHypercubeExclude(true, 17);
    Initial.LatinHypercubeExclude(true, 18);
    Initial.LatinHypercubeExclude(true, 19);
    Initial.LatinHypercubeExclude(true, 20);
    Initial.LatinHypercubeExclude(true, 21);
    Initial.LatinHypercubeExclude(true, 22);
    Initial.LatinHypercubeExclude(true, 23);
    Initial.LatinHypercubeExclude(true, 24);
*/
    Initial(0).Name("Initial direct runoff");
    Initial(1).Name("Initial soil water flow");
    Initial(2).Name("Initial direct runoff nitrate");
    Initial(3).Name("Initial soil water nitrate");
    Initial(4).Name("Initial direct runoff ammonium");
    Initial(5).Name("Initial soil water ammonium");
    Initial(6).Name("Initial soil water DON");
    Initial(7).Name("Initial soil solid organic nitrogen");
    Initial(8).Name("Initial direct runoff DON");
    Initial(9).Name("Initial soil water depth");

    Par(0).Name("Soil water denitrification rate");
    Par(1).Name("Nitrogen fixation rate");
    Par(2).Name("Nitrate uptake rate");
    Par(3).Name("Maximum nitrogen uptake rate");
    Par(4).Name("Nitrate addition rate");
    Par(5).Name("Ammonium nitrification rate");
    Par(6).Name("Ammonium mineralisation rate");
    Par(7).Name("Ammonium immobilisation rate");
    Par(8).Name("Ammonium addition rate");
    Par(9).Name("Ammonium uptake rate");
    Par(10).Name("Growth season start day");
    Par(11).Name("Growth period");
    Par(12).Name("Fertiliser addition start day");
    Par(13).Name("Fertiliser addition period");
    Par(14).Name("Maximum soil moisture deficit");
    Par(15).Name("Maximum temperature difference");
    Par(16).Name("Stop denitrification at");
    Par(17).Name("Stop nitrification at");
    Par(18).Name("Stop mineralisation at");
    Par(19).Name("Stop immobilisation at");
    Par(20).Name("Minimum soil water flow");
    Par(21).Name("Minimum direct runoff flow");
    Par(22).Name("Denitrification response to a 10 degree change in temperature");
    Par(23).Name("Denitrification base temperature response");
    Par(24).Name("Initial snow pack depth");
    Par(25).Name("Degree day factor for snowmelt");
    Par(26).Name("Water equivalent factor");
    Par(27).Name("Snow depth / soil temperature factor");
    Par(28).Name("Thermal conductivity of soil");
    Par(29).Name("Specific heat capacity due to freeze/thaw");
    Par(30).Name("NO3 half-sat concentration");
    Par(31).Name("NH4 half-sat concentration");
    Par(32).Name("Fixation response to a 10 degree change in temperature");
    Par(33).Name("Fixation base temperature response");
    Par(34).Name("Nitrification response to a 10 degree change in temperature");
    Par(35).Name("Nitrification base temperature response");
    Par(36).Name("Mineralisation response to a 10 degree change in temperature");
    Par(37).Name("Mineralisation base temperature response");
    Par(38).Name("Immobilisation response to a 10 degree change in temperature");
    Par(39).Name("Immobilisation base temperature response");
    Par(40).Name("NO3 uptake response to a 10 degree change in temperature");
    Par(41).Name("NO3 uptake base temperature response");
    Par(42).Name("NH4 uptake response to a 10 degree change in temperature");
    Par(43).Name("NH4 uptake base temperature response");
    Par(44).Name("kSD");
    Par(45).Name("Soil water organic nitrogen 'n'");
    Par(46).Name("Kf");
    Par(47).Name("Organic N addition rate");
    Par(48).Name("Bulk porosity");

    Par(49).Name("Land cover specific rain multiplier");
    Par(50).Name("Land cover specific snow multiplier");
    Par(51).Name("Snowmelt offset");
    Par(52).Name("Interception");
    Par(53).Name("Plant growth temperature offset for PET");
    Par(54).Name("Solar radiation scaling factor for PET");
    Par(55).Name("Drought runoff proportion");
    Par(56).Name("Evapotranspiration exponent");
    Par(57).Name("SMD offset");
    Par(58).Name("Land cover specific threshold soil zone flow");
    Par(59).Name("Land cover specific rainfall excess proportion");
    Par(60).Name("Maximum infiltration rate");
    Par(61).Name("Infiltration temperature threshold");
    Par(62).Name("Inert depth");

    TC(0).Name("Direct runoff residence time");
    TC(1).Name("Soil water residence time");
    TC(2).Name("Ratio of total to available water in soil");
}
//---------------------------------------------------------------------------
CCellPar::CCellPar(CCellPar& CellPar)
{
    Initial = CellPar.Initial;
    Par = CellPar.Par;
    TC = CellPar.TC;
}
//---------------------------------------------------------------------------
CCellPar::CCellPar(const CCellPar& CellPar)
{
    Initial = CellPar.Initial;
    Par = CellPar.Par;
    TC = CellPar.TC;
}
//---------------------------------------------------------------------------
CCellPar& CCellPar::operator=(const CCellPar& rhs)
{
    if (&rhs != this)
    {
        Initial = rhs.Initial;
        Par = rhs.Par;
        TC = rhs.TC;
    }

    return *this;
}
//---------------------------------------------------------------------------
CCellPar operator*(const CCellPar& p, const double& t)
{
    CCellPar par(p);

    par.Initial = p.Initial * t;
    par.Par = p.Par * t;
    par.TC = p.TC * t;

    return par;
}
//---------------------------------------------------------------------------
CCellPar operator-(const CCellPar& p, const CCellPar& n)
{
    CCellPar par(p);

    par.Initial = p.Initial - n.Initial;
    par.Par = p.Par - n.Par;
    par.TC = p.TC - n.TC;

    return par;
}
//---------------------------------------------------------------------------
bool operator>(const CCellPar& lhs, const CCellPar& rhs)
{
    if (lhs.Initial <= rhs.Initial) return false;
    if (lhs.Par <= rhs.Par) return false;
    if (lhs.TC <= rhs.TC) return false;

    return true;
}
//---------------------------------------------------------------------------
unsigned int CCellPar::IncludedCount(bool countAll)
{
    return (Initial.IncludedCount(countAll)
            + Par.IncludedCount(countAll)
            + TC.IncludedCount(countAll));
}
//---------------------------------------------------------------------------
double CCellPar::GetParByIndex(const unsigned int index, bool all)
{
    unsigned int iCount = Initial.IncludedCount(all);
    unsigned int pCount = Par.IncludedCount(all);

    if (index < iCount)
    {
        return Initial.GetParByIndex(index, all);
    }

    if (index < (iCount + pCount))
    {
        return Par.GetParByIndex((index - iCount), all);
    }

    return TC.GetParByIndex((index - iCount - pCount), all);
}
//---------------------------------------------------------------------------
const std::string& CCellPar::GetNameByIndex(const unsigned int index, bool all)
{
    unsigned int iCount = Initial.IncludedCount(all);
    unsigned int pCount = Par.IncludedCount(all);

    if (index < iCount)
    {
        return Initial.GetNameByIndex(index, all);
    }

    if (index < (iCount + pCount))
    {
        return Par.GetNameByIndex((index - iCount), all);
    }

    return TC.GetNameByIndex((index - iCount - pCount), all);
}
//---------------------------------------------------------------------------
void CCellPar::writeXml ( tinyxml2::XMLNode * node ) const
{
    Initial.writeXml( node );
    Par.writeXml( node );
    TC.writeXml( node );
}
//---------------------------------------------------------------------------
void CCellPar::readXml ( tinyxml2::XMLNode * node )
{
    tinyxml2::XMLNode * parameter = node->FirstChildElement( "Parameter" );

    parameter = Initial.readXml( parameter );
    parameter = Par.readXml( parameter, false );
    parameter = TC.readXml( parameter, false );
}
//---------------------------------------------------------------------------

