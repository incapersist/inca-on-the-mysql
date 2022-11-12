//---------------------------------------------------------------------------

#include <map>
#include <exception>

#include "CObsItem.h"

//---------------------------------------------------------------------------

CObsItem::CObsItem(int count, std::string reach, int type) :
    Count(count),
    Reach(reach),
    Type((ObsType)type)
{
    Date.resize(Count);
    Data.resize(Count);
}
//---------------------------------------------------------------------------
CObsItem::CObsItem()
{
}
//---------------------------------------------------------------------------
CObsItem::~CObsItem()
{
}
//---------------------------------------------------------------------------
CObsItem::CObsItem(CObsItem& original)
{
    copy(original);
}
//---------------------------------------------------------------------------
CObsItem::CObsItem(const CObsItem& original)
{
    copy(original);
}
//---------------------------------------------------------------------------
CObsItem& CObsItem::operator=(const CObsItem& original)
{
    if (&original != this)
    {
        copy(original);
    }

    return *this;
}
//---------------------------------------------------------------------------
void CObsItem::copy(const CObsItem& rhs)
{
    Count = rhs.Count;
    Reach = rhs.Reach;
    Type = rhs.Type;

	Date.clear();
	for (std::size_t i = 0; i < rhs.Date.size(); ++i)
	{
		Date.push_back(rhs.Date[i]);
	}

	Data.clear();
	for (std::size_t i = 0; i < rhs.Data.size(); ++i)
	{
		Data.push_back(rhs.Data[i]);
	}


//    Date.clear();
//    Data.clear();

//    Date.resize(Count);
//    Data.resize(Count);

//    Date = new TDateTime[Count];
//    Data = new double[Count];

//    for (int i=0; i<Count; ++i)
//    {
//        Date = rhs.Date;
//        Data = rhs.Data;
//    }
}

//---------------------------------------------------------------------------
void CObsItem::CalcMeans(void)
{
    sort();

    CObsItem means;
    means.Reach = Reach;
    means.Type = Type;

    double sum = Data[0];
    int count = 1;

    bool written = false;

    for (int i=1; i<Count; ++i)
    {
        if (Date[i] != Date[i-1])
        {
            means.Date.push_back(Date[i-1]);
            means.Data.push_back(sum / double(count));

            sum = Data[i];
            count = 1;

            written = true;
        }
        else
        {
            sum += Data[i];
            ++count;

            written = false;
        }
    }

    if (written == false || Date[Count-1] != Date[Count-2])
    {
        means.Date.push_back(Date[Count-1]);
        means.Data.push_back(sum / double(count));
    }

    means.Count = means.Date.size();
    copy(means);
}
//---------------------------------------------------------------------------
void CObsItem::sort(void)
{
    std::multimap<dateCl::Date, double> sorted;

    for (int i=0; i<Count; ++i)
    {
        sorted.insert(std::multimap<dateCl::Date, double>::value_type(Date[i], Data[i]));
    }

    Date.clear();
    Data.clear();

    for (std::multimap<dateCl::Date, double>::iterator it=sorted.begin(); it!=sorted.end(); ++it)
    {
        Date.push_back(it->first);
        Data.push_back(it->second);
    }
}
//---------------------------------------------------------------------------
void CObsItem::concatenate(const CObsItem& rhs)
{
/*    TDateTime *oldDate = new TDateTime[Count];
    double *oldData = new double[Count];

    int newCount = Count + rhs.Count;

    for (int i=0; i<Count; ++i)
    {
        oldDate[i] = Date[i];
        oldData[i] = Data[i];
    }

//    delete[] Date;
//    delete[] Data;

    Date = new TDateTime[newCount];
    Data = new double[newCount];

    for (int i=0; i<Count; ++i)
    {
        Date[i] = oldDate[i];
        Data[i] = oldData[i];
    }

    for (int i=Count; i<newCount; ++i)
    {
        Date[i] = rhs.Date[i - Count];
        Data[i] = rhs.Data[i - Count];
    }

//    delete[] oldDate;
//    delete[] oldData;
*/

    for (int i=0; i<rhs.Count; ++i)
    {
        Date.push_back(rhs.Date[i]);
        Data.push_back(rhs.Data[i]);
    }

    Count = Data.size();
}
//---------------------------------------------------------------------------

bool CObsItem::LoadFromFile(std::vector<std::string>& FileContents, int FilePos)
{
    int day, month, year;
    double dat;

    for (int i=0; i<Count; i++)
    {
        sscanf(FileContents[FilePos+i].c_str(), "%d/%d/%d %lf",
                                                &day, &month, &year, &dat);

        try
        {
            Date[i] = dateCl::Date(month, day, year);
        }
        catch(std::exception& E)
        {
            throw ObsException(i+1, Reach, Type);
        }

        Data[i] = dat;
    }

    return true;
}
//---------------------------------------------------------------------------
int CObsItem::GetCount(void)
{
    return Count;
}
//---------------------------------------------------------------------------
std::string CObsItem::GetReach(void)
{
    return Reach;
}
//---------------------------------------------------------------------------
int CObsItem::GetType(void)
{
    return (int)Type;
}
//---------------------------------------------------------------------------

std::string CObsItem::ExtractItemName(unsigned int index) const
{
    std::string name;
    
    switch(index)
    {
        case 1  :   name = "instream flow";
                    break;
        case 2  :   name = "instream nitrate";
                    break;
        case 3  :   name = "instream ammonium";
                    break;
        case 4  :   name = "instream DON";
                    break;
        case 10  :  name = "soil temperature";
                    break;
        case 11  :  name = "snow pack depth";
                    break;
        case 12  :  name = "snow pack water equivalent";
                    break;
        case 20  :  name = "soil water flow";
                    break;
        case 21  :  name = "soil water nitrate";
                    break;
        case 22  :  name = "soil water ammonium";
                    break;
        case 24  :  name = "soil water DON";
                    break;
        case 30 :   name = "groundwater flow";
                    break;
        case 31 :   name = "groundwater nitrate";
                    break;
        case 32 :   name = "groundwater ammonium";
                    break;
        case 34 :   name = "groundwater DON";
                    break;
        default :   name = "unknown";
                    break;
    }

    return name;
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const CObsItem& o)
{
    o.write(os);
    return os;
}
//---------------------------------------------------------------------------
std::ostream& CObsItem::write(std::ostream& os) const
{
    os << "Reach " << Reach << std::endl;
    os << "Type " << Type << " (" << ExtractItemName(Type) << ")" << std::endl;

    for (int i=0; i<Count; ++i)
    {
		Date[i].setFormat(dateCl::Date::DMY);
		os << Date[i] << "\t";
        os << Data[i] << std::endl;
    }

    return os;
}
//---------------------------------------------------------------------------
