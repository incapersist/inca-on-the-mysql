#include <cstdlib>
#include <cmath>

#include "CPar.h"
#include "tinyxml2.h"

//---------------------------------------------------------------------------
CPar::CPar(std::string str,
            double defaultValue,
            double minValue,
            double maxValue)//,
     //       char const* typeName) : typeName_(typeName)
{
    name = str;
    units = "";
//    value = 0.0;
    excluded = false;
    inLH = false;
    LHmin = 0.0;
    LHmax = 0.0;
    LHrange = 0.0;
    missingValue = 9999.0;
    smallestAllowedRange = 0.000000000001;
    defaultValue_ = defaultValue;
    value_ = defaultValue;

    lh.clear();
}
//---------------------------------------------------------------------------
/*CPar::CPar(CPar& original) //: Parameter(original)
{
    copy(original);
}
//---------------------------------------------------------------------------
CPar::CPar(const CPar& original) //: Parameter(original)
{
    copy(original);
}
//---------------------------------------------------------------------------
CPar& CPar::operator=(const CPar& original)
{
    if (&original != this)
    {
        copy(original);
    }

    return *this;
}
//---------------------------------------------------------------------------
void CPar::copy(const CPar& rhs)
{
    name = rhs.name;
    units = rhs.units;
    lh = rhs.lh;
    value = rhs.value;
    missingValue = rhs.missingValue;
    smallestAllowedRange = rhs.smallestAllowedRange;

    excluded = rhs.excluded;
    inLH = rhs.inLH;
    LHmin = rhs.LHmin;
    LHmax = rhs.LHmax;
    LHrange = rhs.LHrange;
}          */
//---------------------------------------------------------------------------
CPar operator*(const CPar& p, const double& t)
{
    CPar par(p);

    if (p.LatinHypercubeInProgress())
    {
        if (!p.LatinHypercubeExcluded() && !p.hasMissingValue())
        {
            par.Value2(p.Value2() * t);
        }
    }
    else
    {
        if (!p.hasMissingValue())
        {
            par.Value2(p.Value2() * t);
        }
    }

    return par;
}
//---------------------------------------------------------------------------
CPar operator-(const CPar& p, const CPar& n)
{
    CPar par(p);

    if (p.LatinHypercubeInProgress())
    {
        if (!p.LatinHypercubeExcluded() && !p.hasMissingValue())
        {
            par.Value2(p.Value2() - n.Value2());
        }
    }
    else
    {
        if (!p.hasMissingValue())
        {
            par.Value2(p.Value2() - n.Value2());
        }
    }

    return par;
}
//---------------------------------------------------------------------------
bool operator>(const CPar& lhs, const CPar& rhs)
{
    if (lhs.hasMissingValue() || rhs.hasMissingValue()) return true;
    return (lhs.value_ > rhs.value_);
}
//---------------------------------------------------------------------------
bool operator<=(const CPar& lhs, const CPar& rhs)
{
    if (lhs.hasMissingValue() || rhs.hasMissingValue()) return true;
    return (lhs.value_ <= rhs.value_);
}
//---------------------------------------------------------------------------
void CPar::LatinHypercubeInit(unsigned int numGroups)
{
    lh.clear();
    lh.resize(numGroups);
    inLH = true;
}
//---------------------------------------------------------------------------
void CPar::LatinHypercubeSetRange(CPar& min, CPar& max)
{
    if (!excluded && !hasMissingValue())
    {
        LHmin = min.Value2();
        LHmax = max.Value2();
        LHrange = LHmax - LHmin;
    }
}
//---------------------------------------------------------------------------
void CPar::LatinHypercubeGenerate(void)
{
    if (!excluded)
    {
        int temp, randomNum;

        for (unsigned int i=0; i<lh.size(); ++i)
        {
            lh[i] = i;
        }

        for (int i=lh.size(); i>1; --i)
        {
            randomNum = rand() % i;
            temp = lh[randomNum];
            lh[randomNum] = lh[i-1];
            lh[i-1] = temp;
        }
    }
}
//---------------------------------------------------------------------------
void CPar::LatinHypercubeGetSample(unsigned int index)
{
    if (!excluded && fabs(LHrange) > smallestAllowedRange && !hasMissingValue())
    {
	    double tmp = ((double)lh[index] + (double)rand() / RAND_MAX) / lh.size();
	    value_ = value_ + tmp * LHrange;
    }
}
//---------------------------------------------------------------------------
void CPar::ExcludeZeroRange(void)
{
    excluded = (fabs(LHrange) < smallestAllowedRange);
}
//---------------------------------------------------------------------------
const bool CPar::hasMissingValue(void) const
{
    return (value_ > (missingValue - 0.000001) && value_ < (missingValue + 0.000001));
//    return (value == missingValue);
}
//---------------------------------------------------------------------------
void CPar::WriteLH(FILE* fp)
{
    for (unsigned int i=0; i<lh.size(); ++i)
    {
        fprintf(fp, "%d", lh[i]);
        if (i < lh.size() - 1) fprintf(fp, ",");
    }

    fprintf(fp, ":%lf:%lf:%lf\n", LHmin, LHmax, LHrange);
}
//---------------------------------------------------------------------------
/*void CPar::Name(char* cn)
{
    std::string n(cn);
//    label_ = n;
    name = n;
}*/
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const CPar& p)
{
    os << p.Value2();
//    if (!p.LatinHypercubeExcluded()) os << p.Value2();
//    if (p.LatinHypercubeExcluded()) os << "*";

    return os;
}
//---------------------------------------------------------------------------
void CPar::writeXml ( tinyxml2::XMLNode * node ) const
{
    tinyxml2::XMLElement * parameter = node->GetDocument()->NewElement( "Parameter" );

    parameter->SetAttribute( "name", name.c_str() );
    parameter->SetAttribute( "units", units.c_str() );
 //   parameter->SetAttribute( "Type", typeName_.c_str() );

    tinyxml2::XMLElement * defaultValue = node->GetDocument()->NewElement( "defaultValue" );
    defaultValue->SetText( defaultValue_ );
    parameter->InsertEndChild( defaultValue );

    tinyxml2::XMLElement * currentValue = node->GetDocument()->NewElement( "currentValue" );
    currentValue->SetText( value_ );
    parameter->InsertEndChild( currentValue );

    tinyxml2::XMLElement * minimumValue = node->GetDocument()->NewElement( "minimumValue" );
    minimumValue->SetText( minimumValue_ );
    parameter->InsertEndChild( minimumValue );

    tinyxml2::XMLElement * maximumValue = node->GetDocument()->NewElement( "maximumValue" );
    maximumValue->SetText( maximumValue_ );
    parameter->InsertEndChild( maximumValue );

    node->InsertEndChild( parameter );
}
//---------------------------------------------------------------------------
void CPar::readXml( tinyxml2::XMLNode * node )
{
//    float val;

//    tinyxml2::XMLNode * parameter = node->FirstChildElement( "Value" );
//    tinyxml2::XMLError eResult = parameter->ToElement()->QueryFloatText( &val );
    defaultValue_   = getXmlValue( node, "defaultValue" );
    value_          = getXmlValue( node, "currentValue" );
    minimumValue_   = getXmlValue( node, "minimumValue" );
    maximumValue_   = getXmlValue( node, "maximumValue" );
}
//---------------------------------------------------------------------------
float CPar::getXmlValue( tinyxml2::XMLNode * node, const char* attribute )
{
    float val;

    tinyxml2::XMLNode * parameter = node->FirstChildElement( attribute );
    parameter->ToElement()->QueryFloatText( &val );

    return val;
}
//---------------------------------------------------------------------------

