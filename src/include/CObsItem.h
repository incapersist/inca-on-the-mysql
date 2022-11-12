#ifndef CObsItemH
#define CObsItemH
//---------------------------------------------------------------------------

#include <stdio.h>
#include <algorithm>
#include <fstream>
#include <vector>
#include "DATECL.h"

class CObsItem
{
    friend std::ostream& operator<<(std::ostream&, const CObsItem&);

public:
    CObsItem();
    CObsItem(int count, std::string reach, int type);
    ~CObsItem();
    CObsItem(CObsItem&);
    CObsItem(const CObsItem&);
    CObsItem& operator=(const CObsItem&);

    void concatenate(const CObsItem&);
    bool LoadFromFile(std::vector<std::string>& FileContents, int FilePos);
    int GetCount(void);
    std::string GetReach(void);
    int GetType(void);
    void CalcMeans(void);

//    TDateTime *Date;
//    double *Data;
    std::vector<dateCl::Date> Date;
    std::vector<double> Data;

private:
    void sort(void);
    std::string ExtractItemName(unsigned int) const;
    std::ostream& write(std::ostream& os) const;

	int Count;
	std::string Reach;
	enum ObsType {Flow = 1, NO3, NH4, TEMP, S_DEPTH, S_WATER,
                    SW_FLOW, SW_NO3, SW_NH4, GW_FLOW, GW_NO3, GW_NH4} Type;

    void copy(const CObsItem&);
};

class ObsException
{
    int line, type;
    std::string reach;

public:
    ObsException(const int i_line,
                    const std::string i_reach,
                    const int i_type)
                                        :   line(i_line),
											type(i_type),
											reach(i_reach)
                                            { }

    const int Line() const { return line; }
    const std::string Reach() const { return reach; }
    const int Type() const { return type; }
};

#endif
