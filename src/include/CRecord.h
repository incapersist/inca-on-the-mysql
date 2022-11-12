//---------------------------------------------------------------------------

#ifndef CRecordH
#define CRecordH
//---------------------------------------------------------------------------
#include <string>
#include <istream>
#include <sstream>
#include <vector>

class CRecord
{
public:
    CRecord();
    CRecord(std::istream&);

    std::istream& read(std::istream& file);
    std::string& str(void);

    void SetComment(char ch);
    void extract(double&);
    void extract(int&);
    void extract(unsigned int&);
    void extract(unsigned short&);
    void extract(bool&);
    double extractCol(int col);
    unsigned int count(void);

private:
    std::string rec;
    char commentChar;
    std::vector<std::string> cols;

    void splitColumns(void);

};

#endif


