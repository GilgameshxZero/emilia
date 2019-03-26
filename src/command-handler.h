#pragma once
#include "rain-aeternum/rain-libraries.h"

#include "utils.h"
#include "main-param.h"
#include "main.h"
#include "deploy-client.h"

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