// Subclasses Rain::Networking::Smtp specializations for custom SMTP server.
#include <smtp.hpp>

#include <emilia.hpp>

namespace Emilia::Smtp {
	Worker::Worker(
		NativeSocket nativeSocket,
		SocketInterface *interrupter,
		Server &server)
			: SuperWorker(nativeSocket, interrupter), server(server) {}
	bool Worker::onInitialResponse() {
		this->send(
			{StatusCode::SERVICE_READY,
			 {{"emilia " STRINGIFY(EMILIA_VERSION_MAJOR) "." STRINGIFY(EMILIA_VERSION_MINOR) "." STRINGIFY(EMILIA_VERSION_REVISION) "." STRINGIFY(EMILIA_VERSION_BUILD) " / rain " STRINGIFY(
				 RAIN_VERSION_MAJOR) "." STRINGIFY(RAIN_VERSION_MINOR) "." STRINGIFY(RAIN_VERSION_REVISION) "." STRINGIFY(RAIN_VERSION_BUILD)}}});
		return false;
	}
	void Worker::send(Response &res) {
		if (this->server.echo) {
			std::cout << "SMTP to " << this->peerHost() << ":\n" << res << std::endl;
		}
		SuperWorker::send(res);
	}
	Worker::Request &Worker::recv(Request &req) {
		SuperWorker::recv(req);
		if (this->server.echo) {
			std::cout << "SMTP from " << this->peerHost() << ":\n"
								<< req << std::endl;
		}
		return req;
	}
	Worker::ResponseAction Worker::onHelo(Request &) {
		return {{StatusCode::REQUEST_COMPLETED, {"gilgamesh.cc", "AUTH LOGIN"}}};
	}
	Worker::ResponseAction Worker::onEhlo(Request &) {
		return {
			{StatusCode::REQUEST_COMPLETED,
			 {"gilgamesh.cc", "AUTH LOGIN", "8BITMIME", "SMTPUTF8"}}};
	}
	Worker::ResponseAction Worker::onMailMailbox(Mailbox const &mailbox) {
		// If authenticated, allow from any mailbox. Otherwise, limit against
		// domain mailboxes.
		if (this->authenticated || mailbox.host != "gilgamesh.cc") {
			return SuperWorker::onMailMailbox(mailbox);
		}
		return {{StatusCode::AUTHENTICATION_REQUIRED}};
	}
	Worker::ResponseAction Worker::onRcptMailbox(Mailbox const &mailbox) {
		// If authenticated, can send to any address. Otherwise, can only send to
		// the domain.
		if (this->authenticated || mailbox.host == "gilgamesh.cc") {
			return SuperWorker::onRcptMailbox(mailbox);
		}
		return {{StatusCode::AUTHENTICATION_REQUIRED}};
	}
	Worker::ResponseAction Worker::onDataStream(std::istream &stream) {
		// This function is only called when mailFrom/rcptTo are non-empty, and
		// those are only non-empty if valid parameters have been given.
		if (
			this->server.smtpForward.name.empty() ||
			this->server.smtpForward.host.node.empty()) {
			return {
				{StatusCode::TRANSACTION_FAILED, {{"No forwarding configured."}}}};
		}

		// Receive and save to a file, whose filename is constructed uniquely.
		std::filesystem::path dataPath;
		while (true) {
			std::time_t timeNow = time(nullptr);
			std::tm timeData;
			Rain::Time::localtime_r(&timeNow, &timeData);
			// Base64 is not filename-safe, so we replace '/' with '_' instead.
			std::string fromB64{Rain::String::Base64::encode(this->mailFrom.value())};
			std::replace(fromB64.begin(), fromB64.end(), '/', '_');
			std::stringstream dataPathStream;
			dataPathStream << "../smtp/" << timeData.tm_year << "-" << timeData.tm_mon
										 << "-" << timeData.tm_mday << "-" << timeData.tm_hour
										 << "-" << timeData.tm_min << "-" << timeData.tm_sec << "-"
										 << fromB64 << "-" << std::rand();
			dataPath = dataPathStream.str();
			if (!std::filesystem::exists(dataPath)) {
				break;
			}
		}
		std::ofstream dataFile(dataPath, std::ios::binary);
		dataFile << "X-Emilia-Mail-From: " << this->mailFrom.value() << "\r\n"
						 << "X-Emilia-Rcpt-To:";
		for (Mailbox const &mailbox : this->rcptTo) {
			dataFile << " " << mailbox;
		}
		dataFile << "\r\n" << stream.rdbuf();
		dataFile.close();

		// Push the new envelopes to the outbox for the Server to send.
		for (Mailbox const &rcptMailbox : this->rcptTo) {
			// Envelope data is copied from the dataFile.
			std::string toB64{Rain::String::Base64::encode(rcptMailbox)};
			std::replace(toB64.begin(), toB64.end(), '/', '_');
			std::filesystem::path envelopeDataPath(
				dataPath.string() + "-" + toB64 + ".email");
			std::filesystem::copy(dataPath, envelopeDataPath);

			// Emplace new envelope.
			std::unique_lock lck(this->server.outboxMtx);
			this->server.outbox.emplace(
				Envelope::Status::PENDING,
				0,
				std::chrono::steady_clock::now(),
				this->mailFrom.value(),
				rcptMailbox,
				envelopeDataPath);
		}

		// Trigger outbox send and delete original email file.
		std::filesystem::remove(dataPath);
		this->server.outboxEv.notify_one();

		return {{StatusCode::REQUEST_COMPLETED}};
	}
	Worker::ResponseAction Worker::onRset(Request &req) {
		// Additionally reset the authenticated field.
		this->authenticated = false;
		return SuperWorker::onRset(req);
	}
	Worker::ResponseAction Worker::onAuthLogin(
		std::string const &username,
		std::string const &password) {
		if (this->server.echo) {
			std::cout << "AUTH LOGIN from " << this->peerHost() << ": " << username
								<< " : " << password << std::endl;
		}
		// Any username is valid, but the password has to match.
		if (
			!this->server.smtpPassword.empty() &&
			this->server.smtpPassword == password) {
			this->authenticated = true;
			return {{StatusCode::AUTHENTICATION_SUCCEEDED}};
		}
		return {{StatusCode::AUTHENTICATION_INVALID}};
	}

	Client::Client(
		std::vector<std::pair<std::size_t, std::string>> const &mxRecords,
		Server &server)
			: SuperClient(mxRecords, 25), server(server) {}
	void Client::send(Request &req) {
		if (this->server.echo) {
			std::cout << "SMTP to " << this->peerHost() << ":\n" << req << std::endl;
		}
		SuperClient::send(req);
	}
	Client::Response &Client::recv(Response &res) {
		SuperClient::recv(res);
		if (this->server.echo) {
			std::cout << "SMTP from " << this->peerHost() << ":\n"
								<< res << std::endl;
		}
		return res;
	}

	Server::Server(
		Host const &host,
		std::atomic_bool const &echo,
		Rain::Networking::Smtp::Mailbox const &smtpForward,
		std::string const &smtpPassword)
			: SuperServer(host),
				echo(echo),
				smtpForward(smtpForward),
				smtpPassword(smtpPassword) {
		// The sender attempts to send envelopes in the outbox.
		this->sender = std::thread([this]() {
			using namespace Rain::Literal;

			// Attempt remaining envelopes in the outbox every hour or so. Envelopes
			// will be ready to be retried every hour.
			while (!this->closed) {
				// Log any throws.
				Rain::Error::consumeThrowable(
					[this]() {
						static auto const retryWait = 4h;
						std::vector<Envelope> toAttempt;
						{
							// This lambda should only be called while we have an exclusive
							// lock on the outboxMtx.
							auto const getAttemptEnvelopes = [this, &toAttempt]() {
								// Assume we have exclusive lock. Move all envelopes we want to
								// send from the outbox to toAttempt.
								for (auto it = this->outbox.begin();
										 it != this->outbox.end();) {
									if (it->status != Envelope::Status::PENDING) {
										break;
									}
									if (it->attemptTime > std::chrono::steady_clock::now()) {
										it++;
										continue;
									}
									toAttempt.emplace_back(*it);
									it = this->outbox.erase(it);
								}
							};

							// Wait on mutex only if no pending envelopes are ready yet.
							std::unique_lock lck(this->outboxMtx);
							getAttemptEnvelopes();
							if (toAttempt.empty()) {
								this->outboxEv.wait_for(lck, retryWait);
								if (this->closed) {
									return;
								}
								getAttemptEnvelopes();
							}

							// Done with exclusive lock; we will add these envelopes back
							// later with an updated status.
						}

						// All attempted envelopes should be PENDING.
						for (auto &it : toAttempt) {
							if (it.attempt == 8) {
								std::cerr << "Exceeded maximum retries: " << it.from << " > "
													<< it.to << "\n : " << it.data << std::endl;
								it.status = Envelope::Status::FAILURE;
								continue;
							}

							// Attempt to send the envelope. Returns an empty optional, or the
							// unexpected error Response.
							auto const attemptEnvelope =
								[this](
									Envelope const &envelope) -> std::optional<Client::Response> {
								// Translate to/from. Mails are always from postmaster@node. If
								// the to mailbox is @node, then it is instead redirected to the
								// smtpForward. Otherwise, it remains the same.
								static Mailbox const from("postmaster", "gilgamesh.cc");
								Mailbox to(
									envelope.to.host == Host("gilgamesh.cc") ? this->smtpForward
																													 : envelope.to);

								Client client(Rain::Networking::getMxRecords(to.host), *this);
								{
									auto res = client.recv();
									if (res.statusCode != StatusCode::SERVICE_READY) {
										return res;
									}
								}
								client.send({Command::EHLO, "gilgamesh.cc"});
								{
									auto res = client.recv();
									if (res.statusCode != StatusCode::REQUEST_COMPLETED) {
										return res;
									}
								}
								client.send(
									{Command::MAIL,
									 "FROM:<" + static_cast<std::string>(from) + ">"});
								{
									auto res = client.recv();
									if (res.statusCode != StatusCode::REQUEST_COMPLETED) {
										return res;
									}
								}
								client.send(
									{Command::RCPT, "TO:<" + static_cast<std::string>(to) + ">"});
								{
									auto res = client.recv();
									if (res.statusCode != StatusCode::REQUEST_COMPLETED) {
										return res;
									}
								}
								client.send({Command::DATA, ""});
								{
									auto res = client.recv();
									if (res.statusCode != StatusCode::START_MAIL_INPUT) {
										return res;
									}
								}
								std::ifstream dataFile(envelope.data, std::ios::binary);
								client << dataFile.rdbuf();
								client.flush();
								{
									auto res = client.recv();
									if (res.statusCode != StatusCode::REQUEST_COMPLETED) {
										return res;
									}
								}
								client.send({Command::QUIT, ""});
								{
									auto res = client.recv();
									if (res.statusCode != StatusCode::SERVICE_CLOSING) {
										return res;
									}
								}
								Rain::Error::consumeThrowable(
									[&client]() { client.shutdown(); });
								return {};
							};

							bool sent = false;
							try {
								auto res = attemptEnvelope(it);
								if (res) {
									std::cerr << "Failed " << it.from << " > " << it.to << ".\n"
														<< res.value() << std::flush;
									sent = false;
								} else {
									std::cout << "Sent " << it.from << " > " << it.to << '.'
														<< std::endl;
									sent = true;
								}
							} catch (std::exception const &exception) {
								std::cout << "Failed " << it.from << " > " << it.to << ".\n"
													<< exception.what() << std::endl;
								sent = false;
							} catch (...) {
								std::cout << "Failed " << it.from << " > " << it.to << '.'
													<< std::endl;
								sent = false;
							}

							// Update envelope status.
							it.attemptTime = std::chrono::steady_clock::now();
							if (sent) {
								it.status = Envelope::Status::SUCCESS;
							} else {
								it.status = Envelope::Status::RETRIED;
							}
						}

						// For each attempt, insert its updated status back into the outbox.
						// None of them should be PENDING anymore. For those marked as
						// RETRIED, insert a PENDING envelope in addition. For those marked
						// as success, delete the data file.
						std::unique_lock lck(this->outboxMtx);
						for (auto const &it : toAttempt) {
							if (it.status == Envelope::Status::SUCCESS) {
								std::filesystem::remove(it.data);
							} else if (it.status == Envelope::Status::RETRIED) {
								this->outbox.emplace(
									Envelope::Status::PENDING,
									it.attempt + 1,
									std::chrono::steady_clock::now() + retryWait,
									it.from,
									it.to,
									it.data);
							}
							// No additional action needed for FAILURE envelopes.
							this->outbox.emplace(it);
						}
					},
					RAIN_ERROR_LOCATION)();
			}
		});
	}
	Server::~Server() {
		this->closed = true;
		this->outboxEv.notify_one();
		this->sender.join();
		this->destruct();
	}
	Worker Server::makeWorker(
		NativeSocket nativeSocket,
		SocketInterface *interrupter) {
		return {nativeSocket, interrupter, *this};
	}
}
