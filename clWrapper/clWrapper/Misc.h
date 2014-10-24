#ifndef MISC_H
#define MISC_H

#include <string>

#pragma once
namespace Misc
{
	// Removes extra whitespace from OpenCL names for some devices/platforms
	std::string Trim(const std::string& str, const std::string& whitespace = " \t");
}
#endif