#pragma once
#include <string>

class Logger
{
public:
	static Logger& GetInstance();
	void log(const std::string& message, bool isLineEnd = true);
	inline void setDebug(bool isDebug) { isDebug_ = isDebug;  }

private:
	Logger() = default;
	~Logger() = default;

	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;
	Logger(Logger&&) = delete;
	Logger& operator=(Logger&&) = delete;

	bool isDebug_;
};

