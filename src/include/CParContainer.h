//---------------------------------------------------------------------------
#ifndef CParContainerH
#define CParContainerH
//---------------------------------------------------------------------------

#include <vector>
//#include <cstdio>

#include "CPar.h"
#include "tinyxml2.h"

class CParContainer
{
    friend CParContainer operator-(const CParContainer&, const CParContainer&);
    friend bool operator<=(const CParContainer&, const CParContainer&);
    friend std::ostream& operator<<(std::ostream&, const CParContainer&);

public:
    typedef std::vector<CPar>::const_iterator const_iterator;
    typedef std::vector<CPar>::iterator iterator;

    iterator begin() { return pars.begin(); }
    const_iterator begin() const { return pars.begin(); }
    iterator end() { return pars.end(); }
    const_iterator end() const { return pars.end(); }

    CParContainer();
    CParContainer(int);
//    CParContainer(CParContainer&);
//    CParContainer(const CParContainer&);
//    CParContainer& operator=(const CParContainer&);
    double& operator[](unsigned int);
    const double& operator[](unsigned int) const;
    CPar& operator()(unsigned int);
    const CPar& operator()(unsigned int) const;

    void clear() { pars.clear(); }
    void writeXml ( tinyxml2::XMLNode * node ) const;
    void writeXml ( tinyxml2::XMLNode * node, char const* name, char const* ) const;
    tinyxml2::XMLNode * readXml ( tinyxml2::XMLNode * node, bool readFirst = true );
    void readXml ( tinyxml2::XMLNode * node, char const* name, char const* );

    void Add(CPar&);

//    void Add(char const*, double val = 0.0, char const* typeName = "double");
    void Add(char const*,
             double defaultValue = 0.0,
             double minValue = (std::numeric_limits<double>::min)(),
             double maxValue = (std::numeric_limits<double>::max)());

    void AddGrainPars(const unsigned int gs,
                        const std::vector<std::string>& gNames,
                        const char *pre,
                        const char *post,
                        double val = 0.0);
    void Init(int);
    void Init(void);
    unsigned int size(void) { return pars.size(); }
    const unsigned int size(void) const { return pars.size(); }
    void LatinHypercubeExcludeAll(bool);
    void LatinHypercubeExclude(bool, unsigned int);

    void LatinHypercubeInit(unsigned int);
    void LatinHypercubeGenerate(void);
    void LatinHypercubeGetSample(unsigned int);
    void LatinHypercubeSetRange(CParContainer&, CParContainer&);
    void ExcludeZeroRangePars(void);
    void SetMissingValue(double, unsigned int);
    void WriteLH(FILE* fp);
    unsigned int IncludedCount(bool);
    double GetParByIndex(const unsigned int, bool);
    const std::string& GetNameByIndex(const unsigned int, bool) const;

private:
    std::vector<CPar> pars;
};

CParContainer operator*(const CParContainer&, const double&);

#endif
