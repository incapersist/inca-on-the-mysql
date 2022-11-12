//---------------------------------------------------------------------------
#include "CObsContainer.h"

//---------------------------------------------------------------------------
CObsSet& CObsContainer::operator[](const std::string key)
{
//    assert(obs.find(key) != obs.end());

    return obs[key];
}
//---------------------------------------------------------------------------
CObsSet& CObsContainer::operator[](int index)
{
    int i = 0;

    for (iterator it=obs.begin(); it!=obs.end(); ++it)
    {
        if (i == index) return it->second;
    }

    return obs.end()->second;
}
//---------------------------------------------------------------------------
std::vector<std::string> CObsContainer::GetKeys(void)
{
    std::vector<std::string> names;

    for (iterator it=obs.begin(); it!=obs.end(); ++it)
    {
        names.push_back(it->first);
    }

    return names;
}
//---------------------------------------------------------------------------
void CObsContainer::UpdateReachList(CObsSet& o)
{
    std::vector<std::string> vec = o.ReachList();

    for (unsigned int i=0; i<vec.size(); ++i)
    {
        reachList.insert(vec[i]);
    }
}
//---------------------------------------------------------------------------
bool CObsContainer::InReachList(std::string reach)
{
    return reachList.find(reach) != reachList.end();
}
//---------------------------------------------------------------------------
void CObsContainer::GenerateChartColours(void)
{
    srand(255);

    for (iterator it=obs.begin(); it!=obs.end(); ++it)
    {
        int red = rand() % 255;
        int green = rand() % 255;
        int blue = rand() % 255;

        it->second.SetColour(red, green, blue);
    }

    if (structure != NULL)
    {
        structure->SetDirection(CStructure::MainStemFirst);
        structure->Reset();

        do
        {
            CStream stream = structure->GetCurrentStream();

            do
            {
                std::string reachKey = stream.GetCurrentReachID();

                for (iterator it=obs.begin(); it!=obs.end(); ++it)
                {
                    if (it->second.find(15, reachKey) != NULL)
                    {
                        it->second.SetColour(255, 0, 0);
                        return;
                    }
                }
            }
            while (stream.NextReach());
        }
        while (structure->NextStream());
    }
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const CObsContainer& o)
{
	os << "Size: " << o.size() << std::endl;
	for (CObsContainer::const_iterator it = o.obs.begin(); it != o.obs.end(); ++it)
	{
		os << "Source: " << it->first << std::endl;

		if (it->second.ObsList.size() > 0) os << "NULL" << std::endl;
		else os << "Count: " << it->second.ObsList.size() << std::endl;

		os << it->second;
	}

	return os;
}
//---------------------------------------------------------------------------

