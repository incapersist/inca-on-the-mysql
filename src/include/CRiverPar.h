//---------------------------------------------------------------------------
#ifndef CRiverParH
#define CRiverParH
//---------------------------------------------------------------------------
#include <fstream>

#include "CParContainer.h"
#include "tinyxml2.h"

class CRiverPar
{
    friend bool operator<=(const CRiverPar&, const CRiverPar&);
    friend std::ostream& operator<<(std::ostream&, const CRiverPar&);
	friend std::istream& operator>>(std::istream& in, CRiverPar& par);
	friend CRiverPar operator*(const CRiverPar&, const double&);
	friend CRiverPar operator-(const CRiverPar&, const CRiverPar&);

public:
    CRiverPar();
 //   CRiverPar(CRiverPar&);
 //   CRiverPar(const CRiverPar&);
 //   CRiverPar& operator=(const CRiverPar&);

    unsigned int IncludedCount(bool);
    double GetParByIndex(const unsigned int, bool);
    const std::string& GetNameByIndex(const unsigned int, bool);
    std::istream& serialize(std::istream& in);

    void writeXml ( tinyxml2::XMLNode * node ) const;
    void readXml ( tinyxml2::XMLNode * node );

    CParContainer Initial;
    CParContainer Pars;
};


#endif
 