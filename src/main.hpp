#pragma once
#include "../rain/cpp/rain.hpp"

#include "command-handler.hpp"
#include "constants.hpp"
#include "utils.hpp"
#include "main-param.hpp"
#include "http-server.hpp"
#include "smtp-server.hpp"
#include "deploy-server.hpp"

int main(int argc, char *argv[]);

namespace Emilia {
	int start(int argc, char *argv[]);
}