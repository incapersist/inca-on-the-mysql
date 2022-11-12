#include "CStream.h"

#include <sstream>
#include "Constant.h"

//---------------------------------------------------------------------------
CStream::CStream(std::string id, unsigned short order, std::string name)
{
    Setup(id, order, name);
    reaches.clear();
    Reset();
}
//---------------------------------------------------------------------------
CStream::CStream(std::string id, unsigned short order)
{
    Setup(id, order, "<none>");
    reaches.clear();
    Reset();
}
//---------------------------------------------------------------------------
CStream::CStream(std::string id)
{
    Setup(id, 0, "<none>");
    reaches.clear();
    Reset();
}
//---------------------------------------------------------------------------
CStream::CStream()
{
    Setup("1", 0, "<none>");
    reaches.clear();
    Reset();
}
//---------------------------------------------------------------------------
void CStream::Setup(std::string id, unsigned short order, std::string name)
{
    streamID = id;
    streamOrder = order;
    streamName = name;

    lakeCount = 0;
}
//---------------------------------------------------------------------------
void CStream::Reset(void)
{
    currentReachIndex = 0;
}
//---------------------------------------------------------------------------
bool CStream::NextReach(void)
{
    ++currentReachIndex;

    if (currentReachIndex == reaches.size()) return false;

    return true;
}
//---------------------------------------------------------------------------
bool CStream::HasReachWithInput(std::string id)
{
    for (unsigned int i=0; i<reaches.size(); ++i)
    {
        if (reaches[i].HasInput(id)) return true;
    }

    return false;
}
//---------------------------------------------------------------------------
CReach CStream::GetReachWithInput(std::string id)
{
    CReach reach;

    for (unsigned int i=0; i<reaches.size(); ++i)
    {
        if (reaches[i].HasInput(id)) return reaches[i];
    }

    return reach;
}
//---------------------------------------------------------------------------
unsigned int CStream::GetReachIndex(std::string& id)
{
    for (unsigned int i=0; i<reaches.size(); ++i)
    {
        if (reaches[i].ID() == id) return i;
    }

    return 0;
}
//---------------------------------------------------------------------------
CReach& CStream::GetCurrentReach(void)
{
    return reaches[currentReachIndex];
}
//---------------------------------------------------------------------------
void CStream::AddReach(CReach& reach)
{
    reaches.push_back(reach);
}
//---------------------------------------------------------------------------
const std::string& CStream::GetReachID(unsigned int index)
{
    return reaches[index].ID();
}
//---------------------------------------------------------------------------
const CReach& CStream::GetReachByIndex(unsigned int index)
{
    return reaches[index];
}
//---------------------------------------------------------------------------
const std::string& CStream::GetCurrentReachID(void)
{
    return reaches[currentReachIndex].ID();
}
//---------------------------------------------------------------------------
void CStream::Pars(const CRiverPar& rp)
{
    riverPars_ = rp;

/*    initial.clear();
    pars.clear();

    for (unsigned int i=0; i<rp.Initial.size(); ++i)
    {
        initial.push_back(rp.Initial[i]);
    }

    for (unsigned int i=0; i<rp.Pars.size(); ++i)
    {
        pars.push_back(rp.Pars[i]);
    }
    */
}
//---------------------------------------------------------------------------
CRiverPar CStream::Pars(void)
{
/*    CRiverPar rp;

    for (unsigned int i=0; i<initial.size(); ++i)
    {
        rp.Initial[i] = initial[i];
    }

    for (unsigned int i=0; i<pars.size(); ++i)
    {
        rp.Pars[i] = pars[i];
    }
*/
//    return rp;
    return riverPars_;
}
//---------------------------------------------------------------------------
std::vector<std::string> CStream::GetAllReachIDs(void)
{
    std::vector<std::string> list;

    for (unsigned int i=0; i<reaches.size(); ++i)
    {
        list.push_back(reaches[i].ID());
    }

    return list;
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const CStream& s)
{
    os << "======================" << std::endl;
    os << "STREAM ID: " << s.streamID << std::endl;
    os << "======================" << std::endl;
    os << "Name: " << s.streamName << std::endl;
    os << "Order: " << s.streamOrder << std::endl;
    os << "Initial values: ";

//    for (unsigned int i=0; i< s.initial.size(); ++i)
    for (unsigned int i=0; i< s.riverPars_.Initial.size(); ++i)
    {
//        os << s.initial[i] << " ";
        os << s.riverPars_.Initial[i] << " ";
    }
    os << std::endl;

    os << "Parameters: ";

//    for (unsigned int i=0; i< s.pars.size(); ++i)
    for (unsigned int i=0; i< s.riverPars_.Pars.size(); ++i)
    {
//        os << s.pars[i] << " ";
        os << s.riverPars_.Pars[i] << " ";
    }
    os << std::endl;

    os << "Reach count: " << s.reaches.size() << std::endl;
    os << "Lake count: " << s.lakeCount << std::endl << std::endl;

    for (unsigned int i=0; i<s.reaches.size(); ++i)
    {
        os << s.reaches[i] << std::endl;
    }

    return os;
}
//---------------------------------------------------------------------------
std::istream& CStream::serialize(std::istream& is)
{
    std::string input, temp;

    // Blank line
    getline(is, input);

    // Separator line
    getline(is, input);

    // Stream ID
    streamID = tokenize(is, "STREAM ID: ");

    // Separator line
    getline(is, input);

    // Stream name
    streamName = tokenize(is, "Name: ");

    // Stream order
    temp = tokenize(is, "Order: ");
    std::istringstream ss1(temp);
    ss1 >> streamOrder;

    // Initial values
    temp = tokenize(is, "Initial values: ");
    std::istringstream ss2(temp);
    double init;

    unsigned int index = 0;

    while (ss2 >> init)
    {
//        initial.push_back(init);
        riverPars_.Initial[index] = init;
        ++index;
    }

    // Parameters
    temp = tokenize(is, "Parameters: ");
    std::istringstream ss3(temp);
    double p;

    index = 0;

    while (ss3 >> p)
    {
//        pars.push_back(p);
        riverPars_.Pars[index] = p;
        ++index;
    }

    // Reach count
    temp = tokenize(is, "Reach count: ");
    std::istringstream ss4(temp);
    ss4 >> reachCount;

    // Lake count
    temp = tokenize(is, "Lake count: ");
    std::istringstream ss5(temp);
    ss5 >> lakeCount;

    reaches.clear();

    for (unsigned int i=0; i<reachCount; ++i)
    {
        CReach r;
        is >> r;
        AddReach(r);
    }

    return is;
}
//---------------------------------------------------------------------------
std::string CStream::tokenize(std::istream& is, std::string str)
{
    std::string input;
    getline(is, input);

    if (input.find(str.size()) == input.size())
    {
        throw;
    }

    return input.substr(str.size());
}
//---------------------------------------------------------------------------
void CStream::writeXml ( tinyxml2::XMLNode * group ) const
{
    unsigned int riverParCount = NUM_RIVER_INITIAL + NUM_RIVER_PARS;

    tinyxml2::XMLNode * groupItem = group->GetDocument()->NewElement( "ParameterContainer" );
    groupItem->ToElement()->SetAttribute( "name", streamName.c_str() );
    groupItem->ToElement()->SetAttribute( "order", streamOrder );
    groupItem->ToElement()->SetAttribute( "reaches", (unsigned int)reaches.size() );
    groupItem->ToElement()->SetAttribute( "count", riverParCount );

    riverPars_.writeXml( groupItem );

    group->InsertEndChild( groupItem );
}
//---------------------------------------------------------------------------
void CStream::readXml ( tinyxml2::XMLNode * group )
{
    streamID = group->ToElement()->Attribute( "name" );

    int order;
    group->ToElement()->QueryIntAttribute( "order", &order );
    streamOrder = order;

    riverPars_.readXml( group );
}
//---------------------------------------------------------------------------

std::istream& operator>>(std::istream& is, CStream& s)
{
	s.serialize(is);
	return is;
}

