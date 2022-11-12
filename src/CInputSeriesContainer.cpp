#include "CInputSeriesContainer.h"

#include "utils.h"

//---------------------------------------------------------------------------
CInputSeriesContainer::~CInputSeriesContainer()
{
//    for (unsigned int i=0; i<inputSeriesList.size(); ++i)
    for (inputIter it = inputSeriesList.begin(); it != inputSeriesList.end(); ++it)
    {
        delete it->second;
    }

    inputSeriesList.clear();
}
//---------------------------------------------------------------------------
void CInputSeriesContainer::Add(std::string reach, CInputSeries* i)
{
    // Check that input series timesteps == parameter file timesteps

//    inputSeriesList.push_back(i);
    inputSeriesList.insert(inputValueType(reach, i));
}

//---------------------------------------------------------------------------
CInputSeries* CInputSeriesContainer::operator[](std::string index)
{
    return inputSeriesList[index];
}

//---------------------------------------------------------------------------
CInputSeries* CInputSeriesContainer::operator[](unsigned int index)
{
    unsigned int count = 0;
    for (inputIter it = inputSeriesList.begin(); it != inputSeriesList.end(); ++it)
    {
        if (count == index) return it->second;
    }
//    return inputSeriesList[index];

	return 0;
}
//---------------------------------------------------------------------------
bool CInputSeriesContainer::LoadFromFile(std::string fname, CParSet* pars)
{
    std::ifstream in(fname.c_str());

    std::vector<std::string> fileStrings;
    std::string row;

    while (getline(in, row))
    {
        fileStrings.push_back(row);
    }

    unsigned int TimeSteps, inputCount;

    SetValue(TimeSteps, fileStrings[0]);
    SetValue(inputCount, fileStrings[1]);

    for (unsigned int i=0; i<inputCount; ++i)
    {
        unsigned int offset = i * (TimeSteps + 1) + 2;

        std::string reach = utils::trim(fileStrings[offset], " \t\f\v\n\r");
  	//	for (size_t j=0; j<reach.length(); ++j) reach[j] = toupper(reach[j]);

        CInputSeries* input = new CInputSeries(pars->TimeSteps, 5, pars->Reaches);

        input->LoadFromStrings(fileStrings, offset+1);

        if (input->Missing())
        {
            reachIter it = pars->ReachPar.find(reach);
            input->CalcSolar(it->second.Par[9], it->second.Par[10], pars->StartDate);
        }

        inputSeriesList.insert(inputValueType(reach, input));

    }

    in.close();

    return true;
}
//---------------------------------------------------------------------------
void CInputSeriesContainer::SetValue(unsigned int& val, std::string& row)
{
    std::istringstream is(row);
    is >> val;
}
//---------------------------------------------------------------------------

