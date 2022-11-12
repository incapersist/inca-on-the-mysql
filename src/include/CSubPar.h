//---------------------------------------------------------------------------
#ifndef CSubParH
#define CSubParH
//---------------------------------------------------------------------------
#include <string>

#include "CParContainer.h"
#include "tinyxml2.h"

class CSubPar
{
    friend bool operator>(const CSubPar&, const CSubPar&);
    friend std::ostream& operator<<(std::ostream&, const CSubPar&);

public:
    CSubPar();
    CSubPar(unsigned int landCount);
    CSubPar(CSubPar&);
    CSubPar(const CSubPar&);
    CSubPar& operator=(const CSubPar&);

    void writeXml ( tinyxml2::XMLNode * node, std::vector<std::string> landNames ) const;
    void readXml ( tinyxml2::XMLNode * node );

    void Init(unsigned int);
    unsigned int IncludedCount(bool);
    double GetParByIndex(const unsigned int, bool);
    const std::string& GetNameByIndex(const unsigned int, bool);

    CParContainer Par;
    CParContainer Land;
    CParContainer Dep;
};

CSubPar operator*(const CSubPar&, const double&);
CSubPar operator-(const CSubPar&, const CSubPar&);

#endif

