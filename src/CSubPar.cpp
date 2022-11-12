//---------------------------------------------------------------------------
#include <sstream>

#include "CSubPar.h"
#include "Constant.h"

//---------------------------------------------------------------------------

CSubPar::CSubPar()
{
    Init(6);
}
//---------------------------------------------------------------------------

CSubPar::CSubPar(unsigned int landCount)
{
    Init(landCount);
}
//---------------------------------------------------------------------------
CSubPar::CSubPar(CSubPar& rhs)
{
    Init(rhs.Land.size());

    Par = rhs.Par;
    Land = rhs.Land;
    Dep = rhs.Dep;
}
//---------------------------------------------------------------------------
CSubPar::CSubPar(const CSubPar& rhs)
{
    Init(rhs.Land.size());

    Par = rhs.Par;
    Land = rhs.Land;
    Dep = rhs.Dep;
}
//---------------------------------------------------------------------------
void CSubPar::Init(unsigned int landCount)
{
    Par.Init(NUM_SUB_PAR);
    Land.Init(landCount);
    Dep.Init(4);

    // Exclude all land use percentages from sensitivity analysis
    Land.LatinHypercubeExcludeAll(true);

    // Exclude subcatchment area from sensitivity analysis
    Par.LatinHypercubeExclude(true, 0);

    // Set missing value for groundwater sustainable flow
    Par.SetMissingValue(9999.0, 12);

    Par(0).Name("Area");
    Par(1).Name("Base flow index");
    Par(2).Name("Groundwater denitrification rate");
    Par(3).Name("Maximum groundwater effective depth");
    Par(4).Name("Proportion of filled pore space");
    Par(5).Name("Groundwater residence time");
    Par(6).Name("Initial groundwater flow");
    Par(7).Name("Initial groundwater nitrate");
    Par(8).Name("Initial groundwater ammonium");
    Par(9).Name("Minimum groundwater flow");
    Par(10).Name("Groundwater organic N mineralisation rate");
    Par(11).Name("Initial groundwater organic N");
    Par(12).Name("Groundwater nitrification rate");
    Par(13).Name("Subcatchment specific rain multiplier");
    Par(14).Name("Subcatchment specific snow multiplier");
    Par(15).Name("Subcatchment specific air temperature offset");
    Par(16).Name("Subcatchment specific snowmelt offset");

    Par(17).Name("Threshold soil zone flow");
    Par(18).Name("Rainfall excess proportion");

    Dep(0).Name("Nitrate dry deposition");
    Dep(1).Name("Nitrate wet deposition");
    Dep(2).Name("Ammonium dry deposition");
    Dep(3).Name("Ammonium wet deposition");

    for (unsigned int i=0; i<landCount; ++i)
    {
//        std::string name;
//        name = "land " + (i+1);

    //    std::ostringstream str;
   //     str << "land " << (i+1);

  //      std::string name = str.str();

        Land(i).Name("Landscape unit percent");
//        Land(i).Name(str.str());
    }
}
//---------------------------------------------------------------------------
CSubPar& CSubPar::operator=(const CSubPar& rhs)
{
    if (&rhs != this)
    {
        Init(rhs.Land.size());

        Par = rhs.Par;
        Land = rhs.Land;
        Dep = rhs.Dep;
    }

    return *this;
}
//---------------------------------------------------------------------------
CSubPar operator*(const CSubPar& p, const double& t)
{
    CSubPar par(p);

    par.Par = p.Par * t;
    par.Dep = p.Dep * t;

    return par;
}
//---------------------------------------------------------------------------
CSubPar operator-(const CSubPar& p, const CSubPar& n)
{
    CSubPar par(p);

    par.Par = p.Par - n.Par;
    par.Dep = p.Dep - n.Dep;

    return par;
}
//---------------------------------------------------------------------------
bool operator>(const CSubPar& lhs, const CSubPar& rhs)
{
    if (lhs.Par <= rhs.Par) return false;
    if (lhs.Dep <= rhs.Dep) return false;
    if (lhs.Land <= rhs.Land) return false;

    return true;
}
//---------------------------------------------------------------------------
unsigned int CSubPar::IncludedCount(bool countAll)
{
    return (Par.IncludedCount(countAll)
            + Land.IncludedCount(countAll)
            + Dep.IncludedCount(countAll));
}
//---------------------------------------------------------------------------
double CSubPar::GetParByIndex(const unsigned int index, bool all)
{
    unsigned int pCount = Par.IncludedCount(all);
    unsigned int lCount = Land.IncludedCount(all);

    if (index < pCount)
    {
        return Par.GetParByIndex(index, all);
    }

    if (index < (pCount + lCount))
    {
        return Land.GetParByIndex((index - pCount), all);
    }

    return Dep.GetParByIndex((index - pCount - lCount), all);
}
//---------------------------------------------------------------------------
const std::string& CSubPar::GetNameByIndex(const unsigned int index, bool all)
{
    unsigned int pCount = Par.IncludedCount(all);
    unsigned int lCount = Land.IncludedCount(all);

    if (index < pCount)
    {
        return Par.GetNameByIndex(index, all);
    }

    if (index < (pCount + lCount))
    {
        return Land.GetNameByIndex((index - pCount), all);
    }

    return Dep.GetNameByIndex((index - pCount - lCount), all);
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const CSubPar& p)
{
    os << "PARAMETERS" << std::endl;
    os << p.Par;

    os << "LAND" << std::endl;
    os << p.Land;

    os << "DEPOSITION" << std::endl;
    os << p.Dep;

    return os;
}
//---------------------------------------------------------------------------
void CSubPar::writeXml ( tinyxml2::XMLNode * node, std::vector<std::string> landNames ) const
{
    Par.writeXml( node );
    Dep.writeXml( node );
//    Land.writeXml( node, "Landscape percentages", "Landscape units" );

    tinyxml2::XMLNode * group = node->GetDocument()->NewElement( "ParameterGroup" );
    group->ToElement()->SetAttribute( "name", "Landscape percentages" );
    group->ToElement()->SetAttribute( "indexer", "Landscape units" );
    group->ToElement()->SetAttribute( "count", (unsigned int)Land.size() );

    for (unsigned int i=0; i<Land.size(); ++i)
    {
        tinyxml2::XMLNode * groupItem = group->GetDocument()->NewElement( "ParameterContainer" );
        groupItem->ToElement()->SetAttribute( "name", landNames[i].c_str() );
        groupItem->ToElement()->SetAttribute( "count", "1" );
        Land(i).writeXml( groupItem );
        group->InsertEndChild( groupItem );
    }

    node->InsertEndChild( group );

}
//---------------------------------------------------------------------------
void CSubPar::readXml ( tinyxml2::XMLNode * node )
{
    tinyxml2::XMLNode * parameter = node->NextSiblingElement( "Parameter" );

    parameter = Par.readXml( parameter );
    parameter = Dep.readXml( parameter, false );
    Land.readXml( parameter, "Landscape percentages", "Landscape units" );
}
//---------------------------------------------------------------------------


