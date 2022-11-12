//---------------------------------------------------------------------------
#include <sstream>

#include "CRiverPar.h"
#include "Constant.h"

//---------------------------------------------------------------------------
CRiverPar::CRiverPar()
{
    Initial.Init(NUM_RIVER_INITIAL);
    Pars.Init(NUM_RIVER_PARS);

/*    Initial.LatinHypercubeExclude(true, 3);
    Initial.LatinHypercubeExclude(true, 4);
    Initial.LatinHypercubeExclude(true, 5);
    Initial.LatinHypercubeExclude(true, 6);
    Initial.LatinHypercubeExclude(true, 7);
    Initial.LatinHypercubeExclude(true, 8);
    Initial.LatinHypercubeExclude(true, 9);
    Initial.LatinHypercubeExclude(true, 10);
    Initial.LatinHypercubeExclude(true, 11);
    Initial.LatinHypercubeExclude(true, 12);
    Initial.LatinHypercubeExclude(true, 13);
*/
    Initial(0).Name("Initial instream flow");
    Initial(1).Name("Initial instream nitrate");
    Initial(2).Name("Initial instream ammonium");
    Initial(3).Name("Initial instream organic N");

    Pars(0).Name("Minimum water temperature");
}
//---------------------------------------------------------------------------
/*CRiverPar::CRiverPar(CRiverPar& rhs)
{
    Initial = rhs.Initial;
    Pars = rhs.Pars;
}

//---------------------------------------------------------------------------
CRiverPar::CRiverPar(const CRiverPar& rhs)
{
    Initial = rhs.Initial;
    Pars = rhs.Pars;
}
//---------------------------------------------------------------------------
CRiverPar& CRiverPar::operator=(const CRiverPar& rhs)
{
    if (&rhs != this)
    {
        Initial = rhs.Initial;
        Pars = rhs.Pars;
    }

    return *this;
}          */
//---------------------------------------------------------------------------
CRiverPar operator*(const CRiverPar& p, const double& t)
{
    CRiverPar par(p);

    par.Initial = p.Initial * t;
    par.Pars = p.Pars * t;

    return par;
}
//---------------------------------------------------------------------------
CRiverPar operator-(const CRiverPar& p, const CRiverPar& n)
{
    CRiverPar par(p);

    par.Initial = p.Initial - n.Initial;
    par.Pars = p.Pars - n.Pars;

    return par;
}
//---------------------------------------------------------------------------
bool operator<=(const CRiverPar& lhs, const CRiverPar& rhs)
{
    return (lhs.Initial <= rhs.Initial && lhs.Pars <= rhs.Pars);
}
//---------------------------------------------------------------------------
unsigned int CRiverPar::IncludedCount(bool countAll)
{
    return (Initial.IncludedCount(countAll)
            + Pars.IncludedCount(countAll));
}
//---------------------------------------------------------------------------
double CRiverPar::GetParByIndex(const unsigned int index, bool all)
{
    unsigned int iCount = Initial.IncludedCount(all);

    if (index < iCount)
    {
        return Initial.GetParByIndex(index, all);
    }

    return Pars.GetParByIndex((index - iCount), all);
}
//---------------------------------------------------------------------------
const std::string& CRiverPar::GetNameByIndex(const unsigned int index, bool all)
{
    unsigned int iCount = Initial.IncludedCount(all);

    if (index < iCount)
    {
        return Initial.GetNameByIndex(index, all);
    }

    return Pars.GetNameByIndex((index - iCount), all);
}
//---------------------------------------------------------------------------
std::istream& CRiverPar::serialize(std::istream& in)
{
    for (int i=0; i<NUM_RIVER_INITIAL; ++i)
    {
        std::string input;
        getline(in, input);
        std::istringstream is(input);

        is >> Initial[i];
    }

    for (int i=0; i<NUM_RIVER_PARS; ++i)
    {
        std::string input;
        getline(in, input);
        std::istringstream is(input);

        is >> Pars[i];
    }

    return in;
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const CRiverPar& p)
{
    for (int i=0; i<NUM_RIVER_INITIAL; ++i)
    {
        os << p.Initial[i] << " ";
    }
    os << std::endl;

    for (int i=0; i<NUM_RIVER_PARS; ++i)
    {
        os << p.Pars[i] << " ";
    }
    os << std::endl;

    return os;
}
//---------------------------------------------------------------------------
void CRiverPar::writeXml ( tinyxml2::XMLNode * node ) const
{
    Initial.writeXml( node );
    Pars.writeXml( node );
}
//---------------------------------------------------------------------------
void CRiverPar::readXml ( tinyxml2::XMLNode * node )
{
    tinyxml2::XMLNode * parameter = node->FirstChildElement( "Parameter" );

    parameter = Initial.readXml( parameter );
    parameter = Pars.readXml( parameter, false );
}
//---------------------------------------------------------------------------

std::istream& operator>>(std::istream& in, CRiverPar& par)
{
	par.serialize(in);
	return in;
}

