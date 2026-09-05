#include <../rain/build/version.hpp>
#include <rain.hpp>

#include <imap.hpp>

namespace Emilia::Imap {
	Worker::Worker(
		NativeSocket nativeSocket,
		SocketInterface *interrupter) :
		SuperWorker(nativeSocket, interrupter) {
		std::cout << "IMAP Worker created." << std::endl;
	}

	Worker Server::makeWorker(
		NativeSocket nativeSocket,
		SocketInterface *interrupter) {
		return {nativeSocket, interrupter};
	}
	Server::~Server() { this->destruct(); }
}
