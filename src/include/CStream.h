#ifndef CStreamH
#define CStreamH

#include "CReach.h"
#include "CRiverPar.h"

#include <vector>
#include <string>
#include <ostream>

class CStream
{
    friend std::ostream& operator<<(std::ostream&, const CStream&);
	friend std::istream& operator>>(std::istream& is, CStream& s);

public:
    CStream(std::string, unsigned short, std::string);
    CStream(std::string, unsigned short);
    CStream(std::string);
    CStream();

    // Do something
    void Setup(std::string, unsigned short, std::string);
    void Reset(void);
    bool NextReach(void);
    std::istream& serialize(std::istream&);
    std::string tokenize(std::istream&, std::string);
    bool HasReachWithInput(std::string);
    CReach GetReachWithInput(std::string);
    void writeXml ( tinyxml2::XMLNode * group ) const;
    void readXml ( tinyxml2::XMLNode * group );

    // Get something
    unsigned int size(void) { return reaches.size(); }
    std::string& ID(void) { return streamID; }
    unsigned short Order(void) { return streamOrder; }
    const std::string& Name(void) { return streamName; }
    CReach& GetCurrentReach(void);
    const std::string& GetReachID(unsigned int);
    const std::string& GetCurrentReachID(void);
    CRiverPar Pars(void);
    const CReach& GetReachByIndex(unsigned int);
    unsigned int GetReachIndex(std::string&);
//    double Par(unsigned int index) { return pars[index]; }
//    double Initial(unsigned int index) { return initial[index]; }
    double Par(unsigned int index) { return riverPars_.Pars[index]; }
    double Initial(unsigned int index) { return riverPars_.Initial[index]; }
    std::vector<std::string> GetAllReachIDs(void);

    // Set something
    void ID(std::string& s) { streamID = s; }
    void Name(std::string& n) { streamName = n; }
    void AddReach(CReach&);
    void Pars(const CRiverPar&);
 //   void Par(unsigned int index, double val) { pars[index] = val; }
 //   void Initial(unsigned int index, double val) { initial[index] = val; }
    void Par(unsigned int index, double val) { riverPars_.Pars[index] = val; }
    void Initial(unsigned int index, double val) { riverPars_.Initial[index] = val; }

private:
    std::string streamID;
    std::string streamName;
    unsigned short streamOrder;
    std::vector<CReach> reaches;
    unsigned int currentReachIndex;
//    std::vector<double> initial, pars;
    unsigned int reachCount, lakeCount;
    CRiverPar riverPars_;
};

#endif