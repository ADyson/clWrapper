#include "Misc.h"

std::string Misc::Trim(const std::string& str, const std::string& whitespace)
{
		const int strBegin = str.find_first_not_of(whitespace);
		if (strBegin == std::string::npos)
			return ""; // no content

		const int strEnd = str.find_last_not_of(whitespace);
		const int strRange = strEnd - strBegin + 1;

		return str.substr(strBegin, strRange);
}