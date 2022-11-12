//---------------------------------------------------------------------------
#ifndef CParH
#define CParH
//---------------------------------------------------------------------------

#include <string>
#include <vector>
#include <stdio.h>
#include <ostream>
#include <limits>
#include "tinyxml2.h"

class CPar// : public Parameter
{
    friend CPar operator*(const CPar&, const double&);
    friend CPar operator-(const CPar&, const CPar&);
    friend bool operator>(const CPar&, const CPar&);
    friend bool operator<=(const CPar&, const CPar&);
    friend std::ostream& operator<<(std::ostream&, const CPar&);

public:
    CPar(std::string str,
         double defaultValue = 0.0,
         double minValue = (std::numeric_limits<double>::min)(),
         double maxValue = (std::numeric_limits<double>::max)());//,
      //   char const* typeName = "double");

//    CPar(CPar&);
//    CPar(const CPar&);
//    CPar& operator=(const CPar&);

//    double& Value2(void) { return x_; }
//    const double& Value2(void) const { return x_; }
//    void Value2(const double& val) { x_ = val; }
//    void Name(std::string& n) { label_ = n; }
    double& Value2(void) { return value_; }
  //  double Value2(void) const { return value_; }
    const double& Value2(void) const { return value_; }
 //   void Value2(const double& val) { value = val; }
    void Value2(double val) { value_ = val; }
     double DefaultValue() { return defaultValue_; }
     double MinimumValue() { return minimumValue_; }
     double MaximumValue() { return maximumValue_; }
    void Name(std::string const& n) { name = n; }
//    void Name(char*);
//    const std::string& Name(void) const { return label_; }
    const std::string& Name(void) const { return name; }

    bool LatinHypercubeExcluded(void) { return excluded; }
    const bool LatinHypercubeExcluded(void) const { return excluded; }
    void LatinHypercubeExcluded(bool val) { excluded = val; }

    double MissingValue(void) { return missingValue; }
    const double MissingValue(void) const { return missingValue; }
    void MissingValue(double val) { missingValue = val; }

    bool LatinHypercubeInProgress(void) { return inLH; }
    const bool LatinHypercubeInProgress(void) const { return inLH; }

    void LatinHypercubeInit(unsigned int);
    void LatinHypercubeGenerate(void);
    void LatinHypercubeGetSample(unsigned int);
    void LatinHypercubeSetRange(CPar&, CPar&);
    void ExcludeZeroRange(void);
    void WriteLH(FILE* fp);
    void writeXml ( tinyxml2::XMLNode * node ) const;
    void readXml ( tinyxml2::XMLNode * node );

private:
    std::string name, units;
//    std::string units;
    std::vector<int> lh;
    double value_, missingValue;
    double LHmin, LHmax, LHrange;
    bool excluded, inLH;
    double smallestAllowedRange;
    double defaultValue_, minimumValue_, maximumValue_;
 //   std::string typeName_;

//    void copy(const CPar&);
    const bool hasMissingValue(void) const;
    float getXmlValue( tinyxml2::XMLNode * node, const char* attribute );
};

#endif

