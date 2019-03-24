#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "main-param.h"
#include "project-utils.h"
#include "main.h"

#include "deploy-client.h"

namespace Emilia {
	namespace CommandHandler {
		//handlers should return nonzero to immediately terminate program
		typedef int(*CommandHandler)(MainParam &);

		int Exit(MainParam &mp);
		int Restart(MainParam &mp);
		int Server(MainParam &mp);
		int Connect(MainParam &mp);
		int Disconnect(MainParam &mp);
		int Project(MainParam &mp);
		int Sync(MainParam &mp);
	}
}