//---------------------------------------------------------------------------
//#include <vcl.h>
#include <sstream>

#include "Constant.h"
#include "CReachPar.h"

//---------------------------------------------------------------------------
CReachPar::CReachPar()
{
    Par.Init(NUM_REACH_PAR);

    // Exclude reach length from LH
    Par.LatinHypercubeExclude(true, 0);

    Par(0).Name("Reach length");
    Par(1).Name("Flow a");
    Par(2).Name("Flow b");
    Par(3).Name("Reach denitrification rate");
    Par(4).Name("Reach nitrification rate");
    Par(5).Name("Effluent nitrate concentration");
    Par(6).Name("Effluent ammonium concentration");
    Par(7).Name("Effluent flow rate");
    Par(8).Name("Abstraction flow rate");
    Par(9).Name("Latitude");
    Par(10).Name("Longitude");
    Par(11).Name("Flow c");
    Par(12).Name("Flow f");
    Par(13).Name("Width at sediment surface");
    Par(14).Name("Manning's roughness");
    Par(15).Name("Slope");
    Par(16).Name("Reach mineralization rate");
    Par(17).Name("Reach immobilization rate");
    Par(18).Name("Effluent Organic N concentration");

    Par.Add("Effluent on/off switch", 0);
}
//---------------------------------------------------------------------------
CReachPar::CReachPar(CReachPar& rhs)
{
    Par = rhs.Par;
    Effluent = rhs.Effluent;
    name = rhs.name;
}
//---------------------------------------------------------------------------
CReachPar::CReachPar(const CReachPar& rhs)
{
    Par = rhs.Par;
    Effluent = rhs.Effluent;
    name = rhs.name;
}
//---------------------------------------------------------------------------
CReachPar operator*(const CReachPar& rhs, const double& t)
{
    CReachPar par(rhs);
    par.Par = rhs.Par * t;

    return par;
}
//---------------------------------------------------------------------------
CReachPar operator-(const CReachPar& p, const CReachPar& n)
{
    CReachPar par(p);
    par.Par = p.Par - n.Par;

    return par;
}

//---------------------------------------------------------------------------
CReachPar& CReachPar::operator=(const CReachPar& rhs)
{
    if (&rhs != this)
    {
        Par = rhs.Par;
        Effluent = rhs.Effluent;
        name = rhs.name;
    }

    return *this;
}
//---------------------------------------------------------------------------
bool operator>(const CReachPar& lhs, const CReachPar& rhs)
{
    if (lhs.Par <= rhs.Par) return false;

    return true;
}
//---------------------------------------------------------------------------
unsigned int CReachPar::IncludedCount(bool countAll)
{
    return Par.IncludedCount(countAll);
}
//---------------------------------------------------------------------------
double CReachPar::GetParByIndex(const unsigned int index, bool all)
{
    return Par.GetParByIndex(index, all);
}
//---------------------------------------------------------------------------
const std::string& CReachPar::GetNameByIndex(const unsigned int index, bool all)
{
    return Par.GetNameByIndex(index, all);
}
//---------------------------------------------------------------------------
std::istream& CReachPar::serialize(std::istream& in)
{
    std::string input;
    getline(in, input);
    std::istringstream is1(input);

    unsigned int eff;

    for(int i=0; i<5; ++i)
    {
        is1 >> Par[i];
    }

    getline(in, input);
    std::istringstream is2(input);

    for(int i=5; i<9; ++i)
    {
        is2 >> Par[i];
    }

    is2 >> eff;

    Effluent = (bool)eff;

    getline(in, input);
    std::istringstream is5(input);

    for(int i=9; i<NUM_REACH_PAR; ++i)
    {
        is5 >> Par[i];
    }

    return in;
}
//---------------------------------------------------------------------------
std::istream& CReachPar::ImportBranching(std::istream& in)
{
    std::string input;
    getline(in, input);
    std::istringstream is1(input);

    unsigned int eff;

    for(int i=0; i<5; ++i)
    {
        is1 >> Par[i];
    }

    getline(in, input);
    std::istringstream is2(input);

    for(int i=5; i<9; ++i)
    {
        is2 >> Par[i];
    }

    is2 >> eff;

    Effluent = (bool)eff;

    getline(in, input);
    std::istringstream is5(input);

    for(int i=9; i<11; ++i)
    {
        is5 >> Par[i];
    }
    
    Par[1] = 2.71;
    Par[2] = 0.557;
    Par[11] = 0.349;
    Par[12] = 0.371;
    Par[13] = 10.0;
    Par[14] = 0.01;
    Par[15] = 0.001;
    Par[16] = 0.0;
    Par[17] = 0.0;
    Par[18] = 0.0;

    return in;
}
//---------------------------------------------------------------------------
void CReachPar::Import(std::istream& in)
{
    std::string input;
    getline(in, input);
    std::istringstream is1(input);

    is1 >> name;
    is1 >> Par[0];
    is1 >> Par[1];
    is1 >> Par[2];
    is1 >> Par[3];
    is1 >> Par[4];
    is1 >> Par[5];
    is1 >> Par[6];
    is1 >> Par[7];
    is1 >> Par[8];
    is1 >> Effluent;

    Par[1] = 2.71;
    Par[2] = 0.557;
    Par[9] = 0.0;
    Par[10] = 0.0;
    Par[11] = 0.349;
    Par[12] = 0.371;
    Par[13] = 10.0;
    Par[14] = 0.01;
    Par[15] = 0.001;
    Par[16] = 0.0;
    Par[17] = 0.0;
    Par[18] = 0.0;
}
//---------------------------------------------------------------------------
void CReachPar::writeXml ( tinyxml2::XMLNode * node ) const
{
 //   Par(NUM_REACH_PAR).Value2( Effluent );
    Par.writeXml( node );
}
//---------------------------------------------------------------------------
tinyxml2::XMLNode * CReachPar::readXml ( tinyxml2::XMLNode * node )
{
    tinyxml2::XMLNode * parameter = node->FirstChildElement( "Parameter" );

    tinyxml2::XMLNode * newNode = Par.readXml( parameter );
    Effluent = Par(NUM_REACH_PAR).Value2();

    return newNode;
}
//---------------------------------------------------------------------------

std::istream& operator>>(std::istream& in, CReachPar& par)
{
	par.serialize(in);
	return in;
}


