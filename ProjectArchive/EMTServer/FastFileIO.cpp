#include "FastFileIO.h"

namespace Rain
{
	void OutputFile (std::string filename, std::string &output, bool append)
	{
		std::ofstream out;
		if (append)
			out.open (filename, std::ios_base::out | std::ios_base::binary | std::ios_base::app);
		else
			out.open (filename, std::ios_base::out | std::ios_base::binary);
		out << output;
		out.close ();
	}
}