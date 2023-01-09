/*
The HTTP service is built around the concept of *storyworlds*, which provide a
themed experience for users.

Endpoints are categorized into three types:

1. API: Storyworld-independent endpoints meant for automated access.
2. User-facing: URLs designed to be visited by humans. Experience is dependent
on the selected storyworld.
3. Static: Shared statics accessible under `/echidna`.

User-facing endpoints:

1. `/`: Landing page.
2. `/dashboard`: Admin dashboard.
3. `/map`: Storyworld selector, which saves into the
`storyworld-selected` cookie.
4. `/timeline`: Snapshot/tag browser.
5. `/snapshots/{snapshot}`: Snapshots may not share the same name with each
other.

The SRP in the front-end picks a storyworld in the following priority:

1. `storyworld-selected` cookie.
2. `storyworld-forced` cookie, which can be set via a query parameter on any
user-facing endpoint. This cookie only lasts for the duration of the session.
3. The user’s system light/dark preferences will select the default light/dark
storyworld.

API endpoints:
* GET `/api/ping`: Returns 200 immediately.
* GET `/api/status`: Human-readable status page.
* GET `/api/outbox.json`: Authentication required. Returns SMTP outbox status as
JSON.
* GET `/api/tags/{tag}.json`: Returns JSON of snapshots of a given tag,
sorted in ascending order by date.
* GET `/api/snapshots/{snapshot}.json`: JSON information for a single snapshot.

Storyworlds:
* `erlija-past` (default light/dark).
*/
#pragma once

#include "smtp.hpp"
#include "snapshot.hpp"

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
		ResponseAction getApiTagsJson(Request &, std::smatch const &);
		ResponseAction getApiSnapshotsJson(Request &, std::smatch const &);

		// Responds with the storyworld-resolved or shared index.html.
		ResponseAction getUserFacing(Request &, std::smatch const &);

		// General agnostic or dependent endpoints.
		ResponseAction getSharedStatic(Request &, std::smatch const &);

		// Returns false if and only if Authorization is valid.
		bool maybeRejectAuthorization(Request &);

		// Resolves a target into a filesystem static path, or 404.
		std::optional<std::filesystem::path> resolvePath(std::string const &);

		// Make a response from a static file at a path. Assumes path is resolved
		// and valid.
		ResponseAction getStaticResponse(std::filesystem::path const &);
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

		// The server maintains a list of snapshots, mapped by tag->name and
		// name->snapshot.
		std::shared_mutex snapshotsMtx;
		std::unordered_map<std::string, std::vector<std::string>> tags;
		std::unordered_map<std::string, Snapshot> snapshots;

		public:
		Server(
			Host const &,
			std::string const &,
			std::tm const &,
			std::unique_ptr<Emilia::Smtp::Server> &,
			std::atomic_bool const &);
		virtual ~Server();

		void refreshSnapshots();

		private:
		virtual Worker makeWorker(NativeSocket, SocketInterface *) override;
	};
}
