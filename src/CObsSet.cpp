//---------------------------------------------------------------------------

#include <sstream>
#include <iomanip>
#include <map>
#include <cstddef>

#include "CObsSet.h"

//---------------------------------------------------------------------------

CObsSet::CObsSet()
{
    name = "Observed";

    red = 255;
    green = 0;
    blue = 0;
}
//---------------------------------------------------------------------------
CObsSet::CObsSet(std::string _name)
{
    name = _name;

    red = 255;
    green = 0;
    blue = 0;
}
//---------------------------------------------------------------------------

CObsSet::~CObsSet()
{
	for (std::size_t i = 0; i < ObsList.size(); ++i)
	{
		delete ObsList[i];
	}
}
//---------------------------------------------------------------------------
CObsSet::CObsSet(const CObsSet& original)
{
    copy(original);
}
//---------------------------------------------------------------------------
CObsSet& CObsSet::operator=(const CObsSet& original)
{
    if (&original != this)
    {
        copy(original);
    }

    return *this;
}
//---------------------------------------------------------------------------
void CObsSet::copy(const CObsSet& rhs)
{
	ObsList.clear();

	ReachCount = rhs.ReachCount;
	filename = rhs.filename;
	name = rhs.name;
	red = rhs.red;
	green = rhs.green;
	blue = rhs.blue;

	ObsList.clear();
	for (std::size_t i = 0; i < rhs.ObsList.size(); ++i)
	{
		ObsList.push_back(new CObsItem(*rhs.ObsList[i]));
	}

	reachList.clear();
    std::set<std::string>::const_iterator it;
    for (it = rhs.reachList.begin(); it != rhs.reachList.end(); ++it) {
		reachList.insert(*it);
    }
}
//---------------------------------------------------------------------------
void CObsSet::Append(const CObsSet& rhs)
{
	for (std::size_t i = 0; i < rhs.ObsList.size(); ++i)
	{
		ObsList.push_back(rhs.ObsList[i]);
	}
}
//---------------------------------------------------------------------------
void CObsSet::CalcMeans(void)
{
	MergeSources();

	for (std::size_t i = 0; i < ObsList.size(); ++i)
	{
		CObsItem* obs = reinterpret_cast<CObsItem *>(ObsList[i]);

		obs->CalcMeans();
	}
}
//---------------------------------------------------------------------------
void CObsSet::MergeTypes(void)
{
    std::vector<int> typeList = GetDistinctTypes();
	std::vector<CObsItem*> newObsList;

    for (unsigned int j=0; j<typeList.size(); ++j)
    {
        std::map<std::string, CObsItem*> merged;

		for (std::size_t i = 0; i < ObsList.size(); ++i)
		{
			CObsItem* obs = ObsList[i];

            if (obs->GetType() == typeList[j])
            {
                std::string reach = obs->GetReach();

                if (merged.find(reach) == merged.end())
                {
                    CObsItem* current = new CObsItem(*obs);
                    merged[reach] = current;
                }
                else
                {
                    merged[reach]->concatenate(*obs);
                }
            }
        }

        for (std::map<std::string, CObsItem*>::iterator it=merged.begin(); it != merged.end(); ++it)
        {
			newObsList.push_back(it->second);
		}
    }

    ObsList.clear();

	for (std::size_t i = 0; i < newObsList.size(); ++i)
	{
		ObsList.push_back(newObsList[i]);
	}

	for (std::size_t i = 0; i < newObsList.size(); ++i)
	{
		delete newObsList[i];
	}
}
//---------------------------------------------------------------------------
std::vector<int> CObsSet::GetDistinctTypes(void)
{
    std::vector<int> typeList;

	for (std::size_t i = 0; i < ObsList.size(); ++i)
	{
		CObsItem* obs = reinterpret_cast<CObsItem *>(ObsList[i]);
		bool found = false;

        for (unsigned int j=0; j<typeList.size(); ++j)
        {
            if (typeList[j] == obs->GetType()) found = true;
        }

        if (!found) typeList.push_back(obs->GetType());
    }

    return typeList;
}
//---------------------------------------------------------------------------
void CObsSet::MergeSources(void)
{
    CObsSet merged;
    CObsItem* current;
    bool stored = true;

	for (std::size_t i = 0; i < ObsList.size(); ++i)
	{
		CObsItem* obs = reinterpret_cast<CObsItem *>(ObsList[i]);

        int type = obs->GetType();
        std::string id = obs->GetReach();

        CObsItem* found = merged.find(type, id);

        if (found == NULL)
        {
            current = new CObsItem(*obs);
			merged.ObsList.push_back(current);

            stored = true;
        }
        else
        {
            current = found;
            current->concatenate(*obs);

            stored = false;
        }
    }

	if (!stored) merged.ObsList.push_back(current);

    copy(merged);
}
//---------------------------------------------------------------------------
CObsItem* CObsSet::find(int t, std::string r)
{
	for (std::size_t i = 0; i < ObsList.size(); ++i)
	{
		CObsItem* obs = reinterpret_cast<CObsItem *>(ObsList[i]);

        int type = obs->GetType();
        std::string id = obs->GetReach();

        if (type == t && id == r) return obs;
    }

    return NULL;
}
//---------------------------------------------------------------------------

bool CObsSet::LoadFromFile(const std::string FileName)
{
	std::string fname(FileName.c_str());
	filename = fname;

	std::vector<std::string> FileContents;

	std::ifstream infile(FileName.c_str());

	while (!infile.eof())
	{
		std::string row;
		getline(infile, row);
		FileContents.push_back(row);
	}

	infile.close();

    LoadFromStrings(FileContents);

	return true;
}
//---------------------------------------------------------------------------

bool CObsSet::LoadFromStrings(std::vector<std::string>& FileContents)
{
	int ItemCount;
	int FilePos;
	CObsItem *Item;

	reachList.clear();

	ItemCount = GetItemCount(FileContents);

	for (int i = 0; i < ItemCount; i++)
	{
		Item = FindNextItem(FileContents, i + 1, &FilePos);
		Item->LoadFromFile(FileContents, FilePos);
		ObsList.push_back(Item);
	}

	return true;
}
//---------------------------------------------------------------------------

bool CObsSet::ReachHasObs(std::string id)
{
	if (ObsList.size() > 0)
	{
		for (std::size_t i = 0; i < ObsList.size(); ++i)
		{
			CObsItem *obs = reinterpret_cast<CObsItem *>(ObsList[i]);
			if (obs->GetReach() == id) return true;
		}
    }

    return false;
}
//---------------------------------------------------------------------------

int CObsSet::GetItemCount(std::vector<std::string>& FileContents)
{
	int ItemCount = 0;

	for (std::size_t i = 0; i < FileContents.size(); i++)
	{
		std::string line(FileContents[i].c_str());
		std::transform(line.begin(), line.end(), line.begin(), ::toupper);

		if (line.find("REACH", 0) < line.size())
		{
			ItemCount++;
		}
	}

	return ItemCount;
}
//---------------------------------------------------------------------------

CObsItem *CObsSet::FindNextItem(std::vector<std::string>& FileContents,
	int index,
	int *FilePos)
{
	int ItemCount = 0, Type;
	std::size_t  Count = 0, Index;
	CObsItem *Item = 0;
	std::string Str, UprStr;
	std::string Reach;

	for (std::size_t i = 0; i < FileContents.size(); i++) // Looking for "Reach" header
	{
		std::string line(FileContents[i].c_str());
		std::transform(line.begin(), line.end(), line.begin(), ::toupper);

		if (line.find("REACH", 0) < line.size())
		{
			ItemCount++;

			if (ItemCount == index)
			{
				Reach = ExtractReach(FileContents[i]);
				Type = ExtractItemType(FileContents[i + 1]);

				*FilePos = i + 2;
				Str = FileContents[*FilePos];

				std::string line2(Str.c_str());
				std::transform(line2.begin(), line2.end(), line2.begin(), ::toupper);

				while (!(line2.find("REACH", 0) < line2.size()))
				{
					Count++;
					Index = *FilePos + Count;

					if (Index < FileContents.size())
					{
						Str = FileContents[Index];
						std::string line3(Str.c_str());
						line2 = line3;
						std::transform(line2.begin(), line2.end(), line2.begin(), ::toupper);
					}
					else
					{
						break;
					}
				}

				Item = new CObsItem(Count, Reach, Type);
				reachList.insert(Reach);
			}
		}
	}

	return Item;
}
//---------------------------------------------------------------------------
std::vector<std::string> CObsSet::ReachList(void)
{
    std::vector<std::string> vec;

    for (std::set<std::string>::iterator it=reachList.begin(); it!=reachList.end(); ++it)
    {
        vec.push_back(*it);
    }

    return vec;
}
//---------------------------------------------------------------------------
std::string CObsSet::ExtractReach(std::string& Str)
{
	std::string SubStr;
	std::string token("REACH");

	std::string input(Str.c_str());
	std::string inputUpper(Str.c_str());

	std::transform(inputUpper.begin(), inputUpper.end(), inputUpper.begin(), ::toupper);
	size_t pos = inputUpper.find(token, 0);

	std::string label = input.substr((pos + token.size() + 1), input.size());

	std::string delim = label.substr(label.size() - 1, 1);
	size_t start = label.find_first_of(delim, 0);

	label = label.substr(0, start);
	label.erase(label.find_last_not_of(' ') + 1);

	std::string reachID;

	if (!label.empty())
	{
		std::istringstream ss(label);
		ss >> reachID;
	}

	return reachID;
}
//---------------------------------------------------------------------------

int CObsSet::ExtractItemType(std::string& Str)
{
    std::string line(Str.c_str());
    std::transform(line.begin(), line.end(), line.begin(), ::toupper);

    if (line.find("FLOW", 0) < line.size()) return 1;
    if (line.find("NITRATE", 0) < line.size()) return 2;
    if (line.find("AMMONIUM", 0) < line.size()) return 3;
    if (line.find("DON", 0) < line.size()) return 5;
    if (line.find("VELOCITY", 0) < line.size()) return 6;
    if (line.find("WIDTH", 0) < line.size()) return 7;
    if (line.find("DEPTH", 0) < line.size()) return 8;

    if (line.find("SOIL TEMPERATURE", 0) < line.size()) return 10;
    if (line.find("SNOW DEPTH", 0) < line.size()) return 11;
    if (line.find("SNOW WATER", 0) < line.size()) return 12;

    if (line.find("SOIL WATER FLOW", 0) < line.size()) return 20;
    if (line.find("SOIL WATER NITRATE", 0) < line.size()) return 21;
    if (line.find("SOIL WATER AMMONIUM", 0) < line.size()) return 22;
    if (line.find("SOIL WATER DON", 0) < line.size()) return 24;

    if (line.find("GROUNDWATER FLOW", 0) < line.size()) return 30;
    if (line.find("GROUNDWATER NITRATE", 0) < line.size()) return 31;
    if (line.find("GROUNDWATER AMMONIUM", 0) < line.size()) return 32;
    if (line.find("GROUNDWATER DON", 0) < line.size()) return 34;

    if (line.find("DIRECT RUNOFF FLOW", 0) < line.size()) return 40;
    if (line.find("DIRECT RUNOFF NITRATE", 0) < line.size()) return 41;
    if (line.find("DIRECT RUNOFF AMMONIUM", 0) < line.size()) return 42;

    return 0;
}
//---------------------------------------------------------------------------
void CObsSet::SetColour(int r, int g, int b)
{
    red = r;
    green = g;
    blue = b;
}
//---------------------------------------------------------------------------
std::string CObsSet::GetColourAsString(void)
{
    std::ostringstream ostr;

    ostr << std::setfill('0') << std::hex;
    ostr << "$02";
    ostr << std::setw(2) << blue;
    ostr << std::setw(2) << green;
    ostr << std::setw(2) << red;

    return ostr.str();
}
//---------------------------------------------------------------------------
void CObsSet::concatenate(const CObsSet& rhs)
{
    bool found;

	for (std::size_t i = 0; i < ObsList.size(); ++i)
	{
        found = false;
		CObsItem *left = reinterpret_cast<CObsItem *>(ObsList[i]);

		for (std::size_t j = 0; j < rhs.ObsList.size(); ++j)
		{
			CObsItem *right = reinterpret_cast<CObsItem *>(rhs.ObsList[j]);

            if (left->GetReach() == right->GetReach())
            {
                found = true;
                left->concatenate(*right);
            }

            if (!found)
            {
				ObsList.push_back(right);
			}
        }
    }
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const CObsSet& o)
{
    o.write(os);
    return os;
}
//---------------------------------------------------------------------------
std::ostream& CObsSet::write(std::ostream& os) const
{
	for (std::size_t i = 0; i < ObsList.size(); ++i)
	{
		CObsItem *item = reinterpret_cast<CObsItem *>(ObsList[i]);
		os << *item;
    }

    return os;
}
//---------------------------------------------------------------------------

