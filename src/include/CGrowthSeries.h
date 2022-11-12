//---------------------------------------------------------------------------
#ifndef CGrowthSeriesH
#define CGrowthSeriesH
//---------------------------------------------------------------------------
#include <string>
#include <fstream>
#include <map>

#include "CGrowthSeriesItem.h"

typedef std::map<std::string, CGrowthSeriesItem> growType;

typedef growType::iterator growIter;
typedef growType::const_iterator growConstIter;

class CGrowthSeries
{
	//friend std::ofstream& operator<<(std::ofstream& out, CGrowthSeries& g);

public:
	CGrowthSeries() {}
    CGrowthSeries(unsigned int _land);

	void Init(int _land);
	bool LoadFromFile(const char* FileName);
    std::string& Filename() { return filename; }
  //  std::ofstream& serialize(std::ofstream&);

//    std::vector<CGrowthSeriesItem> Growth;
    growType Growth;

private:
    std::string filename;
    unsigned int landCount;
};

#endif
