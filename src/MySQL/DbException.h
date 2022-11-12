#pragma once
#include <mysqlx/xdevapi.h>
#include <string>
#include "ExitCodes.h"
#include "Logger.h"

class DbException : public mysqlx::Error
{
public:
	DbException(const mysqlx::Error& err, const ExitCode& code);
	~DbException();

	inline ExitCode getCode() const { return code_; }
private:
	ExitCode code_;
	Logger& logger_;	/// Logging object
};

