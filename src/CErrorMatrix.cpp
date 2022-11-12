#include "CErrorMatrix.h"

#include <ostream>
#include <istream>
#include <iomanip>
#include <cassert>
#include <math.h>

//---------------------------------------------------------------------------
CErrorMatrix::CErrorMatrix(unsigned int numRows, unsigned int numCols, int width)
    : rows (numRows)
    , cols (numCols)
    , writeWidth (width)
{
    errors = NULL;
    Init(rows, cols);
}
//---------------------------------------------------------------------------
CErrorMatrix::CErrorMatrix()
{
    errors = NULL;
    Init(1, 1);
    writeWidth = 20;
}
//---------------------------------------------------------------------------
CErrorMatrix::CErrorMatrix(CErrorMatrix& original)
{
    errors = NULL;
    cols = original.cols;
    rows = original.rows;
    writeWidth = original.writeWidth;
    name = original.name;
    writeSize = original.writeSize;
    labels = original.labels;

    Init(rows, cols);

    for (unsigned int i=0; i<(rows * cols); ++i)
    {
        errors[i] = original.errors[i];
    }
}
//---------------------------------------------------------------------------
CErrorMatrix::CErrorMatrix(const CErrorMatrix& original)
{
    errors = NULL;
    cols = original.cols;
    rows = original.rows;
    writeWidth = original.writeWidth;
    name = original.name;
    writeSize = original.writeSize;
    labels = original.labels;

    Init(rows, cols);

    for (unsigned int i=0; i<(rows * cols); ++i)
    {
        errors[i] = original.errors[i];
    }
}
//---------------------------------------------------------------------------
CErrorMatrix& CErrorMatrix::operator=(const CErrorMatrix& original)
{
    if (&original != this)
    {
        cols = original.cols;
        rows = original.rows;
        writeWidth = original.writeWidth;
        name = original.name;
        writeSize = original.writeSize;
        labels = original.labels;

        Init(rows, cols);

        for (unsigned int i=0; i<(rows * cols); ++i)
        {
            errors[i] = original.errors[i];
        }
    }

    return *this;
}
//---------------------------------------------------------------------------
void CErrorMatrix::Init(unsigned int numRows, unsigned int numCols, double initVal)
{
    cols = numCols;
    rows = numRows;

    delete errors;
    errors = new float[rows * cols];

    for (unsigned int i=0; i<(rows * cols); ++i)
    {
        errors[i] = initVal;
    }

    writeSize = false;
}
//---------------------------------------------------------------------------
CErrorMatrix::~CErrorMatrix()
{
    delete errors;
}
//---------------------------------------------------------------------------
float& CErrorMatrix::operator() (unsigned int colIndex)
{
    return errors[colIndex];
}
//---------------------------------------------------------------------------
float CErrorMatrix::operator() (unsigned int colIndex) const
{
    return errors[colIndex];
}
//---------------------------------------------------------------------------
float& CErrorMatrix::operator() (unsigned int rowIndex, unsigned int colIndex)
{
    return errors[cols * rowIndex + colIndex];
}
//---------------------------------------------------------------------------
float CErrorMatrix::operator() (unsigned int rowIndex, unsigned int colIndex) const
{
    return errors[cols * rowIndex + colIndex];
}
//---------------------------------------------------------------------------
const bool CErrorMatrix::isMissingValue(unsigned int i, unsigned int j) const
{
    float value = errors[cols * i + j];

    return (value < -998 && value > -1000);
}
//---------------------------------------------------------------------------
CErrorMatrix operator+(const CErrorMatrix& lhs, const CErrorMatrix& rhs)
{
    CErrorMatrix err(rhs);

    for (unsigned int i=0; i<err.rows; ++i)
    {
        for (unsigned int j=0; j<err.cols; ++j)
        {
            if (!rhs.isMissingValue(i, j) && !lhs.isMissingValue(i, j))
            {
                err(i, j) = lhs(i, j) + rhs(i, j);
            }
            else
            {
                err(i, j) = -999.0;
            }
        }
    }

    return err;
}
//---------------------------------------------------------------------------
CErrorMatrix operator-(const CErrorMatrix& lhs, const CErrorMatrix& rhs)
{
    CErrorMatrix err(lhs);

    for (unsigned int i=0; i<err.rows; ++i)
    {
        for (unsigned int j=0; j<err.cols; ++j)
        {
            if (!rhs.isMissingValue(i, j) && !lhs.isMissingValue(i, j))
            {
                err(i, j) = lhs(i, j) - rhs(i, j);
            }
            else
            {
                err(i, j) = -999.0;
            }
        }
    }

    return err;
}
//---------------------------------------------------------------------------
CErrorMatrix operator*(const CErrorMatrix& lhs, const CErrorMatrix& rhs)
{
    CErrorMatrix err(rhs);

    for (unsigned int i=0; i<err.rows; ++i)
    {
        for (unsigned int j=0; j<err.cols; ++j)
        {
            if (!rhs.isMissingValue(i, j) && !lhs.isMissingValue(i, j))
            {
                err(i, j) = lhs(i, j) * rhs(i, j);
            }
            else
            {
                err(i, j) = -999.0;
            }
        }
    }

    return err;
}
//---------------------------------------------------------------------------
bool operator<(const CErrorMatrix& lhs, const CErrorMatrix& rhs)
{
    for (unsigned int i=0; i<lhs.rows; ++i)
    {
        for (unsigned int j=0; j<lhs.cols; ++j)
        {
            if (!lhs.isMissingValue(i, j) && !rhs.isMissingValue(i, j))
            {
                if (lhs(i, j) >= rhs(i, j)) return false;
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool operator>(const CErrorMatrix& lhs, const CErrorMatrix& rhs)
{
    if (lhs.rows == rhs.rows)
    {
        for (unsigned int i=0; i<rhs.rows; ++i)
        {
            for (unsigned int j=0; j<rhs.cols; ++j)
            {
                if (!lhs.isMissingValue(i, j) && !rhs.isMissingValue(i, j))
                {
                    if (lhs(i, j) <= rhs(i, j)) return false;
                }
            }
        }

        return true;
    }
    else
    {
        for (unsigned int i=0; i<lhs.rows; ++i)
        {
            for (unsigned int j=0; j<lhs.cols; ++j)
            {
                if (!lhs.isMissingValue(i, j) && !rhs.isMissingValue(0, j))
                {
                    if (lhs(i, j) <= rhs(0, j)) return false;
                }
            }
        }

        return true;
    }
}
//---------------------------------------------------------------------------
bool CErrorMatrix::LooseGreaterThan(const CErrorMatrix& e)
{
    for (unsigned int i=0; i<rows; ++i)
    {
        for (unsigned int j=0; j<cols; ++j)
        {
            if (!isMissingValue(i, j) && !e.isMissingValue(i, j))
            {
                if (errors[cols * i + j] > e.errors[cols * i + j]) return true;
            }
        }
    }

    return false;
}
//---------------------------------------------------------------------------
void CErrorMatrix::FillFromRow(const unsigned int row)
{
    for (unsigned int i=0; i<rows; ++i)
    {
        for (unsigned int j=0; j<cols; ++j)
        {
            errors[cols * i + j] = errors[cols * row + j];
        }
    }
}
//---------------------------------------------------------------------------
CErrorMatrix operator/(const CErrorMatrix& lhs, const float div)
{
    CErrorMatrix err(lhs);

    for (unsigned int i=0; i<err.rows; ++i)
    {
        for (unsigned int j=0; j<err.cols; ++j)
        {
            if (!err.isMissingValue(i, j)) err(i, j) = lhs(i, j) / div;
        }
    }

    return err;
}
//---------------------------------------------------------------------------
CErrorMatrix operator*(const CErrorMatrix& lhs, const float mult)
{
    CErrorMatrix err(lhs);

    for (unsigned int i=0; i<err.rows; ++i)
    {
        for (unsigned int j=0; j<err.cols; ++j)
        {
            if (!err.isMissingValue(i, j)) err(i, j) = lhs(i, j) * mult;
        }
    }

    return err;
}
//---------------------------------------------------------------------------
void CErrorMatrix::sqRoot(void)
{
    for (unsigned int i=0; i<rows; ++i)
    {
        for (unsigned int j=0; j<cols; ++j)
        {
            if (!isMissingValue(i, j))
            {
                errors[cols * i + j] = sqrt(errors[cols * i + j]);
            }
        }
    }
}
//---------------------------------------------------------------------------
std::vector<unsigned int> CErrorMatrix::GetNonNullIndexes(void)
{
    std::vector<unsigned int> indexes;

    for (unsigned int col=0; col<cols; ++col)
    {
        unsigned int row = 0;
        bool found = false;

        while (!found && row < rows)
        {
            if (errors[cols * row + col] > -998.0)
            {
                indexes.push_back(col);
                found = true;
            }

            ++row;
        }
    }

    return indexes;
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const CErrorMatrix& e)
{
    if (e.writeSize)
    {
        os << e.rows << std::endl;
        os << e.cols << std::endl;
    }

    for (unsigned int i=0; i<e.rows; ++i)
    {
        if (e.labels.size() == e.rows)
        {
            os << std::setw(e.writeWidth) << e.labels[i];
        }
        else
        {
            os << std::setw(e.writeWidth) << (i + 1);
        }

        for (unsigned int j=0; j<e.cols; ++j)
        {
            os << std::setw(e.writeWidth) << e.errors[e.cols * i + j];
        }

        os << std::endl;
    }

    return os;
}
//---------------------------------------------------------------------------
std::istream& operator>>(std::istream& is, CErrorMatrix& e)
{
    std::string reach;

    if (e.writeSize)
    {
        unsigned int rows, cols;

        is >> rows;
        is >> cols;

        assert (rows == e.rows);
        assert (cols == e.cols);
    }

    for (unsigned int i=0; i<e.rows; ++i)
    {
        is >> reach;

        for (unsigned int j=0; j<e.cols; ++j)
        {
            is >> e.errors[e.cols * i + j];
        }
    }

    return is;
}
//---------------------------------------------------------------------------

