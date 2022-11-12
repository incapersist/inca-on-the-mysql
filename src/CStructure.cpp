#include "CStructure.h"

#include <sstream>

//---------------------------------------------------------------------------
CStructure::CStructure()
{
    changed = false;
    generated = false;
}
//---------------------------------------------------------------------------
void CStructure::Reset(void)
{
    if (direction == MainStemLast)
    {
        currentStream = streamOrder.begin();
    }
    else
    {
        currentStream = streamOrder.end();
        --currentStream;
    }

    for (CStructure::streamIter it=streams.begin(); it!=streams.end(); ++it)
    {
        it->second.Reset();
    }
}
//---------------------------------------------------------------------------
bool CStructure::NextStream(void)
{
    if (direction == MainStemLast)
    {
        ++currentStream;
        if (currentStream == streamOrder.end()) return false;
    }
    else
    {
        if (currentStream == streamOrder.begin()) return false;
        --currentStream;
    }

    return true;
}

//---------------------------------------------------------------------------
CStream& CStructure::GetCurrentStream(void)
{
    streamIter it = streams.find(currentStream->second);

    if (it == streams.end()) throw;

    return it->second;
}
//---------------------------------------------------------------------------
CStream& CStructure::GetStreamByID(const std::string& id)
{
    streamIter it = streams.find(id);

    if (it == streams.end()) throw;

    return it->second;
}
//---------------------------------------------------------------------------
const CReach& CStructure::FindReachByID(const std::string& id)
{
    for (CStructure::streamIter it=streams.begin(); it!=streams.end(); ++it)
    {
        it->second.Reset();

        do
        {
            if (it->second.GetCurrentReachID() == id)
            {
                return it->second.GetCurrentReach();
            }
        }
        while (it->second.NextReach());
    }

	throw;
}
//---------------------------------------------------------------------------
bool CStructure::FindReachWithInput(std::string id)
{
    for (CStructure::streamIter it=streams.begin(); it!=streams.end(); ++it)
    {
        if (it->second.HasReachWithInput(id))
        {
            return true;
        }
    }

    return false;
}
//---------------------------------------------------------------------------
CReach CStructure::GetReachWithInput(std::string id)
{
    for (CStructure::streamIter it=streams.begin(); it!=streams.end(); ++it)
    {
        if (it->second.HasReachWithInput(id))
        {
            return it->second.GetReachWithInput(id);
        }
    }

	throw;
}
//---------------------------------------------------------------------------
std::vector<std::string> CStructure::GetAllReachIDs(void)
{
    std::vector<std::string> all;

    for (CStructure::streamIter it=streams.begin(); it!=streams.end(); ++it)
    {
        std::vector<std::string> list = it->second.GetAllReachIDs();

        for (unsigned int i=0; i<list.size(); ++i)
        {
            all.push_back(list[i]);
        }
    }

    return all;
}
//---------------------------------------------------------------------------
const CStream& CStructure::FindStreamByReachID(const std::string& id)
{
    for (CStructure::streamIter it=streams.begin(); it!=streams.end(); ++it)
    {
        it->second.Reset();

        do
        {
            if (it->second.GetCurrentReachID() == id)
            {
                return it->second;
            }
        }
        while (it->second.NextReach());
    }

	throw;
}
//---------------------------------------------------------------------------
void CStructure::AddStream(CStream& str)
{
    streams[str.ID()] = str;
    streamOrder.insert(streamOrderType::value_type(str.Order(), str.ID()));
}
//---------------------------------------------------------------------------
void CStructure::clear(void)
{
    streams.clear();
    streamOrder.clear();
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const CStructure& s)
{
    os << "System name: " << s.name << std::endl;
    os << "Stream count: " << s.streams.size() << std::endl << std::endl;

    for(CStructure::streamOrderConstIter it=s.streamOrder.begin(); it!=s.streamOrder.end(); ++it)
    {
        CStructure::streamConstIter it2 = s.streams.find(it->second);
        os << it2->second;
    }

    return os;
}
//---------------------------------------------------------------------------
std::istream& CStructure::serialize(std::istream& is)
{
    std::string input, temp;

    // System name
    name = tokenize(is, "System name: ");

    // Stream count
    temp = tokenize(is, "Stream count: ");
    std::istringstream ss(temp);
    ss >> streamCount;

    streams.clear();

    for (unsigned int i=0; i<streamCount; ++i)
    {
        CStream s;
        is >> s;
        AddStream(s);
    }

    return is;
}
//---------------------------------------------------------------------------
std::string CStructure::tokenize(std::istream& is, std::string str)
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
//bool CStructure::CheckIntegrity(CParSet* pars)
//{
//    return true;
//}
//---------------------------------------------------------------------------
void CStructure::writeXml ( tinyxml2::XMLNode * node ) const
{
    for (CStructure::streamConstIter it=streams.begin(); it!=streams.end(); ++it)
    {
        it->second.writeXml( node );
    }
}
//---------------------------------------------------------------------------
void CStructure::readXml( tinyxml2::XMLNode * group )
{
    int streamCount;

	group->ToElement()->QueryIntAttribute( "count", &streamCount );
    tinyxml2::XMLNode * groupItem = group->FirstChildElement("ParameterContainer");

    for (CStructure::streamIter it=streams.begin(); it!=streams.end(); ++it)
    {
        it->second.readXml( groupItem );
        groupItem = group->NextSiblingElement("ParameterContainer");
    }
}
//---------------------------------------------------------------------------

std::istream& operator>>(std::istream& is, CStructure& s)
{
	s.serialize(is);
	return is;
}
