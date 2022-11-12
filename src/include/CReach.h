#ifndef CReachH
#define CReachH

#include <string>
#include <vector>
#include <ostream>
#include <istream>

class CReach
{
    friend std::ostream& operator<<(std::ostream&, const CReach&);
    friend std::istream& operator>>(std::istream&, CReach&);

public:
    CReach(std::string, std::string);
    CReach(std::string);
    CReach();

    void Setup(std::string, std::string);
    std::istream& serialize(std::istream&);
    std::string tokenize(std::istream&, std::string);

    void Coords(float, float, float, float);
    void AddInput(std::string);
    bool HasInput(std::string);

    const std::string& ID(void) { return reachID; }
    const std::string& Name(void) { return reachName; }
    const std::vector<std::string>& Inputs(void) { return inputReaches; }

    float TopLat(void) { return topLatitude; }
    float TopLong(void) { return topLongitude; }
    float BottomLat(void) { return bottomLatitude; }
    float BottomLong(void) { return bottomLongitude; }

private:
    std::string reachID;
    std::string reachName;
    float topLatitude, topLongitude, bottomLatitude, bottomLongitude;
    std::vector<std::string> inputReaches;
};


#endif