/*
The HTTP service is build around the concept of *storyworlds*. Storyworlds allow
for dynamic adjustment of dependent endpoints based on user input.

Endpoints are categorized into four types:

1. API: Storyworld-independent endpoints meant for automated access.
2. User-facing: URLs designed to be visited by humans. Experience is dependent
on the selected storyworld.
3. Specific: Endpoints which can only be resolved in either one or multiple
storyworlds. Specific endpoints only exist as paths with a `/{storyworld}/`
prefix.
4. Shared: Statics accessible under `/static`, not overridden by the current
storyworld.

Request targets are resolved as follows:

1. If the target beings with `/api`, it is an API endpoint.
2. If the target is one of the user-facing endpoints, we will undergo the
storyworld resolution process (SRP). Once the storyworld has been determined, it
will be routed to `/{storyworld}/index.html`. The front-end will then redirect
to the right user-facing component from there.
3. A target of `/{storyworld}{path}` will route to the static at
`/static/{storyworld}{path}` if it exists, and `/static{path}` if not.
4. Other targets of `{path}` will route directoy to `/static{path}` if it
exists, or return a 404.

User-facing endpoints:

1. `/`: Landing page.
2. `/snapshots/{snapshot}`: Essays, projects, or notes.
3. `/storyworlds`: Storyworld selector, which saves into the
`storyworld-selected` cookie.

The SRP picks a storyworld in the following priority:

1. `storyworld-selected` cookie.
2. `storyworld-defaulted` cookie.
3. Failure.

API endpoints:
* `/api/ping`: Returns 200 immediately.
* `/api/status`: Human-readable status page (TODO: integrate this into
storyworlds).

Storyworlds:
* `erlija-past` (default light).
* `reflections-on-blackfeather` (default dark).
*/
#pragma once

#include "smtp.hpp"

#include <rain.hpp>

namespace Emilia::Http {
	class Server;

	class Worker : public Rain::Networking::Http::Worker<
									 Rain::Networking::Http::Request,
									 Rain::Networking::Http::Response,
									 1 << 10,
									 1 << 10,
									 60000,
									 60000,
									 Rain::Networking::Ipv6FamilyInterface,
									 Rain::Networking::StreamTypeInterface,
									 Rain::Networking::TcpProtocolInterface,
									 Rain::Networking::NoLingerSocketOption> {
		private:
		using SuperWorker = Rain::Networking::Http::Worker<
			Rain::Networking::Http::Request,
			Rain::Networking::Http::Response,
			1 << 10,
			1 << 10,
			60000,
			60000,
			Rain::Networking::Ipv6FamilyInterface,
			Rain::Networking::StreamTypeInterface,
			Rain::Networking::TcpProtocolInterface,
			Rain::Networking::NoLingerSocketOption>;

		public:
		Worker(NativeSocket, SocketInterface *, Server &);

		using SuperWorker::send;
		using SuperWorker::recv;
		virtual void send(Response &) override;
		virtual Request &recv(Request &) override;

		private:
		// Server state.
		Server &server;

		virtual std::vector<RequestFilter> const &filters() override;

		ResponseAction getApiPing(Request &, std::smatch const &);
		ResponseAction getApiStatus(Request &, std::smatch const &);
		ResponseAction getApiOutboxJson(Request &, std::smatch const &);

		// Responds with the storyworld-resolved or shared index.html.
		ResponseAction getUserFacing(Request &, std::smatch const &);

		// Storyworld-specific or shared static.
		ResponseAction getStoryworldStatic(Request &, std::smatch const &);

		// General agnostic or dependent endpoints.
		ResponseAction getSharedStatic(Request &, std::smatch const &);

		// Returns false if and only if Authorization is valid.
		bool maybeRejectAuthorization(Request &);

		// Implements the SRP. Returns an empty string on failure.
		std::string resolveStoryworld(Request &);

		// Resolves a storyworld + (storyworld-unincluded) target into a filesystem
		// static path, or 404. Prefers storyworld-specific files
		std::optional<std::filesystem::path> resolveStoryworldPath(
			std::string const &,
			std::string const &);

		// Make a response from a static file at a path. Assumes path is resolved
		// and valid.
		ResponseAction getStatic(std::filesystem::path const &);
	};

	class Server : public Rain::Networking::Http::Server<
									 Worker,
									 Rain::Networking::Ipv6FamilyInterface,
									 Rain::Networking::StreamTypeInterface,
									 Rain::Networking::TcpProtocolInterface,
									 Rain::Networking::DualStackSocketOption,
									 Rain::Networking::NoLingerSocketOption> {
		// Allow state access.
		friend Worker;

		private:
		using SuperServer = Rain::Networking::Http::Server<
			Worker,
			Rain::Networking::Ipv6FamilyInterface,
			Rain::Networking::StreamTypeInterface,
			Rain::Networking::TcpProtocolInterface,
			Rain::Networking::DualStackSocketOption,
			Rain::Networking::NoLingerSocketOption>;

		static std::string const STATIC_ROOT, HTTP_USERNAME;

		// State.
		std::string const &httpPassword;
		std::tm const &processBegin;
		std::unique_ptr<Emilia::Smtp::Server> &smtpServer;
		std::atomic_bool const &echo;

		public:
		Server(
			Host const &,
			std::string const &,
			std::tm const &,
			std::unique_ptr<Emilia::Smtp::Server> &,
			std::atomic_bool const &);
		virtual ~Server();

		private:
		virtual Worker makeWorker(NativeSocket, SocketInterface *) override;
	};
}
