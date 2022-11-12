//---------------------------------------------------------------------------
#ifndef CParSetH
#define CParSetH
//---------------------------------------------------------------------------
#include "CCellPar.h"
#include "CRiverPar.h"
#include "CReachPar.h"
#include "CSubPar.h"
#include "DATECL.h"
#include "tinyxml2.h"
#include "CPar.h"
#include "CStructure.h"

#include <vector>
#include <map>

typedef std::map<std::string, CReachPar> reachType;
typedef std::map<std::string, CSubPar> subType;
typedef std::map<std::string, CRiverPar> riverType;

typedef subType::iterator subIter;
typedef subType::const_iterator subConstIter;
typedef reachType::iterator reachIter;
typedef reachType::const_iterator reachConstIter;
typedef riverType::iterator riverIter;
typedef riverType::const_iterator riverConstIter;

typedef reachType::value_type reachValueType;
typedef subType::value_type subValueType;
typedef riverType::value_type riverValueType;

class CParSet
{
    friend bool operator<=(const CParSet&, const CParSet&);

public:
    CParSet();
    CParSet(std::string const& fname, CStructure** structure);
    CParSet(CParSet& ParSet);
    CParSet(const CParSet& ParSet);
    CParSet& operator=(const CParSet&);
    ~CParSet();
    bool LoadFromFile(std::string const& FileName, CStructure** structure);
    bool Import1v11(std::string const& FileName, CStructure** structure);
    bool ImportBranching1v0(std::string const& FileName, CStructure** structure);

    bool SaveToFile(std::string const& FileName, CStructure** structure);
    bool SaveToFile(CStructure** structure);
    unsigned int LandCount(void) { return landCount; }
    unsigned int LandCount(void) const { return landCount; }
	void LandCount(unsigned int l) { landCount = l; }
    void LatinHypercubeInit(unsigned int);
    void LatinHypercubeGenerate(void);
    void LatinHypercubeGetSample(unsigned int);
    void LatinHypercubeSetRange(CParSet&, CParSet&);
    void ExcludeZeroRangePars(void);
    unsigned int IncludedCount(bool);
    double GetParByIndex(const unsigned int, bool);
    const std::string GetNameByIndex(const unsigned int, bool);
    std::vector<std::string> GetLandUseNames(bool);
    std::vector<std::string> GetReachNames(bool);
    void Homogeneous(unsigned int, std::string&);
    void Heterogeneous(void);
    void AddToLog(std::string);
    std::vector<std::string> GetLog(void) { return changeLog; }

    bool writeXml (std::string const& filename, CStructure** structure) const;
    bool readXml (std::string const& filename, CStructure** structure);

    void Success(bool s) { successful = s; }
    bool Success(void) const { return successful; }

//private:
    std::vector<std::string> LongLand;
    std::vector<std::string> ShortLand;

    bool Changed, loaded, hetero;
    int TimeSteps, Reaches;
    dateCl::Date StartDate;
    std::string Name;
    std::string ParFileName;
    unsigned int numLatinHypercubeGroups;
    unsigned int homoLand;
    std::string homoReach;

    std::vector<CCellPar> CellPar;
    CRiverPar RiverPar;
    std::map<std::string, CReachPar> ReachPar;
    std::map<std::string, CSubPar> SubPar;

private:
    void CreateStringList(std::vector<std::string>& str, const char* buf);
    void Init(void);
    std::vector<std::string> changeLog;
    void writeXmlStartDate ( tinyxml2::XMLNode * node ) const;
    void readXmlStartDate ( tinyxml2::XMLNode * node );

    unsigned int landCount;
    bool successful;
};

CParSet operator*(const CParSet& p, const double& t);
CParSet operator-(const CParSet& p, const CParSet& n);

#endif

