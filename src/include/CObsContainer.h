//---------------------------------------------------------------------------
#ifndef CObsContainerH
#define CObsContainerH
//---------------------------------------------------------------------------

#include <map>
#include <set>

#include "CObsSet.h"
#include "CStructure.h"

class CObsContainer
{
    friend std::ostream& operator<<(std::ostream&, const CObsContainer&);

public:
    typedef std::map<const std::string, CObsSet>::const_iterator const_iterator;
    typedef std::map<const std::string, CObsSet>::iterator iterator;

    iterator begin() { return obs.begin(); }
    const_iterator begin() const { return obs.begin(); }
    iterator end() { return obs.end(); }
    const_iterator end() const { return obs.end(); }

    CObsSet& operator[](const std::string);
    CObsSet& operator[](int);
    unsigned int size(void) const { return obs.size(); }
    std::string& Filename() { return filename; }

    void Filename(std::string f) { filename = f; }
    void UpdateReachList(CObsSet& o);
    bool InReachList(std::string reach);
    std::vector<std::string> GetKeys(void);

    void GenerateChartColours(void);
    void Structure(CStructure* s) { structure = s; }
    void clear(void) { obs.clear(); }

private:
    std::map<const std::string, CObsSet> obs;
    CStructure* structure;
    std::string filename;
    std::set<std::string> reachList;
};

#endif
