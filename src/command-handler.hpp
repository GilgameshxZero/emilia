#pragma once
#include "../rain/src/rain.hpp"

#include "utils.hpp"
#include "main-param.hpp"
#include "main.hpp"
#include "deploy-client.hpp"

namespace Emilia {
	namespace CommandHandler {
		//handlers should return nonzero to immediately terminate program
		typedef int(*CommandHandler)(MainParam &);

		int exit(MainParam &mp);
		int restart(MainParam &mp);
		int server(MainParam &mp);
		int connect(MainParam &mp);
		int disconnect(MainParam &mp);
		int project(MainParam &mp);
		int sync(MainParam &mp);
	}
}