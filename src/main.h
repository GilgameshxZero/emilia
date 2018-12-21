#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "command-handler.h"

#include "http-server.h"
#include "smtp-server.h"
#include "update-server.h"

extern const std::string Rain::CRLF;

int main(int argc, char *argv[]);

namespace Emilia {
	int start(int argc, char *argv[]);
}