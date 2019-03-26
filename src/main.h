#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "command-handler.h"
#include "constants.h"
#include "utils.h"
#include "main-param.h"
#include "http-server.h"
#include "smtp-server.h"
#include "deploy-server.h"

int main(int argc, char *argv[]);

namespace Emilia {
	int start(int argc, char *argv[]);
}