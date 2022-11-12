#include "Logger.h"
#include <iostream>

Logger& Logger::GetInstance()
{
	static Logger instance;
	return instance;
}

void Logger::log(const std::string& message, bool isLineEnd)
{
	if (isDebug_)
	{
		std::cout << message;

		if (isLineEnd)
		{
			std::cout << std::endl;
		}
	}
}