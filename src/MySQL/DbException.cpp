#include "DbException.h"
#include <sstream>

using namespace mysqlx;

DbException::DbException(const mysqlx::Error& err, const ExitCode& code) : Error(err), code_(code), logger_(Logger::GetInstance())
{
	std::stringstream str;
	str << "ERROR: " << this->what() << " (" << static_cast<int>(code_) << ")";
	logger_.log(str.str());
}

DbException::~DbException()
{
}
