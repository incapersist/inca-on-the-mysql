#ifndef CInputSeriesContainerH
#define CInputSeriesContainerH

#include <map>
#include <string>

#include "CInputSeries.h"
#include "CParSet.h"

typedef std::map<std::string, CInputSeries*> inputType;
typedef inputType::iterator inputIter;
typedef inputType::const_iterator inputConstIter;
typedef inputType::value_type inputValueType;

class CInputSeriesContainer
{
public:
    CInputSeries* operator[](std::string index);
    CInputSeries* operator[](unsigned int index);

    ~CInputSeriesContainer();

    bool LoadFromFile(std::string fname, CParSet* pars);
    unsigned int size(void) { return inputSeriesList.size(); }
    void clear(void) { inputSeriesList.clear(); }

    void Add(std::string reach, CInputSeries*);

private:
//    std::vector<CInputSeries*> inputSeriesList;
    inputType inputSeriesList;
    void SetValue(unsigned int& val, std::string& row);

};

#endif