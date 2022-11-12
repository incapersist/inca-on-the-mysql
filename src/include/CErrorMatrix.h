//---------------------------------------------------------------------------
#ifndef CErrorMatrixH
#define CErrorMatrixH
//---------------------------------------------------------------------------

#include <vector>
#include <string>

class CErrorMatrix
{
    friend std::istream& operator>>(std::istream&, CErrorMatrix&);
    friend std::ostream& operator<<(std::ostream&, const CErrorMatrix&);

    friend CErrorMatrix operator+(const CErrorMatrix&, const CErrorMatrix&);
    friend CErrorMatrix operator-(const CErrorMatrix&, const CErrorMatrix&);
    friend CErrorMatrix operator*(const CErrorMatrix&, const CErrorMatrix&);
    friend bool operator<(const CErrorMatrix&, const CErrorMatrix&);
    friend bool operator>(const CErrorMatrix&, const CErrorMatrix&);

    friend CErrorMatrix operator/(const CErrorMatrix&, const float);
    friend CErrorMatrix operator*(const CErrorMatrix&, const float);

public:
    float& operator() (unsigned int col);
    float operator() (unsigned int col) const;
    float& operator() (unsigned int row, unsigned int col);
    float operator() (unsigned int row, unsigned int col) const;

    CErrorMatrix(unsigned int numRows, unsigned int numCols, int width = 12);
    CErrorMatrix();
    CErrorMatrix(CErrorMatrix&);
    CErrorMatrix(const CErrorMatrix&);
    CErrorMatrix& operator=(const CErrorMatrix&);
    ~CErrorMatrix();

    bool LooseGreaterThan(const CErrorMatrix&);
    void FillFromRow(const unsigned int);

    void Init(unsigned int numRows, unsigned int numCols, double initVal = -999.0);
    const bool isMissingValue(unsigned int i, unsigned int j) const;
    void sqRoot(void);
    std::vector<unsigned int> GetNonNullIndexes(void);

    int setw(void) { return writeWidth; }
    void setw(int w) { writeWidth = w; }

    std::string Name(void) { return name; }
    const std::string Name(void) const { return name; }
    void Name(const std::string& n) { name = n; }
    const unsigned int Cols(void) const { return cols; }
    const unsigned int Rows(void) const { return rows; }
    void WriteSize(bool w) { writeSize = w; }
    void Labels(std::vector<std::string> l) { labels = l; }

private:

    float* errors;
    unsigned int rows, cols;
    int writeWidth;
    std::string name;
    bool writeSize;
    std::vector<std::string> labels;
};

#endif
