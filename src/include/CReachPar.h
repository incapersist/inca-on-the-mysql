//---------------------------------------------------------------------------
#ifndef CReachParH
#define CReachParH
//---------------------------------------------------------------------------

#include "CParContainer.h"
#include "tinyxml2.h"

#include <string>
#include <iostream>

class CReachPar
{
    friend bool operator>(const CReachPar&, const CReachPar&);
	friend std::istream& operator>>(std::istream& in, CReachPar& par);
	friend CReachPar operator*(const CReachPar&, const double&);
	friend CReachPar operator-(const CReachPar&, const CReachPar&);

public:
    CReachPar();
    CReachPar(CReachPar& rhs);
    CReachPar(const CReachPar& rhs);
    CReachPar& operator=(const CReachPar&);

    std::string Name(void) const { return name; }
    void Name(std::string n) { name = n; }
    std::istream& serialize(std::istream& in);
    std::istream& ImportBranching(std::istream& in);
    void Import(std::istream& in);
    unsigned int IncludedCount(bool);
    double GetParByIndex(const unsigned int, bool);
    const std::string& GetNameByIndex(const unsigned int, bool);

    void writeXml ( tinyxml2::XMLNode * node ) const;
    tinyxml2::XMLNode * readXml ( tinyxml2::XMLNode * node );

    CParContainer Par;
    bool Effluent;

//private:
    std::string name;
};

#endif
 