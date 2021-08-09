// Subclasses Rain::Networking::Smtp specializations for custom SMTP server.
#include <smtp.hpp>

#include <emilia.hpp>

#include <fstream>

namespace Emilia::Smtp {
	using namespace Rain::Literal;
	using namespace Rain::Networking::Smtp;

	Worker::Worker(
		Rain::Networking::Resolve::AddressInfo const &addressInfo,
		Rain::Networking::Socket &&socket,
		std::size_t recvBufferLen,
		std::size_t sendBufferLen,
		Duration maxRecvIdleDuration,
		Duration sendOnceTimeoutDuration,
		Mailbox const *forwardTo,
		std::string const *domain,
		std::string const *sendAsPassword,
		std::multimap<std::chrono::steady_clock::time_point, Envelope> *outbox,
		std::mutex *outboxMtx,
		std::condition_variable *outboxEv)
			: Interface(
					addressInfo,
					std::move(socket),
					recvBufferLen,
					sendBufferLen,
					maxRecvIdleDuration,
					sendOnceTimeoutDuration),
				forwardTo(*forwardTo),
				domain(*domain),
				sendAsPassword(*sendAsPassword),
				outbox(*outbox),
				outboxMtx(*outboxMtx),
				outboxEv(*outboxEv),
				authenticated(false) {}

	bool Worker::onInitialResponse() {
		*this << Response(
			StatusCode::SERVICE_READY,
			{{"emilia " STRINGIFY(EMILIA_VERSION_MAJOR) "." STRINGIFY(EMILIA_VERSION_MINOR) "." STRINGIFY(EMILIA_VERSION_REVISION) "." STRINGIFY(EMILIA_VERSION_BUILD) " / rain " STRINGIFY(
				RAIN_VERSION_MAJOR) "." STRINGIFY(RAIN_VERSION_MINOR) "." STRINGIFY(RAIN_VERSION_REVISION) "." STRINGIFY(RAIN_VERSION_BUILD) " ready"}});
		return false;
	}
	Worker::PreResponse Worker::onHelo(Request &) {
		return {StatusCode::REQUEST_COMPLETED, {this->domain, "AUTH LOGIN"}};
	}
	Worker::PreResponse Worker::onMailMailbox(Mailbox const &mailbox) {
		// If authenticated, allow from any mailbox. Otherwise, limit against
		// domain mailboxes.
		if (this->authenticated || mailbox.domain != this->domain) {
			return Interface::onMailMailbox(mailbox);
		}
		return {StatusCode::AUTHENTICATION_REQUIRED};
	}
	Worker::PreResponse Worker::onRcptMailbox(
		Rain::Networking::Smtp::Mailbox const &mailbox) {
		// If authenticated, can send to any address. Otherwise, can only send to
		// the domain.
		if (this->authenticated || mailbox.domain == this->domain) {
			return Interface::onRcptMailbox(mailbox);
		}
		return {StatusCode::REQUEST_NOT_TAKEN_MAILBOX_UNAVAILABLE_PERMANENT};
	}
	Worker::PreResponse Worker::onDataStream(std::istream &stream) {
		// This function is only called when mailFrom/rcptTo are non-empty, and
		// those are only non-empty if valid parameters have been given.
		if (this->forwardTo.name.empty() || this->forwardTo.domain.empty()) {
			return Interface::onDataStream(stream);
		}

		// Receive and save to a file, whose filename is constructed uniquely.
		std::filesystem::path baseEnvelopePath;
		while (true) {
			std::time_t timeNow = time(nullptr);
			std::tm timeData;
			Rain::Time::localtime_r(&timeNow, &timeData);
			std::stringstream envelopeNameStream;
			envelopeNameStream << "../data/emails/" << timeData.tm_year << "-"
												 << timeData.tm_mon << "-" << timeData.tm_mday << "-"
												 << timeData.tm_hour << "-" << timeData.tm_min << "-"
												 << timeData.tm_sec << "-"
												 << Rain::String::Base64::encode(this->mailFrom.value())
												 << "-" << std::rand();
			baseEnvelopePath = envelopeNameStream.str();
			if (!std::filesystem::exists(baseEnvelopePath)) {
				break;
			}
		}
		std::ofstream envelopeFile(baseEnvelopePath, std::ios::binary);
		envelopeFile << "Emilia-Mail-From: " << this->mailFrom.value() << "\r\n"
								 << "Emilia-Rcpt-To:";
		for (Mailbox const &mailbox : this->rcptTo) {
			envelopeFile << " " << mailbox;
		}
		envelopeFile << "\r\n" << stream.rdbuf();
		envelopeFile.close();

		// Push the new envelopes to the outbox for the Server to send.
		for (Mailbox const &mailbox : this->rcptTo) {
			std::multimap<std::chrono::steady_clock::time_point, Envelope>::iterator
				it;
			// Discard if sending to spamtrap.
			if (mailbox.name == "canned-pork" && mailbox.domain == this->domain) {
				continue;
			}

			{
				std::lock_guard lck(this->outboxMtx);
				it = this->outbox.emplace(
					std::chrono::steady_clock::time_point::min(), Envelope());
			}
			Envelope &envelope = it->second;
			envelope.from = this->mailFrom.value();
			envelope.to = mailbox;

			// Copy the envelope data file.
			std::filesystem::path thisEnvelopePath(
				baseEnvelopePath.string() + "-" +
				Rain::String::Base64::encode(mailbox) + ".email");
			std::filesystem::copy(baseEnvelopePath, thisEnvelopePath);
			envelope.file = thisEnvelopePath;

			// Set attempts to 0.
			envelope.cAttempts = 0;
		}

		// Trigger outbox send and delete original email file.
		this->outboxEv.notify_one();
		std::filesystem::remove(baseEnvelopePath);

		return {StatusCode::REQUEST_COMPLETED};
	}
	Worker::PreResponse Worker::onRset(Request &req) {
		// Additionally reset the authenticated field.
		this->authenticated = false;
		return Interface::onRset(req);
	}
	Worker::PreResponse Worker::onAuthLogin(
		std::string const &,
		std::string const &password) {
		// Any username is valid, but the password has to match.
		if (this->sendAsPassword == password && !this->sendAsPassword.empty()) {
			this->authenticated = true;
			return {StatusCode::AUTHENTICATION_SUCCEEDED};
		}
		return {StatusCode::AUTHENTICATION_INVALID};
	}

	void Worker::streamOutImpl(Tag<Interface>, Response &) {
		// Echo to std::cout if flagged.
		// std::cout << reinterpret_cast<void *>(this) << " " << res << std::flush;
	}
	void Worker::streamInImpl(Tag<Interface>, Request &) {
		// Echo to std::cout if flagged.
		// std::cout << reinterpret_cast<void *>(this) << " " << req << std::flush;
	}

	Server::Server(
		std::size_t maxThreads,
		Rain::Networking::Specification::ProtocolFamily pf,
		std::size_t recvBufferLen,
		std::size_t sendBufferLen,
		Duration maxRecvIdleDuration,
		Duration sendOnceTimeoutDuration,
		Rain::Networking::Smtp::Mailbox const &forwardTo,
		std::string const &domain,
		std::string const &sendAsPassword,
		std::list<std::tuple<
			std::chrono::system_clock::time_point,
			bool,
			Rain::Networking::Smtp::Mailbox,
			Rain::Networking::Smtp::Mailbox>> *mailboxActivity,
		std::mutex *mailboxActivityMtx)
			: Interface(
					maxThreads,
					// Relay worker construction arguments.
					pf,
					recvBufferLen,
					sendBufferLen,
					maxRecvIdleDuration,
					sendOnceTimeoutDuration),
				forwardTo(forwardTo),
				domain(domain),
				sendAsPassword(sendAsPassword),
				outboxClosed(false),
				mailboxActivity(*mailboxActivity),
				mailboxActivityMtx(*mailboxActivityMtx) {
		this->outboxThread = std::thread([this]() {
			// Attempt remaining envelopes in the outbox every hour or so. Envelopes
			// will be ready to be retried every hour.
			static auto const retryWait = 1h;
			while (true) {
				{
					std::unique_lock lck(this->outboxMtx);
					this->outboxEv.wait_for(lck, retryWait);
				}
				if (this->outboxClosed) {
					break;
				}

				for (auto it = this->outbox.begin(); it != this->outbox.end();) {
					// If it is still too early to retry, break.
					if (it->first + retryWait > std::chrono::steady_clock::now()) {
						break;
					}

					// If the Envelope has been retried too many times, give up.
					if (it->second.cAttempts > 24) {
						std::cerr << "Exceeded maximum retries: " << it->second.from
											<< " > " << it->second.to << "\n : " << it->second.file
											<< std::endl;
						{
							std::lock_guard lck(this->mailboxActivityMtx);
							this->mailboxActivity.emplace_back(
								std::chrono::system_clock::now(),
								false,
								it->second.from,
								it->second.to);
						}
						std::lock_guard lck(this->outboxMtx);
						this->outbox.erase(it++);
						continue;
					}

					// Attempt to send the envelope.
					static auto const sendEnvelope = [](
																						 Envelope const &envelope,
																						 std::string const &domain,
																						 Mailbox const &forwardTo) {
						Client client(
							false, Rain::Networking::Specification::ProtocolFamily::INET);
						Mailbox translatedTo =
							(envelope.to.domain == domain ? forwardTo : envelope.to);
						if (client.connectMx({translatedTo.domain, "25"})) {
							return true;
						}

						if (client.recvResponse().statusCode != StatusCode::SERVICE_READY) {
							return true;
						}
						client << Request{Command::HELO, domain};
						if (
							client.recvResponse().statusCode !=
							StatusCode::REQUEST_COMPLETED) {
							return true;
						}
						client << Request{
							Command::MAIL, "FROM:<postmaster@" + domain + ">"};
						if (
							client.recvResponse().statusCode !=
							StatusCode::REQUEST_COMPLETED) {
							return true;
						}
						client << Request{
							Command::RCPT,
							"TO:<" + static_cast<std::string>(translatedTo) + ">"};
						if (
							client.recvResponse().statusCode !=
							StatusCode::REQUEST_COMPLETED) {
							return true;
						}
						client << Request{Command::DATA};
						if (
							client.recvResponse().statusCode !=
							StatusCode::START_MAIL_INPUT) {
							return true;
						}

						std::ifstream envelopeFile(envelope.file, std::ios::binary);
						client << envelopeFile.rdbuf();
						client.flush();
						envelopeFile.close();
						if (
							client.recvResponse().statusCode !=
							StatusCode::REQUEST_COMPLETED) {
							return true;
						}

						client << Request{Command::QUIT};
						if (
							client.recvResponse().statusCode != StatusCode::SERVICE_CLOSING) {
							return true;
						}

						client.close();

						// Successfully sent, so delete the file on disk.
						std::cout << "Sent " << envelope.from << " > " << envelope.to << "."
											<< std::endl;
						std::filesystem::remove(envelope.file);
						return false;
					};

					// Remove the envelope from the outbox. If unsuccessful, re-add it
					// back with a new time, and an additional attempt.
					bool successfullySent = false;
					try {
						successfullySent =
							!sendEnvelope(it->second, this->domain, this->forwardTo);
					} catch (...) {
						// Consume exceptions.
					}

					std::lock_guard mailboxLck(this->mailboxActivityMtx);
					std::lock_guard outboxLck(this->outboxMtx);
					if (!successfullySent) {
						this->outbox.emplace(std::chrono::steady_clock::now(), it->second)
							->second.cAttempts++;
						this->mailboxActivity.emplace_back(
							std::chrono::system_clock::now(),
							false,
							it->second.from,
							it->second.to);
					} else {
						this->mailboxActivity.emplace_back(
							std::chrono::system_clock::now(),
							true,
							it->second.from,
							it->second.to);
					}
					this->outbox.erase(it++);
				}
			}
		});
	}
	Server::~Server() {
		// Close the outbox, and wait for the thread to close.
		{
			std::lock_guard lck(this->outboxMtx);
			this->outboxClosed = true;
		}
		this->outboxEv.notify_all();
		this->outboxThread.join();
	}

	std::unique_ptr<Worker> Server::workerFactory(
		std::shared_ptr<std::pair<
			Rain::Networking::Socket,
			Rain::Networking::Resolve::AddressInfo>> acceptRes) {
		return std::make_unique<Worker>(
			acceptRes->second,
			std::move(acceptRes->first),
			this->recvBufferLen,
			this->sendBufferLen,
			this->maxRecvIdleDuration,
			this->sendOnceTimeoutDuration,
			&this->forwardTo,
			&this->domain,
			&this->sendAsPassword,
			&this->outbox,
			&this->outboxMtx,
			&this->outboxEv);
	};
}
