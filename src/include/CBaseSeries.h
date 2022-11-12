//---------------------------------------------------------------------------
#ifndef CBaseSeriesH
#define CBaseSeriesH

#include <string>
#include <vector>
//---------------------------------------------------------------------------

//typedef double** DataCollection;

class CBaseSeries
{
    friend std::ostream& operator<<(std::ostream&, const CBaseSeries&);

public:
    CBaseSeries();
    CBaseSeries(unsigned int _item, unsigned int _index,
                std::vector<std::string>& strs, std::string _name = "");
    CBaseSeries(unsigned int _item, unsigned int _index,
                std::string _name = "");
//    ~CBaseSeries();
//    CBaseSeries& operator=(const CBaseSeries& rhs);
//    CBaseSeries(const CBaseSeries& rhs);

    bool LoadFromFile(const char* FileName);
    void SaveToFile(const char* FileName);
//    DataCollection GetSection(unsigned int start, unsigned int period);

    std::vector<std::vector<double> > GetVector(void);

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
    bool Loaded(void) { return loaded; }
    void Loaded(bool l) { loaded = l; }
    void clear(void);

//    float** Data;
    std::vector< std::vector<float> > Data;

protected:
    unsigned int count, ItemCount;
    std::string name;

private:
    void copy(const CBaseSeries& rhs);

    std::string filename;
    std::vector<std::string> names;
    bool missing, loaded;
};
#endif
