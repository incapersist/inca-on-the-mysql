#ifndef UtilsH
#define UtilsH

#include <sstream>

namespace utils
{
//---------------------------------------------------------------------------
	inline std::string trim( const std::string &str,
							 const std::string &whitespace = " \t" )
	{
		const size_t strBegin = str.find_first_not_of( whitespace );

		if (strBegin == std::string::npos)
			return ""; // no content

		const size_t strEnd = str.find_last_not_of( whitespace );
		const size_t strRange = strEnd - strBegin + 1;

		return str.substr(strBegin, strRange);
	}
//---------------------------------------------------------------------------
	inline void remove_extension(std::string& fname)
	{
		std::basic_string <char>::size_type pos;
		static const std::basic_string <char>::size_type npos = -1;

		pos = fname.find_last_of(".");

		if (pos != npos) fname.erase(pos, fname.size() - 1);
	}
//---------------------------------------------------------------------------
	inline void add_extension(std::string& fname, const char* ext, int i = 0)
	{
		std::ostringstream ostr;
		ostr << fname << ext;

		if (i > 0) ostr << i;

		fname = ostr.str();
	}
//---------------------------------------------------------------------------
	inline void change_extension(std::string& fname, const char* ext, int i = 0)
	{
		remove_extension( fname );
		add_extension( fname, ext, i );
	}
//---------------------------------------------------------------------------
	inline std::string& remove_path( std::string& filename)
	{
		const size_t last_slash_idx = filename.find_last_of("\\/");

		if (std::string::npos != last_slash_idx)
		{
			filename.erase(0, last_slash_idx + 1);
		}

		return filename;
	}
//---------------------------------------------------------------------------
}
#endif