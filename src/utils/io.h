#pragma once

#include <string>
#include <cstring>

namespace io {
	/// <summary>
	/// Get the file contents in a string
	/// </summary>
	/// <param name="path">Absolute path of the file</param>
	/// <returns>The contents of the file as a string</returns>
	std::string get_file_contents(const char* path);
}
