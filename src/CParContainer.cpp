#include <cassert>
#include <sstream>

#include "CParContainer.h"
//#include <boost/any.hpp>

//---------------------------------------------------------------------------
CParContainer::CParContainer()
{
    pars.clear();
}
//---------------------------------------------------------------------------
CParContainer::CParContainer(int num)
{
    Init(num);
}
//---------------------------------------------------------------------------
void CParContainer::Init(int num)
{
    pars.clear();

    for (int i=0; i<num; ++i)
    {
        CPar par("empty");
        pars.push_back(par);
    }
}
//---------------------------------------------------------------------------
void CParContainer::Init(void)
{
    pars.clear();
}
//---------------------------------------------------------------------------
/*CParContainer::CParContainer(CParContainer& original)
{
//    pars.clear();

//    for (unsigned int i=0; i<original.pars.size(); ++i)
//    {
//        CPar par(original.pars[i]);
//        pars.push_back(par);
//    }

    pars = original.pars;
}
//---------------------------------------------------------------------------
CParContainer::CParContainer(const CParContainer& original)
{
//    pars.clear();

//    for (unsigned int i=0; i<original.pars.size(); ++i)
//    {
//        CPar par(original.pars[i]);
//        pars.push_back(par);
//    }

    pars = original.pars;
}
//---------------------------------------------------------------------------
CParContainer& CParContainer::operator=(const CParContainer& original)
{
    if (&original != this)
    {
//        pars.clear();

//        for (unsigned int i=0; i<original.pars.size(); ++i)
//        {
//            CPar par(original.pars[i]);
//            pars.push_back(par);
//        }

        pars = original.pars;
    }

    return *this;
}    */
//---------------------------------------------------------------------------
CParContainer operator*(const CParContainer& p, const double& t)
{
    CParContainer pars;

    for (CParContainer::const_iterator i = p.begin(); i != p.end(); ++i)
    {
        CPar par = *i * t;
        pars.Add(par);
    }

    return pars;
}
//---------------------------------------------------------------------------
CParContainer operator-(const CParContainer& p, const CParContainer& n)
{
    CParContainer pars;

    for (unsigned int i=0; i<p.size(); ++i)
    {
        CPar par = p.pars[i] - n.pars[i];
        pars.Add(par);
    }

    return pars;
}
//---------------------------------------------------------------------------
bool operator<=(const CParContainer& lhs, const CParContainer& rhs)
{
    assert(lhs.size() == rhs.size());

    for (unsigned int i=0; i<lhs.size(); ++i)
    {
        if (lhs(i) > rhs(i)) return false;
    }

    return true;
}
//---------------------------------------------------------------------------
double& CParContainer::operator[](unsigned int index)
{
    assert(index < pars.size());

    return pars[index].Value2();
}
//---------------------------------------------------------------------------
const double& CParContainer::operator[](unsigned int index) const
{
    assert(index < pars.size());

    return pars[index].Value2();
}
//---------------------------------------------------------------------------
CPar& CParContainer::operator()(unsigned int index)
{
    assert(index < pars.size());

    return pars[index];
}
//---------------------------------------------------------------------------
const CPar& CParContainer::operator()(unsigned int index) const
{
    assert(index < pars.size());

    return pars[index];
}
//---------------------------------------------------------------------------
void CParContainer::LatinHypercubeInit(unsigned int groups)
{
    // Initialise LH arrays in all parameters
    for (unsigned int i=0; i<pars.size(); ++i)
    {
        pars[i].LatinHypercubeInit(groups);
    }
}
//---------------------------------------------------------------------------
void CParContainer::LatinHypercubeSetRange(CParContainer& min, CParContainer& max)
{
    for (unsigned int i=0; i<pars.size(); ++i)
    {
        pars[i].LatinHypercubeSetRange(min.pars[i], max.pars[i]);
    }
}
//---------------------------------------------------------------------------
void CParContainer::LatinHypercubeGenerate(void)
{
    // Fill parameter LH arrays with random permutation
    for (unsigned int i=0; i<pars.size(); ++i)
    {
        pars[i].LatinHypercubeGenerate();
    }
}
//---------------------------------------------------------------------------
void CParContainer::LatinHypercubeGetSample(unsigned int group)
{
    // Use LH to get random samples from parameter space
    for (unsigned int i=0; i<pars.size(); ++i)
    {
        pars[i].LatinHypercubeGetSample(group);
    }
}
//---------------------------------------------------------------------------
void CParContainer::ExcludeZeroRangePars(void)
{
    // Mark as excluded any parameter with zero range
    for (unsigned int i=0; i<pars.size(); ++i)
    {
        pars[i].ExcludeZeroRange();
    }
}
//---------------------------------------------------------------------------
void CParContainer::Add(CPar& par)
{
    pars.push_back(par);
}
//---------------------------------------------------------------------------
//void CParContainer::Add(char* name, double val)
//{
//    std::string str(name);
//    Add(str, val);
//}
//---------------------------------------------------------------------------
void CParContainer::Add(char const* name,
                        double val,
                        double minValue,
                        double maxValue)
{
//    std::string str(name);
//    CPar par(name, val, typeName);
    CPar par(name, val, minValue, maxValue);
    Add(par);
}
//---------------------------------------------------------------------------
//void CParContainer::Add(std::string& name, double val)
//{
//    CPar par(name, val);
//    Add(par);
//}
//---------------------------------------------------------------------------
void CParContainer::AddGrainPars(const unsigned int gs,
                                    const std::vector<std::string>& gNames,
                                    const char *pre,
                                    const char *post,
                                    double val)
{
    for (unsigned int j=0; j<gs; ++j)
    {
        std::ostringstream str;
        str << pre << " " << gNames[j] << " " << post;

        Add(str.str().c_str(), val);
    }
}
//---------------------------------------------------------------------------
void CParContainer::LatinHypercubeExcludeAll(bool ex)
{
    for (unsigned int i=0; i<pars.size(); ++i)
    {
        pars[i].LatinHypercubeExcluded(ex);
    }
}
//---------------------------------------------------------------------------
void CParContainer::LatinHypercubeExclude(bool ex, unsigned int index)
{
    pars[index].LatinHypercubeExcluded(ex);
}
//---------------------------------------------------------------------------
void CParContainer::SetMissingValue(double missingValue, unsigned int index)
{
    pars[index].MissingValue(missingValue);
}
//---------------------------------------------------------------------------
void CParContainer::WriteLH(FILE* fp)
{
    for (unsigned int i=0; i<pars.size(); ++i)
    {
        pars[i].WriteLH(fp);
    }
}
//---------------------------------------------------------------------------
unsigned int CParContainer::IncludedCount(bool countAll)
{
    if (countAll) return pars.size();

    unsigned int count = 0;

    for (unsigned int i=0; i<pars.size(); ++i)
    {
        if (!pars[i].LatinHypercubeExcluded()) ++count;
    }

    return count;
}
//---------------------------------------------------------------------------
double CParContainer::GetParByIndex(const unsigned int index, bool all)
{
    assert (index < pars.size());

    unsigned int excludedCount = 0, trueIndex = 0;
    bool found = false;

    if (all)
    {
        return pars[index].Value2();
    }
    else
    {
        for (unsigned int i=0; i<pars.size(); ++i)
        {
            if (!pars[i].LatinHypercubeExcluded()) ++excludedCount;

            if (excludedCount == (index+1) && !found)
            {
                trueIndex = i;
                found = true;
            }
        }
    }

    return pars[trueIndex].Value2();
}
//---------------------------------------------------------------------------
const std::string& CParContainer::GetNameByIndex(const unsigned int index, bool all) const
{
    assert (index < pars.size());

    unsigned int excludedCount = 0, trueIndex = 0;
    bool found = false;

    if (all)
    {
        return pars[index].Name();
    }
    else
    {
        for (unsigned int i=0; i<pars.size(); ++i)
        {
            if (!pars[i].LatinHypercubeExcluded()) ++excludedCount;

            if (excludedCount == (index+1) && !found)
            {
                trueIndex = i;
                found = true;
            }
        }
    }

    return pars[trueIndex].Name();
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const CParContainer& p)
{
    for (unsigned int i=0; i<p.size(); ++i)
    {
        os << p(i).Name() << "\t" << p[i] << std::endl;
    }

    return os;
}
//---------------------------------------------------------------------------
void CParContainer::writeXml ( tinyxml2::XMLNode * node ) const
{
    for (unsigned int i=0; i<pars.size(); ++i)
    {
        pars[i].writeXml( node );
    }
}
//---------------------------------------------------------------------------
void CParContainer::writeXml ( tinyxml2::XMLNode * node,
                                char const* name,
                                char const* indexer ) const
{
    tinyxml2::XMLNode * group = node->GetDocument()->NewElement( "ParameterGroup" );
    group->ToElement()->SetAttribute( "name", name );
    group->ToElement()->SetAttribute( "indexer", indexer );
    group->ToElement()->SetAttribute( "count", (unsigned int)pars.size() );

    for (unsigned int i=0; i<pars.size(); ++i)
    {
        tinyxml2::XMLNode * groupItem = group->GetDocument()->NewElement( "ParameterContainer" );
        groupItem->ToElement()->SetAttribute( "count", "1" );
        pars[i].writeXml( groupItem );
        group->InsertEndChild( groupItem );
    }

    node->InsertEndChild( group );
}
//---------------------------------------------------------------------------
tinyxml2::XMLNode * CParContainer::readXml ( tinyxml2::XMLNode * parameter, bool readFirst )
{
    for (unsigned int i=0; i<pars.size(); ++i)
    {
        pars[i].readXml( parameter );

        parameter = parameter->NextSibling();
    }

    return parameter;
}
//---------------------------------------------------------------------------
void CParContainer::readXml ( tinyxml2::XMLNode * group,
                                char const* name,
                                char const* indexer )
{
    tinyxml2::XMLNode * groupItem = group->FirstChildElement("ParameterContainer");

    for (unsigned int i=0; i<pars.size(); ++i)
    {
        tinyxml2::XMLNode * parameter = groupItem->FirstChildElement( "Parameter" );

        pars[i].readXml( parameter );

        groupItem = groupItem->NextSiblingElement("ParameterContainer");
    }
}
//---------------------------------------------------------------------------
