//---------------------------------------------------------------------------
#ifndef CResultsGroupH
#define CResultsGroupH
//---------------------------------------------------------------------------
#include <vector>
#include <map>
#include <cstddef>

#include "CParSet.h"
#include "CResultsSet.h"
#include "CObsSet.h"
#include "CErrorMatrix.h"
#include "CStructure.h"
#include "CObsContainer.h"


//typedef std::map<std::string, CResultsSet> resultsType;
typedef std::map<std::string, CResultsSet*> resultsType;

typedef resultsType::value_type resultsValueType;

typedef resultsType::iterator resultsIter;
typedef resultsType::const_iterator resultsConstIter;

class CResultsGroup
{
public:
    CResultsGroup(int _reach, int _index,
                    CParSet* ParSet, CStructure* strctr,
                    unsigned int _land = 6, bool _full = true);
    CResultsGroup(CResultsGroup&);
    CResultsGroup(const CResultsGroup&);
    CResultsGroup& operator=(const CResultsGroup&);
    ~CResultsGroup();

    bool LoadFromFile(const char*);
    bool SaveToFile(const char*, CParSet*, int, int, unsigned int, bool writeDaily = false, bool writeRiverBalance = false, bool writeCellBalance = false, bool writeSoilTemperature = false, bool writeSnowDepth = false, bool writeSnowWater = false);
    void CalcRiverBal(CParSet&);
    void CalcCellBal(CParSet&);
    void CalcErrors(CParSet*, int, bool, CObsContainer& ObsSources);
    int GetReaches(void) { return Reaches; }
    void Init(double val);
    bool FullSet(void) { return fullSet; }
    bool Available(void) { return available; }
    void Available(bool a) { available = a; }
    bool WriteDates(void) { return writeDates; }
    void WriteDates(bool wd) { writeDates = wd; }

    void CalcStats(void);
    void ResetErrorMatrix(void);

    CErrorMatrix GetErrorMatrix(void);

    bool Saved;
//    CResultsSet **SubCatchRes;
    resultsType SubCatchRes;
private:
	unsigned int Count;
	int Reaches;
    unsigned int LandCount;
    bool fullSet, available, writeDates;
    CStructure* structure;
    std::ofstream outfile;
    std::string parfile, time;
    int header;
    std::vector<unsigned int> yearsList;

    void FindYears(const dateCl::Date&);
    bool NewYear(dateCl::Date& Today);
    void Copy(const CResultsGroup&);
    std::size_t countColumns(std::string& rec);
    std::ofstream& WriteStatsHeader(std::ofstream&, const int&);
	void OpenFile(const char* FileName,
		std::string title,
		std::string newext,
		std::string subType,
		std::string stream,
		std::string reach,
		int i = -1);
};
#endif
