/*
The HTTP service is build around the concept of *storyworlds*. Endpoints are
categorized into four groups:

* Agnostic: API endpoints and statics living under `./static` invariant to
storyworlds.
* Dependent: Endpoints available in all storyworlds, with different styling.
* Specific: Statics which may only be available in select storyworlds.
* Resolution: The `/` endpoint, which sets a default storyworld then redirects
to the dependent endpoint `/`.

A specific endpoint may be converted into an agnostic endpoint by prepending
`{storyworld}` to the endpoint. Each request is accompanied with a storyworld
resolution process (SRP). Failing SRPs will cause specific endpoints to return
404s, and dependent endpoints to redirect to the resolution endpoint `/` before
redirecting back.

The storyworld resolution process picks a storyworld in the following priority:

1. `storyworld-selected` cookie.
2. Endpoint preference: select dependent endpoints have a storyworld preference.
3. `storyworld-preferred` cookie.
4. `storyworld-defaulted` cookie.
5. Failure.

An SRP which resolves in stage 2 will set its result in the
`storyworld-preferred` cookie from the server-side. This allows endpoints with
preference to load all their resource with from the same storyworld. Storyworlds
can be forced for a session with a URL by prepending `{storyworld}` to a
dependent endpoint.

Agnostic endpoints:
* `/api/status`: Human-readable status page.
* `/api/storyworlds/defaults/{lightness}`: Given the lightness (light or dark)
of the client, return the default storyworld for that lightness.
	* For light-theme clients: `erlija-past`.
	* For dark-theme clients: `reflections-on-blackfeather`.
* Any statics directly under `./static`.

Dependent endpoints:
* `/`.
* `/snapshots/{snapshot}`: Essays, projects, pages, or the like. Each storyworld
may implement a different path to this snapshot and display it in different
views.
* `/storyworlds`: Selector for storyworlds. Selections made here are saved into
the `storyworld-selected` cookie from the client-side.

Specific endpoints:
* Any statics (except `index.html`) directly under `./static/{storyworld}`.

Resolution endpoints:
* `/`: Upon storyworld resolution, the storyworld is saved client-side to the
`storyworld-defaulted` cookie.

Storyworlds:
* `erlija-past`.
* `reflections-on-blackfeather`.
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

		virtual std::vector<RequestFilter> filters() override;

		// Server status.
		ResponseAction requestApiStatus(Request &, std::smatch const &);

		// Themed-defaults for storyworlds.
		ResponseAction requestApiStoryworldsDefaults(
			Request &,
			std::smatch const &);

		// All dependent & resolution endpoints actually respond with some
		// `index.html` based on SRP. This allows for code reuse and smooth
		// transitions on the front-end.
		ResponseAction requestDependentOrResolution(Request &, std::smatch const &);

		// Stages 1-4 of storyworld determination prioritization. The bool is true
		// only if this storyworld was determined via preference (stage 2).
		std::pair<std::optional<std::string>, bool> resolveStoryworld(
			Request &,
			std::string const &);

		// Resolves a path + theme into a filesystem path based on precedence.
		// Any path returned is a valid file.
		std::optional<std::filesystem::path> resolveStoryworldPath(
			std::string const &,
			std::optional<std::string> const &);

		// Make a response from a static file at a path. Assumes path is resolved
		// and valid.
		ResponseAction requestStatic(std::filesystem::path const &);

		// Return an in-code HTML to set a forced storyworld and reload.
		ResponseAction requestForcedDependent(Request &, std::smatch const &);

		// General agnostic or dependent endpoints.
		ResponseAction requestSpecificOrStaticAgnostic(
			Request &,
			std::smatch const &);
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

		// List of storyworld IDs. Must be kept consistent.
		static std::vector<std::string> const storyworlds;

		// State.
		std::tm const &processBegin;
		std::unique_ptr<Emilia::Smtp::Server> &smtpServer;
		std::atomic_bool const &echo;

		public:
		Server(
			Host const &,
			std::tm const &,
			std::unique_ptr<Emilia::Smtp::Server> &,
			std::atomic_bool const &);
		virtual ~Server();

		private:
		virtual Worker makeWorker(NativeSocket, SocketInterface *) override;
	};
}
