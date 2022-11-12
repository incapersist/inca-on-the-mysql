#include "CReach.h"

#include <sstream>

//---------------------------------------------------------------------------
CReach::CReach(std::string id, std::string name)
{
    Setup(id, name);
}
//---------------------------------------------------------------------------
CReach::CReach(std::string id)
{
    Setup(id, "<none>");
}
//---------------------------------------------------------------------------
CReach::CReach()
{
    Setup("1", "<none>");
}
//---------------------------------------------------------------------------
void CReach::Setup(std::string id, std::string name)
{
    reachID = id;
    reachName = name;
}
//---------------------------------------------------------------------------
void CReach::Coords(float tLat, float tLong, float bLat, float bLong)
{
    topLatitude = tLat;
    topLongitude = tLong;
    bottomLatitude = bLat;
    bottomLongitude = bLong;
}
//---------------------------------------------------------------------------
void CReach::AddInput(std::string id)
{
    inputReaches.push_back(id);
}
//---------------------------------------------------------------------------
bool CReach::HasInput(std::string id)
{
    for (unsigned int i=0; i<inputReaches.size(); ++i)
    {
        if (inputReaches[i] == id) return true;
    }

    return false;
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const CReach& r)
{
    os << "Reach ID: " << r.reachID << std::endl;
    os << "-------------------" << std::endl;
    os << "Name: " << r.reachName << std::endl;
    os << "Upstream boundary latitude: " << r.topLatitude << std::endl;
    os << "Upstream boundary longitude: " << r.topLongitude << std::endl;
    os << "Downstream boundary latitude: " << r.bottomLatitude << std::endl;
    os << "Downstream boundary longitude: " << r.bottomLongitude << std::endl;
    os << "Inputs: ";

    for (unsigned int i=0; i<r.inputReaches.size(); ++i)
    {
        os << r.inputReaches[i] << " ";
    }
    os << std::endl;

    return os;
}
//---------------------------------------------------------------------------
std::istream& CReach::serialize(std::istream& is)
{
    std::string input, temp;

    // Blank line
    getline(is, input);

    // Reach ID
    reachID = tokenize(is, "Reach ID: ");

    // Separator line
    getline(is, input);

    // Reach name
    reachName = tokenize(is, "Name: ");

    // Upstream boundary latitude
    temp = tokenize(is, "Upstream boundary latitude: ");
    std::istringstream ss1(temp);
    ss1 >> topLatitude;

    // Upstream boundary longitude
    temp = tokenize(is, "Upstream boundary longitude: ");
    std::istringstream ss2(temp);
    ss2 >> topLongitude;

    // Downstream boundary latitude
    temp = tokenize(is, "Downstream boundary latitude: ");
    std::istringstream ss3(temp);
    ss3 >> bottomLatitude;

    // Downstream boundary longitude
    temp = tokenize(is, "Downstream boundary longitude: ");
    std::istringstream ss4(temp);
    ss4 >> bottomLongitude;

    // Inputs
    temp = tokenize(is, "Inputs: ");
    std::istringstream ss5(temp);
    std::string id;

    while (ss5 >> id)
    {
        inputReaches.push_back(id);
    }

    return is;
}
//---------------------------------------------------------------------------
std::string CReach::tokenize(std::istream& is, std::string str)
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
std::istream& operator>>(std::istream& is, CReach& r)
{
	r.serialize(is);
	return is;
}