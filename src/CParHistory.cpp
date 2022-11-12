#include "CParHistory.h"

#include <sstream>

//---------------------------------------------------------------------------
CParHistory::CParHistory()
{
    version = 1;
}
//---------------------------------------------------------------------------
void CParHistory::Reset(void)
{
    if (direction == Ascending)
    {
        currentRecord = history.begin();
    }
    else
    {
        currentRecord = history.end();
        --currentRecord;
    }
}
//---------------------------------------------------------------------------
void CParHistory::Add(CParSet par, std::string label)
{
    dateCl::Date date;
    date.Set();

    std::ostringstream name;
    name << "Version " << version;
    par.Name = name.str();

    std::pair<CParSet, std::string> pi(par, label);
    history[date] = pi;

    ++version;

    currentRecord = history.find(date);
}
//---------------------------------------------------------------------------
bool CParHistory::Next(void)
{
    if (direction == Ascending)
    {
        ++currentRecord;
        if (currentRecord == history.end()) return false;
    }
    else
    {
        if (currentRecord == history.begin()) return false;
        --currentRecord;
    }

    return true;
}
//---------------------------------------------------------------------------
dateCl::Date CParHistory::CurrentDate(void)
{
    return currentRecord->first;
}
//---------------------------------------------------------------------------
bool CParHistory::CurrentSuccess(void)
{
    return currentRecord->second.first.Success();
}
//---------------------------------------------------------------------------
std::string CParHistory::CurrentLabel(void)
{
    return currentRecord->second.second;
}
//---------------------------------------------------------------------------
std::string CParHistory::CurrentName(void)
{
    return currentRecord->second.first.Name;
}
//---------------------------------------------------------------------------
CParSet CParHistory::FindByName(std::string name)
{
    CParHistory::historyIter it;

    for (it=history.begin(); it!=history.end(); ++it)
    {
        std::string n = it->second.first.Name;

        if (n == name)
        {
            CParSet p = it->second.first;
            return p;
        }
    }

    if (it == history.end()) throw;

    return history.end()->second.first;
}
//---------------------------------------------------------------------------

