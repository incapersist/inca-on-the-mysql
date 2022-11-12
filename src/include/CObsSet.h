//---------------------------------------------------------------------------
#ifndef CObsSetH
#define CObsSetH
//---------------------------------------------------------------------------
#include <stdio.h>
#include <set>

#include "CObsItem.h"

class CObsSet
{
	friend std::ostream& operator<<(std::ostream&, const CObsSet&);

public:
	CObsSet();
	CObsSet(std::string _name);
	~CObsSet();
	//    CObsSet(CObsSet&);
 	CObsSet(const CObsSet&);
 	CObsSet& operator=(const CObsSet&);

	void Append(const CObsSet&);
	void concatenate(const CObsSet&);
	bool LoadFromFile(const std::string FileName);
	bool LoadFromStrings(std::vector<std::string>& FileContents);
	std::string& Filename() { return filename; }
	std::string Name(void) { return name; }
	bool ReachHasObs(std::string id);
	void CalcMeans(void);
	void MergeSources(void);
	void MergeTypes(void);
	std::vector<int> GetDistinctTypes(void);
	CObsItem* find(int t, std::string r);
	std::string GetColourAsString(void);
	void SetColour(int, int, int);
	std::vector<std::string> ReachList(void);

	std::vector<CObsItem*> ObsList;

private:
	int GetItemCount(std::vector<std::string>& FileContents);
	CObsItem *FindNextItem(std::vector<std::string>& FileContents,
		int index,
		int *FilePos);
	std::string ExtractReach(std::string& Str);
	int ExtractItemType(std::string& Str);
 	void copy(const CObsSet&);
	std::ostream& write(std::ostream& os) const;

	int ReachCount;
	std::string filename, name;
	int red, green, blue;
	std::set<std::string> reachList;
};

#endif
