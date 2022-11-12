//---------------------------------------------------------------------------
#ifndef CCellParH
#define CCellParH
//---------------------------------------------------------------------------
#include "CParContainer.h"
#include "tinyxml2.h"

#include <string>

class CCellPar
{
    friend bool operator>(const CCellPar&, const CCellPar&);

public:
    CCellPar();
    CCellPar(CCellPar&);
    CCellPar(const CCellPar&);
    CCellPar& operator=(const CCellPar&);
    unsigned int IncludedCount(bool);
    double GetParByIndex(const unsigned int, bool);
    const std::string& GetNameByIndex(const unsigned int, bool);

    void writeXml ( tinyxml2::XMLNode * node ) const;
    void readXml ( tinyxml2::XMLNode * node );

    CParContainer Initial;
    CParContainer Par;
    CParContainer TC;
};

CCellPar operator*(const CCellPar&, const double&);
CCellPar operator-(const CCellPar&, const CCellPar&);

#endif
 