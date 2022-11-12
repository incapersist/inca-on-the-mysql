#ifndef CParHistoryH
#define CParHistoryH

#include "CParSet.h"
#include "DATECL.h"

class CParHistory
{

public:
    enum directions {Ascending, Descending};

    CParHistory();

    // Do something
    void Reset(void);
    bool Next(void);

    // Set something
    void Add(CParSet, std::string);
    void SetDirection(unsigned int d) { direction = d; }

    // Get something
    unsigned int size(void) { return history.size(); }
    dateCl::Date CurrentDate(void);
    bool CurrentSuccess(void);
    std::string CurrentLabel(void);
    std::string CurrentName(void);
    CParSet FindByName(std::string);

private:
    typedef std::map<dateCl::Date, std::pair<CParSet, std::string> > historyType;
    typedef historyType::iterator historyIter;
    typedef historyType::const_iterator historyConstIter;

    historyType history;
    historyConstIter currentRecord;

    unsigned int direction, version;
};

#endif
