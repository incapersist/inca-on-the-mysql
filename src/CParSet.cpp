//---------------------------------------------------------------------------
#include <stdio.h>
#include <math.h>

#include "Constant.h"
#include "CParSet.h"
#include "utils.h"
#include "version.h"

#include <iostream>
#include <fstream>
#include <sstream>

//---------------------------------------------------------------------------
CParSet::CParSet()
{
    Changed = false;
    loaded = false;
    TimeSteps = 0;
    Reaches = 0;
    Name = "";
    ParFileName = "<none>";

    landCount = 6;

    Init();
    Heterogeneous();
}

//---------------------------------------------------------------------------
CParSet::CParSet(std::string const& fname, CStructure** structure)
{
    Changed = false;
    loaded = false;
    TimeSteps = 0;
    Reaches = 0;
    Name = "";
    ParFileName = std::string(fname);

    landCount = 6;

    LoadFromFile(fname, structure);
    Heterogeneous();
}

//---------------------------------------------------------------------------
CParSet::CParSet(CParSet& rhs)
{
    LongLand.clear();
    ShortLand.clear();

    for (unsigned int i=0; i<rhs.LongLand.size(); ++i)
    {
        LongLand.push_back(rhs.LongLand[i]);
    }

    for (unsigned int i=0; i<rhs.ShortLand.size(); ++i)
    {
        ShortLand.push_back(rhs.ShortLand[i]);
    }

    Changed = rhs.Changed;
    TimeSteps = rhs.TimeSteps;
    Reaches = rhs.Reaches;
    Name = rhs.Name;
    ParFileName = rhs.ParFileName;
    StartDate = rhs.StartDate;
    landCount = rhs.landCount;

    CellPar.clear();

    for (unsigned int l=0; l<landCount; l++)
    {
        CCellPar par(rhs.CellPar[l]);
        CellPar.push_back(par);
    }

    ReachPar.clear();
    SubPar.clear();

/*    for (int i=0; i<Reaches; i++)
    {
        CReachPar reach(rhs.ReachPar[i]);
        ReachPar.push_back(reach);

        CSubPar sub(rhs.SubPar[i]);
        SubPar.push_back(sub);
    }
*/

    ReachPar = rhs.ReachPar;
    SubPar = rhs.SubPar;

    RiverPar = rhs.RiverPar;

    loaded = true;
    hetero = rhs.hetero;
    homoLand = rhs.homoLand;
    homoReach = rhs.homoReach;
    numLatinHypercubeGroups = rhs.numLatinHypercubeGroups;
    successful = rhs.successful;

    changeLog.clear();
    for (unsigned int i=0; i<rhs.changeLog.size(); ++i)
    {
        changeLog.push_back(rhs.changeLog[i]);
    }
}

//---------------------------------------------------------------------------
CParSet::CParSet(const CParSet& rhs)
{
    LongLand.clear();
    ShortLand.clear();

    for (unsigned int i=0; i<rhs.LongLand.size(); ++i)
    {
        LongLand.push_back(rhs.LongLand[i]);
    }

    for (unsigned int i=0; i<rhs.ShortLand.size(); ++i)
    {
        ShortLand.push_back(rhs.ShortLand[i]);
    }

    Changed = rhs.Changed;
    TimeSteps = rhs.TimeSteps;
    Reaches = rhs.Reaches;
    Name = rhs.Name;
    ParFileName = rhs.ParFileName;
    StartDate = rhs.StartDate;
    landCount = rhs.landCount;

    CellPar.clear();

    for (unsigned int l=0; l<landCount; l++)
    {
        CCellPar par(rhs.CellPar[l]);
        CellPar.push_back(par);
    }

    ReachPar.clear();
    SubPar.clear();

    ReachPar = rhs.ReachPar;
    SubPar = rhs.SubPar;

/*    for (int i=0; i<Reaches; i++)
    {
        CReachPar reach(rhs.ReachPar[i]);
        ReachPar.push_back(reach);

        CSubPar sub(rhs.SubPar[i]);
        SubPar.push_back(sub);
    }
*/
    RiverPar = rhs.RiverPar;

    loaded = true;
    hetero = rhs.hetero;
    homoLand = rhs.homoLand;
    homoReach = rhs.homoReach;
    numLatinHypercubeGroups = rhs.numLatinHypercubeGroups;
    successful = rhs.successful;

    changeLog.clear();
    for (unsigned int i=0; i<rhs.changeLog.size(); ++i)
    {
        changeLog.push_back(rhs.changeLog[i]);
    }
}
//---------------------------------------------------------------------------
CParSet& CParSet::operator=(const CParSet& rhs)
{
    if (&rhs != this)
    {
        LongLand.clear();
        ShortLand.clear();

        for (unsigned int i=0; i<rhs.LongLand.size(); ++i)
        {
            LongLand.push_back(rhs.LongLand[i]);
        }

        for (unsigned int i=0; i<rhs.ShortLand.size(); ++i)
        {
            ShortLand.push_back(rhs.ShortLand[i]);
        }

        Changed = rhs.Changed;
        TimeSteps = rhs.TimeSteps;
        Reaches = rhs.Reaches;
        Name = rhs.Name;
        ParFileName = rhs.ParFileName;
        StartDate = rhs.StartDate;
        landCount = rhs.landCount;

        CellPar.clear();

        for (unsigned int l=0; l<landCount; l++)
        {
            CCellPar par(rhs.CellPar[l]);
            CellPar.push_back(par);
        }

        ReachPar.clear();
        SubPar.clear();

    ReachPar = rhs.ReachPar;
    SubPar = rhs.SubPar;

/*        for (int i=0; i<Reaches; i++)
        {
            CReachPar reach(rhs.ReachPar[i]);
            ReachPar.push_back(reach);

            CSubPar sub(rhs.SubPar[i]);
            SubPar.push_back(sub);
        }
*/
        RiverPar = rhs.RiverPar;

        loaded = true;
        hetero = rhs.hetero;
        homoLand = rhs.homoLand;
        homoReach = rhs.homoReach;
        numLatinHypercubeGroups = rhs.numLatinHypercubeGroups;
        successful = rhs.successful;

        changeLog.clear();
        for (unsigned int i=0; i<rhs.changeLog.size(); ++i)
        {
            changeLog.push_back(rhs.changeLog[i]);
        }
    }

    return *this;
}

//---------------------------------------------------------------------------
CParSet operator*(const CParSet& p, const double& t)
{
    CParSet par(p);

    for (unsigned int i=0; i<p.LandCount(); ++i)
    {
        par.CellPar[i] = p.CellPar[i] * t;
    }

    for (subIter it=par.SubPar.begin(); it != par.SubPar.end(); ++it)
    {
        subConstIter it2 = p.SubPar.find(it->first);
        it->second = it2->second * t;
    }

    for (reachIter it=par.ReachPar.begin(); it != par.ReachPar.end(); ++it)
    {
        reachConstIter it2 = p.ReachPar.find(it->first);
        it->second = it2->second * t;
    }

    par.RiverPar = p.RiverPar * t;

    return par;
}
//---------------------------------------------------------------------------
CParSet operator-(const CParSet& p, const CParSet& n)
{
    CParSet par(p);

    for (unsigned int i=0; i<p.LandCount(); ++i)
    {
        par.CellPar[i] = p.CellPar[i] - n.CellPar[i];
    }

    for (subIter it=par.SubPar.begin(); it != par.SubPar.end(); ++it)
    {
        subConstIter it2 = p.SubPar.find(it->first);
        subConstIter it3 = n.SubPar.find(it->first);
        it->second = it2->second - it3->second;
    }

    for (reachIter it=par.ReachPar.begin(); it != par.ReachPar.end(); ++it)
    {
        reachConstIter it2 = p.ReachPar.find(it->first);
        reachConstIter it3 = n.ReachPar.find(it->first);
        it->second = it2->second - it3->second;
    }

    par.RiverPar = p.RiverPar - n.RiverPar;

    return par;
}
//---------------------------------------------------------------------------
bool operator<=(const CParSet& lhs, const CParSet& rhs)
{
    for (unsigned int i=0; i<rhs.LandCount(); ++i)
    {
        if (lhs.CellPar[i] > rhs.CellPar[i]) return false;
    }

    for (subConstIter it=lhs.SubPar.begin(); it != lhs.SubPar.end(); ++it)
    {
        subConstIter it2 = rhs.SubPar.find(it->first);
        if (it->second > it2->second) return false;
    }

    for (reachConstIter it=lhs.ReachPar.begin(); it != lhs.ReachPar.end(); ++it)
    {
        reachConstIter it2 = rhs.ReachPar.find(it->first);
        if (it->second > it2->second) return false;
    }

    return (lhs.RiverPar <= rhs.RiverPar);
}
//---------------------------------------------------------------------------
unsigned int CParSet::IncludedCount(bool countAll)
{
    unsigned int count;

    if (hetero)
    {
        count = RiverPar.IncludedCount(countAll);

        for (unsigned int i=0; i<landCount; ++i)
        {
            count += CellPar[i].IncludedCount(countAll);
        }

        for (subIter it=SubPar.begin(); it != SubPar.end(); ++it)
        {
            count += it->second.IncludedCount(countAll);
        }

        for (reachIter it=ReachPar.begin(); it != ReachPar.end(); ++it)
        {
            count += it->second.IncludedCount(countAll);
        }
    }
    else
    {
        subIter itSub=SubPar.begin();
        reachIter itReach=ReachPar.begin();

        count = (CellPar[0].IncludedCount(countAll)
                    + RiverPar.IncludedCount(countAll)
                    + itReach->second.IncludedCount(countAll)
                    + itSub->second.IncludedCount(countAll));
    }

    return count;
}
//---------------------------------------------------------------------------
double CParSet::GetParByIndex(const unsigned int index, bool all)
{
    unsigned int iCount = RiverPar.IncludedCount(all);
    unsigned int cCount = 0;
    unsigned int rCount = 0;
    unsigned int sCount = 0;

    unsigned int land = 0;
    unsigned int landIndex = index, reachIndex = 0, subIndex = 0;
//    std::string reachNum, subNum;

    subIter sub = SubPar.begin();
//    subNum = sub->first;

    reachIter reach = ReachPar.begin();
//    reachNum = reach->first;

    if (hetero)
    {
        for (unsigned int i=0; i<landCount; ++i)
        {
            cCount += CellPar[i].IncludedCount(all);

            if (index >= cCount)
            {
                land = i+1;
                landIndex = index - cCount;
            }
        }

        reachIndex = index - (cCount + iCount);

        for (reachIter it=ReachPar.begin(); it != ReachPar.end(); ++it)
        {
            rCount += it->second.IncludedCount(all);
            reach = it;

            if (index >= (cCount + iCount + rCount))
            {
                reachIndex = index - (cCount + iCount + rCount);
            }
        }

        subIndex = reachIndex;

        for (subIter it=SubPar.begin(); it != SubPar.end(); ++it)
        {
            sCount += it->second.IncludedCount(all);
            sub = it;

            if (index >= (cCount + iCount + rCount + sCount))
            {
                subIndex = index - (cCount + iCount + rCount + sCount);
            }
        }
    }
    else
    {
        land = homoLand;
//        reachNum = homoReach;
        reach = ReachPar.find(homoReach);
//        subNum = homoReach;
        sub = SubPar.find(homoReach);

        cCount = CellPar[land].IncludedCount(all);
        rCount = reach->second.IncludedCount(all);
        sCount = sub->second.IncludedCount(all);

        landIndex = index;
        reachIndex = (index - cCount - iCount);
        subIndex = (index - cCount - iCount - rCount);
    }

    if (index < cCount)
    {
        return CellPar[land].GetParByIndex(landIndex, all);
    }

    if (index < (cCount + iCount))
    {
        return RiverPar.GetParByIndex((index - cCount), all);
    }

    if (index < (cCount + iCount + rCount))
    {
//        reachIter it = ReachPar.find(reachNum);
        return reach->second.GetParByIndex(reachIndex, all);
    }

//    subIter it = SubPar.find(subNum);
    return sub->second.GetParByIndex(subIndex, all);
}
//---------------------------------------------------------------------------
void CParSet::Homogeneous(unsigned int land, std::string& reach)
{
    homoLand = land;
    homoReach = reach;
    hetero = false;
}
//---------------------------------------------------------------------------
void CParSet::Heterogeneous(void)
{
    hetero = true;
    homoLand = 0;
//    homoReach = ReachPar.begin()->first;
}
//---------------------------------------------------------------------------
const std::string CParSet::GetNameByIndex(const unsigned int index, bool all)
{
    unsigned int iCount = RiverPar.IncludedCount(all);
    unsigned int cCount = 0;
    unsigned int rCount = 0;
    unsigned int sCount = 0;

    unsigned int land = 0;
    unsigned int landIndex = index, reachIndex = 0, subIndex = 0;
//    std::string reachNum, subNum;

    subIter sub = SubPar.begin();
//    subNum = sub->first;

    reachIter reach = ReachPar.begin();
//    reachNum = reach->first;

    if (hetero)
    {
        for (unsigned int i=0; i<landCount; ++i)
        {
            cCount += CellPar[i].IncludedCount(all);

            if (index >= cCount)
            {
                land = i+1;
                landIndex = index - cCount;
            }
        }

        reachIndex = index - (cCount + iCount);

        for (reachIter it=ReachPar.begin(); it != ReachPar.end(); ++it)
        {
            rCount += it->second.IncludedCount(all);
            reach = it;

            if (index >= (cCount + iCount + rCount))
            {
//                reachNum = (++reach)->first;
                reachIndex = index - (cCount + iCount + rCount);
            }
        }

        subIndex = reachIndex;

        for (subIter it=SubPar.begin(); it != SubPar.end(); ++it)
        {
            sCount += it->second.IncludedCount(all);
            sub = it;

            if (index >= (cCount + iCount + rCount + sCount))
            {
//                subNum = (++sub)->first;
                subIndex = index - (cCount + iCount + rCount + sCount);
            }
        }
    }
    else
    {
        land = homoLand;
//        reachNum = homoReach;
        reach = ReachPar.find(homoReach);
//        subNum = homoReach;
        sub = SubPar.find(homoReach);

        cCount = CellPar[land].IncludedCount(all);
        rCount = reach->second.IncludedCount(all);
        sCount = sub->second.IncludedCount(all);

        landIndex = index;
        reachIndex = (index - cCount - iCount);
        subIndex = (index - cCount - iCount - rCount);
    }

    if (index < cCount)
    {
        std::string parStr = CellPar[land].GetNameByIndex(landIndex, all);
        std::string name = LongLand[land] + " " + parStr;

        return name;
    }

    if (index < (cCount + iCount))
    {
        return RiverPar.GetNameByIndex((index - cCount), all);
    }

    if (index < (cCount + iCount + rCount))
    {
        std::ostringstream name;

//        reachIter it = ReachPar.find(reachNum);

        name << reach->second.name;// << " (" << reachNum << ") ";
        name << reach->second.GetNameByIndex(reachIndex, all);

        return name.str();
    }

    std::ostringstream name;

//    reachIter it = ReachPar.find(subNum);
//    subIter it2 = SubPar.find(subNum);

    name << reach->second.name;// << " (" << subNum << ") ";
    name << sub->second.GetNameByIndex(subIndex, all);

    return name.str();
}
//---------------------------------------------------------------------------
void CParSet::LatinHypercubeInit(unsigned int groups)
{
    numLatinHypercubeGroups = groups;

    // Initialise LH arrays in all parameters
    for (unsigned int i=0; i<landCount; ++i)
    {
        CellPar[i].Initial.LatinHypercubeInit(groups);
        CellPar[i].Par.LatinHypercubeInit(groups);
        CellPar[i].TC.LatinHypercubeInit(groups);
    }

    RiverPar.Initial.LatinHypercubeInit(groups);
    RiverPar.Pars.LatinHypercubeInit(groups);

    for (reachIter it=ReachPar.begin(); it != ReachPar.end(); ++it)
    {
        it->second.Par.LatinHypercubeInit(groups);
    }

    for (subIter it=SubPar.begin(); it != SubPar.end(); ++it)
    {
        it->second.Par.LatinHypercubeInit(groups);
        it->second.Land.LatinHypercubeInit(groups);
        it->second.Dep.LatinHypercubeInit(groups);
    }
}
//---------------------------------------------------------------------------
void CParSet::LatinHypercubeSetRange(CParSet& min, CParSet& max)
{
    for (unsigned int i=0; i<landCount; ++i)
    {
        CellPar[i].Initial.LatinHypercubeSetRange(min.CellPar[i].Initial, max.CellPar[i].Initial);
        CellPar[i].Par.LatinHypercubeSetRange(min.CellPar[i].Par, max.CellPar[i].Par);
        CellPar[i].TC.LatinHypercubeSetRange(min.CellPar[i].TC, max.CellPar[i].TC);
    }

    RiverPar.Initial.LatinHypercubeSetRange(min.RiverPar.Initial, max.RiverPar.Initial);
    RiverPar.Pars.LatinHypercubeSetRange(min.RiverPar.Pars, max.RiverPar.Pars);

    for (reachIter it=ReachPar.begin(); it != ReachPar.end(); ++it)
    {
        reachIter minIter = min.ReachPar.find(it->first);
        reachIter maxIter = max.ReachPar.find(it->first);

        it->second.Par.LatinHypercubeSetRange(minIter->second.Par, maxIter->second.Par);
    }

    for (subIter it=SubPar.begin(); it != SubPar.end(); ++it)
    {
        subIter minIter = min.SubPar.find(it->first);
        subIter maxIter = max.SubPar.find(it->first);

        it->second.Par.LatinHypercubeSetRange(minIter->second.Par, maxIter->second.Par);
        it->second.Land.LatinHypercubeSetRange(minIter->second.Land, maxIter->second.Land);
        it->second.Dep.LatinHypercubeSetRange(minIter->second.Dep, maxIter->second.Dep);
    }
}
//---------------------------------------------------------------------------
void CParSet::LatinHypercubeGenerate(void)
{
    // Fill parameter LH arrays with random permutation
    for (unsigned int i=0; i<landCount; ++i)
    {
        CellPar[i].Initial.LatinHypercubeGenerate();
        CellPar[i].Par.LatinHypercubeGenerate();
        CellPar[i].TC.LatinHypercubeGenerate();
    }

    RiverPar.Initial.LatinHypercubeGenerate();
    RiverPar.Pars.LatinHypercubeGenerate();

    for (reachIter it=ReachPar.begin(); it != ReachPar.end(); ++it)
    {
        it->second.Par.LatinHypercubeGenerate();
    }

    for (subIter it=SubPar.begin(); it != SubPar.end(); ++it)
    {
        it->second.Par.LatinHypercubeGenerate();
        it->second.Land.LatinHypercubeGenerate();
        it->second.Dep.LatinHypercubeGenerate();
    }
}
//---------------------------------------------------------------------------
void CParSet::ExcludeZeroRangePars(void)
{
    // Fill parameter LH arrays with random permutation
    for (unsigned int i=0; i<landCount; ++i)
    {
        CellPar[i].Initial.ExcludeZeroRangePars();
        CellPar[i].Par.ExcludeZeroRangePars();
        CellPar[i].TC.ExcludeZeroRangePars();
    }

    RiverPar.Initial.ExcludeZeroRangePars();
    RiverPar.Pars.ExcludeZeroRangePars();

    for (reachIter it=ReachPar.begin(); it != ReachPar.end(); ++it)
    {
        it->second.Par.ExcludeZeroRangePars();
    }

    for (subIter it=SubPar.begin(); it != SubPar.end(); ++it)
    {
        it->second.Par.ExcludeZeroRangePars();
        it->second.Land.ExcludeZeroRangePars();
        it->second.Dep.ExcludeZeroRangePars();
    }
}
//---------------------------------------------------------------------------
void CParSet::LatinHypercubeGetSample(unsigned int group)
{
    // Use LH to get random samples from parameter space
    for (unsigned int i=0; i<landCount; ++i)
    {
        CellPar[i].Initial.LatinHypercubeGetSample(group);
        CellPar[i].Par.LatinHypercubeGetSample(group);
        CellPar[i].TC.LatinHypercubeGetSample(group);
    }

    RiverPar.Initial.LatinHypercubeGetSample(group);
    RiverPar.Pars.LatinHypercubeGetSample(group);

    for (reachIter it=ReachPar.begin(); it != ReachPar.end(); ++it)
    {
        it->second.Par.LatinHypercubeGetSample(group);
    }

    for (subIter it=SubPar.begin(); it != SubPar.end(); ++it)
    {
        it->second.Par.LatinHypercubeGetSample(group);
        it->second.Land.LatinHypercubeGetSample(group);
        it->second.Dep.LatinHypercubeGetSample(group);
    }
}
//---------------------------------------------------------------------------
CParSet::~CParSet()
{
}

//---------------------------------------------------------------------------
void CParSet::Init(void)
{
    CellPar.clear();

    for (unsigned int l=0; l<landCount; l++)
    {
        CCellPar par;
        CellPar.push_back(par);
    }
}

//---------------------------------------------------------------------------
bool CParSet::LoadFromFile(std::string const& FileName, CStructure** structure)
{
    bool newStructure = false;

    CStream stream;

    ParFileName = std::string(FileName);

    std::ifstream in(FileName.c_str());

    std::string rec;
    getline(in, rec);

    Name = utils::trim( rec );

    if ((*structure) == NULL)
    {
        (*structure) = new CStructure();
        (*structure)->Generated(true);
        (*structure)->Changed(true);
        (*structure)->Name(Name);
        newStructure = true;
    }

    std::string shortland;
    getline(in, shortland);
    shortland = utils::trim( shortland );
    CreateStringList(ShortLand, shortland.c_str());

    std::string longland;
    getline(in, longland);
    longland = utils::trim( longland );
    CreateStringList(LongLand, longland.c_str());

    landCount = ShortLand.size();
    Init();

    for (int i=0; i<NUM_CELL_INITIAL; ++i)
    {
        std::string input;
        getline(in, input);
        std::istringstream is(input);

        for (unsigned int j=0; j<landCount; ++j)
        {
            is >> CellPar[j].Initial[i];
        }
    }

    // Read in start date
    unsigned int day, month, year;
    char c;

    std::string input;
    getline(in, input);
    std::istringstream is1(input);

    is1 >> day >> c >> month >> c >> year;
    StartDate = dateCl::Date(month, day, year);

    input.clear();
    getline(in, input);
    std::istringstream is2(input);

    is2 >> TimeSteps;

    for (int i=0; i<NUM_CELL_PARS; ++i)
    {
        std::string input;
        getline(in, input);
        std::istringstream is(input);

        for (unsigned int j=0; j<landCount; ++j)
        {
            is >> CellPar[j].Par[i];
        }
    }

    for (int i=0; i<NUM_CELL_TC; ++i)
    {
        std::string input;
        getline(in, input);
        std::istringstream is(input);

        for (unsigned int j=0; j<landCount; ++j)
        {
            is >> CellPar[j].TC[i];
        }
    }

    in >> RiverPar;

    std::string input2;
    getline(in, input2);
    std::istringstream is4(input2);

    is4 >> Reaches;

    float lastLat = 0.0, lastLong = 0.0;
    std::string lastReachID;

    for (int i=0; i<Reaches; ++i)
    {
        CReachPar reach;

        std::string id;
        getline(in, id);

        id = utils::trim( id );

        std::string name;
        getline(in, name);
        reach.Name( utils::trim ( name ) );

        CSubPar sub(landCount);

        std::string inputSub;
        getline(in, inputSub);
        std::istringstream is5(inputSub);

        is5 >> sub.Par[0];

        getline(in, inputSub);
        std::istringstream is6(inputSub);

        for (unsigned int j=0; j<landCount; j++)
        {
            is6 >> sub.Land[j];
        }

        getline(in, inputSub);
        std::istringstream is8(inputSub);

        for (unsigned int j=1; j<NUM_SUB_PAR; j++)
        {
            is8 >> sub.Par[j];
        }

        getline(in, inputSub);
        std::istringstream is9(inputSub);

        for (unsigned int j=0; j<NUM_DEP; j++)
        {
            is9 >> sub.Dep[j];
        }

        in >> reach;

        ReachPar.insert(reachValueType(id, reach));
        SubPar.insert(subValueType(id, sub));

        if (newStructure)
        {
            CReach r(id, name);
            r.Coords(lastLat, lastLong, reach.Par[9], reach.Par[10]);

            if (i > 0)
            {
                r.AddInput(lastReachID);
            }

            stream.AddReach(r);

            lastLat = reach.Par[9];
            lastLong = reach.Par[10];
            lastReachID = id;
        }
    }

    if (newStructure)
    {
        stream.Name(Name);
        stream.Pars(RiverPar);
        (*structure)->AddStream(stream);
    }

	loaded = true;

    return true;
}

//---------------------------------------------------------------------------
bool CParSet::readXml (std::string const& filename, CStructure** structure)
{
    bool newStructure = false;

    CStream stream;
    if (*structure) delete (*structure);

 //   if ((*structure) == NULL)
 //   {
        (*structure) = new CStructure;
        (*structure)->Generated(true);
        (*structure)->Changed(true);
        (*structure)->Name(Name);
        newStructure = true;
 //   }

    /////////////////////////////////////////////////////////////

    tinyxml2::XMLDocument xmlDoc;
    xmlDoc.LoadFile( filename.c_str() );
//    XMLCheckResult( eResult );

    tinyxml2::XMLNode* pRoot = xmlDoc.FirstChild();
    if ( !pRoot ) return false;

    std::string version = pRoot->ToElement()->Attribute( "version" );
    Name = pRoot->ToElement()->Attribute( "description" );

    tinyxml2::XMLNode * parameterBlock = pRoot->FirstChildElement( "ParameterContainer" );
    tinyxml2::XMLNode * systemGroup = parameterBlock->FirstChildElement( "ParameterGroup" );
    tinyxml2::XMLNode * systemContainer = systemGroup->FirstChildElement( "ParameterContainer" );
    tinyxml2::XMLNode * parameter = systemContainer->FirstChildElement( "Parameter" );

    CPar timeSteps_("TimeSteps", TimeSteps);
    timeSteps_.readXml( parameter );
    TimeSteps = (int)timeSteps_.Value2();

    parameter = parameter->NextSiblingElement( "Parameter" );
    readXmlStartDate( parameter );

    /////////////////////////////////////////////////////////////

    int landSize;
    tinyxml2::XMLNode * group = systemGroup->NextSibling();
    group->ToElement()->QueryIntAttribute( "count", &landSize );

    landCount = landSize;
    Init();

    LongLand.resize( landCount );
    ShortLand.resize( landCount );

    tinyxml2::XMLNode * groupItem = group->FirstChildElement("ParameterContainer");

    for (unsigned int land=0; land<landCount; ++land)
    {
        LongLand[land] = groupItem->ToElement()->Attribute( "name" );
        ShortLand[land] = groupItem->ToElement()->Attribute( "name" );

        CellPar[land].readXml( groupItem );

        groupItem = groupItem->NextSiblingElement( "ParameterContainer" );
    }

    /////////////////////////////////////////////////////////////

    int reachCount;
    group = group->NextSibling();
    group->ToElement()->QueryIntAttribute( "count", &reachCount );

    Reaches = reachCount;

    groupItem = group->FirstChildElement("ParameterContainer");

    float lastLat = 0.0, lastLong = 0.0;
    std::string lastReachID;

    for (int i=0; i<Reaches; ++i)
    {
        std::string reachName, reachID;
        reachID = groupItem->ToElement()->Attribute( "name" );
        reachName = groupItem->ToElement()->Attribute( "name" );

        CReachPar reach;
        reach.Name( reachName );

        tinyxml2::XMLNode * subNode = reach.readXml( groupItem );
        ReachPar.insert(reachValueType(reachID, reach));

        if (newStructure)
        {
            CReach r(reachID, reachName);
            r.Coords(lastLat, lastLong, reach.Par[9], reach.Par[10]);

            if (i > 0)
            {
                r.AddInput(lastReachID);
            }

            stream.AddReach(r);

            lastLat = reach.Par[9];
            lastLong = reach.Par[10];
            lastReachID = reachID;
        }

        CSubPar sub(landCount);
        sub.readXml( subNode->PreviousSibling() );
        SubPar.insert(subValueType(reachID, sub));

        groupItem = groupItem->NextSiblingElement( "ParameterContainer" );
    }

   if (newStructure)
    {
        stream.Name(Name);
        stream.ID(Name);
//        stream.Pars(RiverPar);
        (*structure)->AddStream(stream);
    }

 //   if (*structure) delete (*structure);
 //   (*structure) = new CStructure;

    /////////////////////////////////////////////////////////////

    group = group->NextSibling();
    (*structure)->readXml( group );

    (*structure)->Reset();
    stream = (*structure)->GetCurrentStream();
    RiverPar = stream.Pars();

    /////////////////////////////////////////////////////////////

 //   group = group->NextSibling();
 //   groupItem = group->FirstChildElement("ParameterContainer");

//    for (int i=0; i<Reaches; ++i)
//    {
//        CSubPar sub(landCount);

//        std::string reachID;
//        reachID = groupItem->ToElement()->Attribute( "name" );

//        sub.readXml( groupItem );

//        SubPar.insert(subValueType(reachID, sub));

//        groupItem = groupItem->NextSiblingElement( "ParameterContainer" );
//    }

    /////////////////////////////////////////////////////////////

//    if (newStructure)
//    {
//        stream.Name(Name);
//        stream.Pars(RiverPar);
//        structure->AddStream(stream);
//    }

	loaded = true;

    return true;
}
//---------------------------------------------------------------------------
bool CParSet::writeXml (std::string const& filename, CStructure** structure) const
{
//    CPar timeSteps_("TimeSteps", TimeSteps, "unsigned int");
    CPar timeSteps_("TimeSteps", TimeSteps);

    tinyxml2::XMLDocument xmlDoc;
    tinyxml2::XMLNode * pRoot = xmlDoc.NewElement("ParameterSet");

    pRoot->ToElement()->SetAttribute( "model",          "INCA-ON (THE)" );
    pRoot->ToElement()->SetAttribute( "version",        VERSION.c_str() );
    pRoot->ToElement()->SetAttribute( "description",    Name.c_str() );

    xmlDoc.InsertFirstChild( pRoot );


    //////////////////

    /* Landscape units indexer */
    tinyxml2::XMLNode * indexerBlock = pRoot->GetDocument()->NewElement( "Indexers" );
    indexerBlock->ToElement()->SetAttribute( "count", "3" );

    tinyxml2::XMLNode * indexer1 = indexerBlock->GetDocument()->NewElement( "Indexer" );
    indexer1->ToElement()->SetAttribute( "name", "Landscape units" );
    indexer1->ToElement()->SetAttribute( "count", landCount );

    for (unsigned int land=0; land<landCount; ++land)
    {
        tinyxml2::XMLElement * index = indexer1->GetDocument()->NewElement( "index" );
        index->SetText( LongLand[land].c_str() );
        indexer1->InsertEndChild( index );
    }

    indexerBlock->InsertEndChild( indexer1 );

    /* Stream indexer */
    (*structure)->SetDirection(CStructure::MainStemLast);
    (*structure)->Reset();

    tinyxml2::XMLNode * indexer4 = indexerBlock->GetDocument()->NewElement( "Indexer" );
    indexer4->ToElement()->SetAttribute( "name", "Stream" );
    indexer4->ToElement()->SetAttribute( "count", (*structure)->size() );

    do
    {
        CStream stream = (*structure)->GetCurrentStream();
        stream.Reset();

        tinyxml2::XMLElement * index = indexer4->GetDocument()->NewElement( "index" );
//        index->SetText( stream.ID().c_str() );
        index->SetText( stream.Name().c_str() );
        indexer4->InsertEndChild( index );
    }
    while ((*structure)->NextStream());

    indexerBlock->InsertEndChild( indexer4 );


    /* Reach indexer */
    tinyxml2::XMLNode * indexer2 = indexerBlock->GetDocument()->NewElement( "Indexer" );
    indexer2->ToElement()->SetAttribute( "name", "Reaches" );
    indexer2->ToElement()->SetAttribute( "count", Reaches );

    (*structure)->Reset();

    do
    {
        CStream stream = (*structure)->GetCurrentStream();
        stream.Reset();

        do
        {
            reachConstIter reach = ReachPar.find(stream.GetCurrentReachID());

            tinyxml2::XMLElement * index = indexer2->GetDocument()->NewElement( "index" );
            index->SetText( reach->second.Name().c_str() );
//            index->SetText( stream.GetCurrentReachID().c_str() );
            indexer2->InsertEndChild( index );
        }
        while (stream.NextReach());

    }
    while ((*structure)->NextStream());

    indexerBlock->InsertEndChild( indexer2 );

    /* Subcatchments indexer */
/*    tinyxml2::XMLNode * indexer3 = indexerBlock->GetDocument()->NewElement( "Indexer" );
    indexer3->ToElement()->SetAttribute( "name", "Subcatchments" );
    indexer3->ToElement()->SetAttribute( "count", Reaches );

    (*structure)->Reset();

    do
    {
        CStream stream = (*structure)->GetCurrentStream();
        stream.Reset();

        do
        {
            reachConstIter reach = ReachPar.find(stream.GetCurrentReachID());

            tinyxml2::XMLElement * index = indexer3->GetDocument()->NewElement( "index" );
            index->SetText( reach->second.Name().c_str() );
//            index->SetText( stream.GetCurrentReachID().c_str() );
            indexer3->InsertEndChild( index );
        }
        while (stream.NextReach());

    }
    while ((*structure)->NextStream());

    indexerBlock->InsertEndChild( indexer3 );
 */
    pRoot->InsertEndChild( indexerBlock );

    //////////////////

    tinyxml2::XMLNode * parmeterBlock = pRoot->GetDocument()->NewElement( "ParameterContainer" );
    parmeterBlock->ToElement()->SetAttribute( "name", "INCA-ON (THE)" );
    parmeterBlock->ToElement()->SetAttribute( "count", "4" );

    //////////////////

    tinyxml2::XMLNode * groupSystem = parmeterBlock->GetDocument()->NewElement( "ParameterGroup" );
    groupSystem->ToElement()->SetAttribute( "name", "System" );
    groupSystem->ToElement()->SetAttribute( "count", "1" );

    tinyxml2::XMLNode * systemItem = groupSystem->GetDocument()->NewElement( "ParameterContainer" );
    systemItem->ToElement()->SetAttribute( "name", "System" );
    systemItem->ToElement()->SetAttribute( "count", "2" );

    timeSteps_.writeXml( systemItem );
    writeXmlStartDate( systemItem );

    groupSystem->InsertEndChild( systemItem );
    parmeterBlock->InsertEndChild( groupSystem );

    //////////////////

    tinyxml2::XMLNode * group = parmeterBlock->GetDocument()->NewElement( "ParameterGroup" );
    group->ToElement()->SetAttribute( "name", "Landscape units" );
    group->ToElement()->SetAttribute( "indexer", "Landscape units" );
    group->ToElement()->SetAttribute( "count", landCount );

    unsigned int cellParCount = NUM_CELL_INITIAL + NUM_CELL_PARS + NUM_CELL_TC;

    for (unsigned int land=0; land<landCount; ++land)
    {
        tinyxml2::XMLNode * groupItem = group->GetDocument()->NewElement( "ParameterContainer" );
        groupItem->ToElement()->SetAttribute( "name", LongLand[land].c_str() );
        groupItem->ToElement()->SetAttribute( "count", cellParCount );
        CellPar[land].writeXml( groupItem );
        group->InsertEndChild( groupItem );
    }

    parmeterBlock->InsertEndChild( group );

    /////////////////

    group = parmeterBlock->GetDocument()->NewElement( "ParameterGroup" );
    group->ToElement()->SetAttribute( "name", "Reaches" );
    group->ToElement()->SetAttribute( "indexer", "Reaches" );
    group->ToElement()->SetAttribute( "count", (unsigned int)ReachPar.size() );

    (*structure)->Reset();

    do
    {
        CStream stream = (*structure)->GetCurrentStream();
        stream.Reset();

        do
        {
            reachConstIter reach = ReachPar.find(stream.GetCurrentReachID());
            subConstIter sub = SubPar.find(stream.GetCurrentReachID());

            tinyxml2::XMLNode * groupItem = group->GetDocument()->NewElement( "ParameterContainer" );
//            groupItem->ToElement()->SetAttribute( "name", reach->first.c_str() );
            groupItem->ToElement()->SetAttribute( "name", reach->second.Name().c_str() );
            groupItem->ToElement()->SetAttribute( "count", NUM_REACH_PAR +NUM_SUB_PAR + NUM_DEP + 1);

            reach->second.writeXml( groupItem );
            sub->second.writeXml( groupItem, LongLand );
            group->InsertEndChild( groupItem );
        }
        while (stream.NextReach());

    }
    while ((*structure)->NextStream());

    parmeterBlock->InsertEndChild( group );

    ////////////////////

    group = parmeterBlock->GetDocument()->NewElement( "ParameterGroup" );
    group->ToElement()->SetAttribute( "name", "Stream" );
    group->ToElement()->SetAttribute( "indexer", "Stream" );
    group->ToElement()->SetAttribute( "count", (*structure)->size() );

    (*structure)->writeXml( group );

    parmeterBlock->InsertEndChild( group );

    /////////////////
/*
    group = parmeterBlock->GetDocument()->NewElement( "ParameterGroup" );
    group->ToElement()->SetAttribute( "name", "Subcatchments" );
    group->ToElement()->SetAttribute( "indexer", "Subcatchments" );
    group->ToElement()->SetAttribute( "count", (unsigned int)ReachPar.size() );

    unsigned int subParCount = NUM_SUB_PAR + NUM_DEP;

    (*structure)->Reset();

    do
    {
        CStream stream = (*structure)->GetCurrentStream();
        stream.Reset();

        do
        {
            subConstIter sub = SubPar.find(stream.GetCurrentReachID());
            reachConstIter reach = ReachPar.find(stream.GetCurrentReachID());

            tinyxml2::XMLNode * groupItem = group->GetDocument()->NewElement( "ParameterContainer" );
            std::string name = sub->first;
//            groupItem->ToElement()->SetAttribute( "name", sub->first.c_str() );
            groupItem->ToElement()->SetAttribute( "name", reach->second.Name().c_str() );
            groupItem->ToElement()->SetAttribute( "count", subParCount );

            sub->second.writeXml( groupItem );
            group->InsertEndChild( groupItem );
        }
        while (stream.NextReach());

    }
    while ((*structure)->NextStream());

    parmeterBlock->InsertEndChild( group );
*/
    /////////////////

    pRoot->InsertEndChild( parmeterBlock );

    xmlDoc.SaveFile( filename.c_str() );

    return true;
}
//---------------------------------------------------------------------------
bool CParSet::SaveToFile(CStructure** structure)
{
    return SaveToFile(ParFileName.c_str(), structure);
}
//---------------------------------------------------------------------------
bool CParSet::SaveToFile(std::string const& FileName, CStructure** structure)
{
    std::ofstream out(FileName.c_str());

    out << Name << std::endl;

    for (unsigned int i=0; i<ShortLand.size(); ++i)
    {
        out << ShortLand[i];
        if (i < ShortLand.size() - 1) out << ",";
    }
    out << std::endl;

    for (unsigned int i=0; i<LongLand.size(); ++i)
    {
        out << LongLand[i];
        if (i < LongLand.size() - 1) out << ",";
    }
    out << std::endl;

    for (int i=0; i<NUM_CELL_INITIAL; ++i)
    {
        for (unsigned int j=0; j<landCount; ++j)
        {
            out << CellPar[j].Initial[i] << "\t";
        }
        out << std::endl;
    }

    StartDate.setFormat(dateCl::Date::DMY);
    out << StartDate << std::endl;

    out << TimeSteps <<std::endl;

    for (int i=0; i<NUM_CELL_PARS; ++i)
    {
        for (unsigned int j=0; j<landCount; ++j)
        {
            out << CellPar[j].Par[i] << "\t";
        }
        out << std::endl;
    }

    for (int i=0; i<NUM_CELL_TC; ++i)
    {
        for (unsigned int j=0; j<landCount; ++j)
        {
            out << CellPar[j].TC[i] << "\t";
        }
        out << std::endl;
    }

    for (int i=0; i<NUM_RIVER_INITIAL; ++i)
    {
        out << RiverPar.Initial[i] << std::endl;
    }

    for (int i=0; i<NUM_RIVER_PARS; ++i)
    {
        out << RiverPar.Pars[i] << std::endl;
    }

    out << Reaches << std::endl;

    (*structure)->SetDirection(CStructure::MainStemFirst);
    (*structure)->Reset();

    do
    {
        CStream stream = (*structure)->GetCurrentStream();
        stream.Reset();

        do
        {
            reachIter reach = ReachPar.find(stream.GetCurrentReachID());
            subIter sub = SubPar.find(stream.GetCurrentReachID());

            // Reach name and ID
            out << reach->first << std::endl;
            out << reach->second.Name() << std::endl;

            // Sub-catchment area
            out << sub->second.Par[0] << std::endl;

            // Land use percentages
            for (unsigned int j=0; j<landCount; ++j)
            {
                out << sub->second.Land[j] << "\t";
            }
            out << std::endl;

            // Sub-catchment parameters
            for (int j=1; j<NUM_SUB_PAR; ++j)
            {
                out << sub->second.Par[j] << "\t";
            }
            out << std::endl;

            // Deposition loads
            for (int j=0; j<NUM_DEP; ++j)
            {
                out << sub->second.Dep[j] << "\t";
            }
            out << std::endl;

            // Reach parameters
            for (int j=0; j<5; ++j)
            {
                out << reach->second.Par[j] << "\t";
            }
            out << std::endl;

            // Reach parameters
            for (int j=5; j<9; ++j)
            {
                out << reach->second.Par[j] << "\t";
            }

            out << reach->second.Effluent << std::endl;

            // Reach parameters
            for (int j=9; j<NUM_REACH_PAR; ++j)
            {
                out << reach->second.Par[j] << "\t";
            }

            out << std::endl;
        }
        while (stream.NextReach());

    }
    while ((*structure)->NextStream());

    out.close();

    return true;
}
//---------------------------------------------------------------------------
bool CParSet::ImportBranching1v0(std::string const& FileName, CStructure** structure)
{
    bool newStructure = false;

    CStream stream;

    ParFileName = std::string(FileName);

    std::ifstream in(FileName.c_str());

    std::string rec;
    getline(in, rec);

    Name = utils::trim( rec );

    if ((*structure) == NULL)
    {
        (*structure) = new CStructure;
        (*structure)->Generated(true);
        (*structure)->Changed(true);
        (*structure)->Name(Name);
        newStructure = true;
    }

    std::string shortland;
    getline(in, shortland);
    shortland = utils::trim( shortland );
    CreateStringList(ShortLand, shortland.c_str());

    std::string longland;
    getline(in, longland);
    longland = utils::trim( longland );
    CreateStringList(LongLand, longland.c_str());

    landCount = ShortLand.size();
    Init();

    for (int i=0; i<6; ++i)
    {
        std::string input;
        getline(in, input);
        std::istringstream is(input);

        for (unsigned int j=0; j<landCount; ++j)
        {
            is >> CellPar[j].Initial[i];
        }
    }

    for (unsigned int j=0; j<landCount; ++j)
    {
        CellPar[j].Initial[6] = 5000.0;
        CellPar[j].Initial[7] = 0.0;
        CellPar[j].Initial[8] = 0.0;
        CellPar[j].Initial[9] = 0.0;
    }

    // Read in start date
    unsigned int day, month, year;
    char c;

    std::string input;
    getline(in, input);
    std::istringstream is1(input);

    is1 >> day >> c >> month >> c >> year;
    StartDate = dateCl::Date(month, day, year);

    input.clear();
    getline(in, input);
    std::istringstream is2(input);

    is2 >> TimeSteps;

    for (int i=0; i<44; ++i)
    {
        std::string input;
        getline(in, input);
        std::istringstream is(input);

        for (unsigned int j=0; j<landCount; ++j)
        {
            is >> CellPar[j].Par[i];
        }
    }

    for (unsigned int j=0; j<landCount; ++j)
    {
        CellPar[j].Par[44] = 0.001;
        CellPar[j].Par[45] = 1.0;
        CellPar[j].Par[46] = 10.0;
        CellPar[j].Par[47] = 0.00002;
        CellPar[j].Par[48] = 0.33;

        CellPar[j].Par[49] = 1.0;
        CellPar[j].Par[50] = 1.0;
        CellPar[j].Par[51] = 0.0;
        CellPar[j].Par[52] = 3.0;
        CellPar[j].Par[53] = 0.0;
        CellPar[j].Par[54] = 0.0;
        CellPar[j].Par[55] = 60.0;
        CellPar[j].Par[56] = 0.0;
        CellPar[j].Par[57] = 1.0;
        CellPar[j].Par[58] = 1000.0;
        CellPar[j].Par[62] = 0.1;
        CellPar[j].Par[63] = 200.0;
    }

    for (int i=0; i<3; ++i)
    {
        std::string input;
        getline(in, input);
        std::istringstream is(input);

        for (unsigned int j=0; j<landCount; ++j)
        {
            is >> CellPar[j].TC[i];
        }
    }

//    in >> RiverPar;
    for (int i=0; i<3; ++i)
    {
        std::string input;
        getline(in, input);
        std::istringstream is(input);

        is >> RiverPar.Initial[i];
    }

    RiverPar.Initial[3] = 1.0;

    for (int i=0; i<NUM_RIVER_PARS; ++i)
    {
        std::string input;
        getline(in, input);
        std::istringstream is(input);

        is >> RiverPar.Pars[i];
    }

    std::string input2;
    getline(in, input2);
    std::istringstream is4(input2);

    is4 >> Reaches;

    float lastLat = 0.0, lastLong = 0.0;
    std::string lastReachID;

    for (int i=0; i<Reaches; ++i)
    {
        CReachPar reach;

        std::string id;
        getline(in, id);

        id = utils::trim( id );

        std::string name;
        getline(in, name);
        reach.Name( utils::trim ( name ) );

        CSubPar sub(landCount);

        std::string inputSub;
        getline(in, inputSub);
        std::istringstream is5(inputSub);

        is5 >> sub.Par[0];

        getline(in, inputSub);
        std::istringstream is6(inputSub);

        for (unsigned int j=0; j<landCount; j++)
        {
            is6 >> sub.Land[j];
        }

        getline(in, inputSub);
        std::istringstream is7(inputSub);

        for (unsigned int j=1; j<5; j++)
        {
            is7 >> sub.Par[j];
        }

        getline(in, inputSub);
        std::istringstream is8(inputSub);

        for (unsigned int j=5; j<13; j++)
        {
            is8 >> sub.Par[j];
        }

        if (i == 0)
        {
            for (unsigned int j=0; j<landCount; ++j)
            {
                CellPar[j].Par[59] = sub.Par[2];
                CellPar[j].Par[60] = sub.Par[3];
                CellPar[j].Par[61] = sub.Par[4];
            }
        }

        sub.Par[2] = sub.Par[5];
        sub.Par[3] = sub.Par[6];
        sub.Par[4] = sub.Par[7];
        sub.Par[5] = sub.Par[8];
        sub.Par[6] = sub.Par[9];
        sub.Par[7] = sub.Par[10];
        sub.Par[8] = sub.Par[11];
        sub.Par[9] = sub.Par[12];

        sub.Par[10] = 0.0;
        sub.Par[11] = 0.0;
        sub.Par[12] = 0.0;
        sub.Par[13] = 1.0;
        sub.Par[14] = 1.0;
        sub.Par[15] = 0.0;
        sub.Par[16] = 0.0;
        sub.Par[17] = 0.0;
        sub.Par[18] = 0.0;

        getline(in, inputSub);
        std::istringstream is9(inputSub);

        for (unsigned int j=0; j<NUM_DEP; j++)
        {
            is9 >> sub.Dep[j];
        }

//        in >> reach;
//        CReachPar reach;
        reach.ImportBranching(in);

        ReachPar.insert(reachValueType(id, reach));
        SubPar.insert(subValueType(id, sub));

        if (newStructure)
        {
            CReach r(id, name);
            r.Coords(lastLat, lastLong, reach.Par[9], reach.Par[10]);

            if (i > 0)
            {
                r.AddInput(lastReachID);
            }

            stream.AddReach(r);

            lastLat = reach.Par[9];
            lastLong = reach.Par[10];
            lastReachID = id;
        }
    }

    if (newStructure)
    {
        stream.Name(Name);
        stream.Pars(RiverPar);
        (*structure)->AddStream(stream);
    }

	loaded = true;

    return true;
}
//---------------------------------------------------------------------------
bool CParSet::Import1v11(std::string const& FileName, CStructure** structure)
{
    if ((*structure) == NULL)
    {
        (*structure) = new CStructure;
        (*structure)->Generated(true);
    }

    CStream stream;

    ParFileName = std::string(FileName);

    std::ifstream in(FileName.c_str());

    std::string rec;
    getline(in, Name);

    std::string shortland;
    getline(in, shortland);
    CreateStringList(ShortLand, shortland.c_str());

    std::string longland;
    getline(in, longland);
    CreateStringList(LongLand, longland.c_str());

    landCount = 6;
    Init();

    for (int i=0; i<6; ++i)
    {
        std::string input;
        getline(in, input);
        std::istringstream is(input);

        for (unsigned int j=0; j<landCount; ++j)
        {
            is >> CellPar[j].Initial[i];
        }
    }

    for (unsigned int j=0; j<landCount; ++j)
    {
        CellPar[j].Initial[6] = 5000.0;
        CellPar[j].Initial[7] = 0.0;
        CellPar[j].Initial[8] = 0.0;
        CellPar[j].Initial[9] = 0.0;
    }

    // Read in start date
    unsigned int day, month, year;
    char c;

    std::string input;
    getline(in, input);
    std::istringstream is1(input);

    is1 >> day >> c >> month >> c >> year;
    StartDate = dateCl::Date(month, day, year);

    input.clear();
    getline(in, input);
    std::istringstream is2(input);

    is2 >> TimeSteps;

    for (int i=0; i<44; ++i)
    {
        std::string input;
        getline(in, input);
        std::istringstream is(input);

        for (unsigned int j=0; j<landCount; ++j)
        {
            is >> CellPar[j].Par[i];
        }
    }

    for (unsigned int j=0; j<landCount; ++j)
    {
        CellPar[j].Par[44] = 0.001;
        CellPar[j].Par[45] = 1.0;
        CellPar[j].Par[46] = 10.0;
        CellPar[j].Par[47] = 0.00002;
        CellPar[j].Par[48] = 0.33;

        CellPar[j].Par[49] = 1.0;
        CellPar[j].Par[50] = 1.0;
        CellPar[j].Par[51] = 0.0;
        CellPar[j].Par[52] = 3.0;
        CellPar[j].Par[53] = 0.0;
        CellPar[j].Par[54] = 0.0;
        CellPar[j].Par[55] = 60.0;
        CellPar[j].Par[56] = 0.0;
        CellPar[j].Par[57] = 1.0;
        CellPar[j].Par[58] = 1000.0;
        CellPar[j].Par[62] = 0.1;
        CellPar[j].Par[63] = 100.0;
    }

    for (int i=0; i<NUM_CELL_TC; ++i)
    {
        std::string input;
        getline(in, input);
        std::istringstream is(input);

        for (unsigned int j=0; j<landCount; ++j)
        {
            is >> CellPar[j].TC[i];
        }
    }

//    in >> RiverPar;
    for (int i=0; i<3; ++i)
    {
        std::string input;
        getline(in, input);
        std::istringstream is(input);

        is >> RiverPar.Initial[i];
    }

    RiverPar.Initial[3] = 1.0;

    for (int i=0; i<NUM_RIVER_PARS; ++i)
    {
        std::string input;
        getline(in, input);
        std::istringstream is(input);

        is >> RiverPar.Pars[i];
    }

    std::string input2;
    getline(in, input2);
    std::istringstream is4(input2);

    is4 >> Reaches;

    float lastLat = 0.0, lastLong = 0.0;
    std::string lastReachID;

    for (int i=0; i<Reaches; ++i)
    {
        CReachPar reach;
        reach.Import(in);

        std::ostringstream str;
        str << i+1;
        ReachPar.insert(reachValueType(str.str(), reach));

        CReach r(str.str(), reach.Name());
        r.Coords(lastLat, lastLong, 0.0, 0.0);

        if (i > 0)
        {
            r.AddInput(lastReachID);
        }

        stream.AddReach(r);

        lastLat = 0.0;
        lastLong = 0.0;
        lastReachID = str.str();
    }

    (*structure)->Name(Name);

    stream.Name(Name);
    stream.Pars(RiverPar);
    (*structure)->AddStream(stream);

    SubPar.clear();

    for (int i=0; i<Reaches; i++)
    {
        std::string input;
        getline(in, input);
        std::istringstream is(input);

        CSubPar sub(landCount);

        is >> sub.Par[0];

        for (unsigned int j=0; j<landCount; j++)
        {
            is >> sub.Land[j];
        }

        is >> sub.Par[1];

        for (unsigned int j=0; j<NUM_DEP; j++)
        {
            is >> sub.Dep[j];
        }

        for (unsigned int j=2; j<13; j++)
        {
            is >> sub.Par[j];
        }

        if (i == 0)
        {
            for (unsigned int j=0; j<landCount; ++j)
            {
                CellPar[j].Par[59] = sub.Par[2];
                CellPar[j].Par[60] = sub.Par[3];
                CellPar[j].Par[61] = sub.Par[4];
            }
        }

        sub.Par[2] = sub.Par[5];
        sub.Par[3] = sub.Par[6];
        sub.Par[4] = sub.Par[7];
        sub.Par[5] = sub.Par[8];
        sub.Par[6] = sub.Par[9];
        sub.Par[7] = sub.Par[10];
        sub.Par[8] = sub.Par[11];
        sub.Par[9] = sub.Par[12];

        sub.Par[10] = 0.0;
        sub.Par[11] = 0.0;
        sub.Par[12] = 0.0;
        sub.Par[13] = 1.0;
        sub.Par[14] = 1.0;
        sub.Par[15] = 0.0;
        sub.Par[16] = 0.0;
        sub.Par[17] = 0.0;
        sub.Par[18] = 0.0;

        std::ostringstream str;
        str << i+1;
        SubPar.insert(subValueType(str.str(), sub));
    }

	loaded = true;

    return true;
}

//---------------------------------------------------------------------------
void CParSet::CreateStringList(std::vector<std::string>& str, const char* buf)
{
    std::string tmp(buf);
    char ctmp[255];
    unsigned int index = 0;
    bool inDelimit = false;

    for (unsigned int j=0; j<tmp.size(); ++j)
    {
        if(tmp[j] != '\"' && tmp[j] != '\n')
        {
            if (tmp[j] != ',' || inDelimit)
            {
                ctmp[index] = tmp[j];
                ++index;
            }
            else
            {
                std::string land;
                land.assign(ctmp, index);
                str.push_back(land);
                index = 0;
                inDelimit = false;
            }
        }
        else
        {
            inDelimit = !inDelimit;
        }
    }

    std::string land;
    land.assign(ctmp, index);
    str.push_back(land);
}
//---------------------------------------------------------------------------
std::vector<std::string> CParSet::GetLandUseNames(bool addNum)
{
	std::vector<std::string> names;

	for (unsigned int i = 0; i < landCount; ++i)
	{
		std::ostringstream ostr;
		ostr << (i + 1) << ": ";
		std::string num = ostr.str();
		std::string name(LongLand[i].c_str());

		if (addNum) name = num + name;

		std::string str(name.c_str());
		names.push_back(str);
	}

	return names;
}
//---------------------------------------------------------------------------
std::vector<std::string> CParSet::GetReachNames(bool addNum)
{
	std::vector<std::string> names;
	unsigned int i = 0;

	for (reachIter it = ReachPar.begin(); it != ReachPar.end(); ++it)
	{
		std::ostringstream ostr;
		ostr << (i + 1) << ": ";
		std::string num = ostr.str();
		std::string name(it->second.name.c_str());

		if (addNum) name = num + name;

		std::string str(name.c_str());
		names.push_back(str);

		++i;
	}

	return names;
}
//---------------------------------------------------------------------------
void CParSet::AddToLog(std::string row)
{
    changeLog.push_back(row);
}
//---------------------------------------------------------------------------
void CParSet::writeXmlStartDate ( tinyxml2::XMLNode * node ) const
{
    StartDate.setFormat(dateCl::Date::DMY);

    std::ostringstream ostr;
    ostr << StartDate;

    tinyxml2::XMLElement * parameter = node->GetDocument()->NewElement( "Parameter" );

    parameter->SetAttribute( "name", "StartDate" );
    parameter->SetAttribute( "type", "date" );

    tinyxml2::XMLElement * defaultValue = node->GetDocument()->NewElement( "currentValue" );
    defaultValue->SetText( ostr.str().c_str() );
    parameter->InsertEndChild( defaultValue );

    node->InsertEndChild( parameter );
}
//---------------------------------------------------------------------------
void CParSet::readXmlStartDate( tinyxml2::XMLNode * node )
{
//    tinyxml2::XMLNode * parameter = node->FirstChildElement( "Parameter" );
    tinyxml2::XMLNode * value = node->FirstChildElement( "currentValue" );

    std::string startDate = value->ToElement()->GetText();

    unsigned int day, month, year;
    char c;
    std::istringstream istr( startDate );

    istr >> day >> c >> month >> c >> year;
    StartDate = dateCl::Date(month, day, year);
}
//---------------------------------------------------------------------------

