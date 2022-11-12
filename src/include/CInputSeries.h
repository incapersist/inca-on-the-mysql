//---------------------------------------------------------------------------
#ifndef CInputSeriesH
#define CInputSeriesH

#include <string>
#include "CBaseSeries.h"
#include "DATECL.h"
//---------------------------------------------------------------------------

class CInputSeries //: public CBaseSeries
{
    friend std::ostream& operator<<(std::ostream&, const CInputSeries&);

public:
	CInputSeries() {}
    CInputSeries(unsigned int _index,
                    unsigned int _item,
                    int _reaches,
                    std::string _name = "");
//    ~CInputSeries();
//    CInputSeries& operator=(const CInputSeries& rhs);
//    CInputSeries(const CInputSeries& rhs);

    bool LoadFromFile(const char* FileName);
    void Setup(unsigned int,
                unsigned int,
                std::string);
    unsigned int Count(void) { return count; }
    unsigned int Items(void) { return ItemCount; }
    unsigned int size(void) { return count; }
    std::string& Name(void) { return name; }
    void Name(std::string& n) { name = n; }
    std::string& Filename() { return filename; }
    const std::string& Filename() const { return filename; }
    void Init(double val);
    bool Missing(void) { return missing; }
    void Missing(bool isMissing) { missing = isMissing; }
    bool Loaded(void) { return loaded; }
    void Loaded(bool l) { loaded = l; }
//    void clear(void);

//    void InsertRunupPeriod(int period);
    void SetReaches(std::string);

    bool useNewHydrology() { return _useNewHydrology; }
	void useNewHydrology(bool b) { _useNewHydrology = b; }

    std::string SpatialReaches(void);

    int ReachCount;
    std::vector<std::string> Reaches;
//    float** Data;
    std::vector< std::vector<float> > Data;

    void CalcSolar(double latitude, double longitude, dateCl::Date date);
    bool LoadFromStrings(const std::vector<std::string>& fileStrings,
                                    unsigned int offset = 0);

protected:
    unsigned int count, ItemCount;
    std::string name;

private:
    double sunrise, sunset;
    std::string filename;
    bool missing, loaded;
    bool _useNewHydrology;

//    void copy(const CInputSeries& rhs);
    double tand(double);
    double SolarRad(double, double, dateCl::Date&, double&);
    bool SolarNoon(double, double, dateCl::Date&);
    double cosd(double);
    double sind(double);
    double asind(double);
    double DaysInYear(dateCl::Date&);
    std::vector<float> GetWords(const std::string& row);
    void loadOldHydrology(unsigned int cols,
                            const std::vector<std::string>& fileStrings,
                            unsigned int offset = 0);
    void loadNewHydrology(unsigned int cols,
                            const std::vector<std::string>& fileStrings,
                            unsigned int offset = 0);

};
#endif

