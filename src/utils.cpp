#include "utils.h"

namespace Emilia {
	void initProjectDir(std::string dir) {
		Rain::createDirRec(dir + ".emilia\\");

		//create default configuration and empty index
		std::ofstream out(dir + ".emilia\\index.idx");
		out.close();
		struct _utimbuf ut;
		ut.actime = ut.modtime = 0;
		_utime((dir + ".emilia\\index.idx").c_str(), &ut);

		out.open(dir + ".emilia\\config.ini");
		out << DEFAULT_CONFIGURATION;
		out.close();
	}

	void initMainParam(std::string project, MainParam &mp) {
		//configuration, logging, and command line arguments
		mp.project = project;
		mp.config = new Rain::Configuration(project + ".emilia/config.ini");
		Rain::Configuration &config = *mp.config;

		Rain::createDirRec(project + config["log-root"].s());
		mp.cerrRedirect = Rain::redirectCerrFile(project + config["log-root"].s() + config["log-error"].s(), true);
		mp.hFMemLeak = Rain::logMemoryLeaks(project + config["log-root"].s() + config["log-memory"].s());
		mp.logGeneral = new Rain::LogStream();
		mp.logHTTP = new Rain::LogStream();
		mp.logSMTP = new Rain::LogStream();
		mp.logDeploy = new Rain::LogStream();
		mp.logGeneral->setFileDst(project + config["log-root"].s() + config["log-general"].s(), true);
		mp.logGeneral->setStdHandleSrc(STD_OUTPUT_HANDLE, true);
		mp.logHTTP->setFileDst(project + config["log-root"].s() + config["log-http"].s(), true);
		mp.logSMTP->setFileDst(project + config["log-root"].s() + config["log-smtp"].s(), true);
		mp.logDeploy->setFileDst(project + config["log-root"].s() + config["log-deploy"].s(), true);

		//http setup
		mp.httpCCP.project = mp.project;
		mp.httpCCP.config = &config;
		mp.httpCCP.logHTTP = mp.logHTTP;
		mp.httpSM.setRecvBufLen(config["emilia-buffer"].i());
		mp.httpSM.setEventHandlers(HTTPServer::onConnect, HTTPServer::onMessage, HTTPServer::onDisconnect, &mp.httpCCP);

		//smtp setup
		mp.smtpCCP.project = mp.project;
		mp.smtpCCP.config = &config;
		mp.smtpCCP.logSMTP = mp.logSMTP;
		mp.smtpSM.setRecvBufLen(config["emilia-buffer"].i());
		mp.smtpSM.setEventHandlers(SMTPServer::onConnect, SMTPServer::onMessage, SMTPServer::onDisconnect, &mp.smtpCCP);

		//deploy setup
		mp.deployCCP.project = mp.project;
		mp.deployCCP.config = &config;
		mp.deployCCP.logDeploy = mp.logDeploy;
		mp.deployCCP.httpSM = &mp.httpSM;
		mp.deployCCP.smtpSM = &mp.smtpSM;
		mp.deploySM.setRecvBufLen(config["emilia-buffer"].i());
		mp.deploySM.setEventHandlers(DeployServer::onConnect, DeployServer::onMessage, DeployServer::onDisconnect, &mp.deployCCP);
		DWORD updateServerPort = config["deploy-port"].i();
		if (!mp.deploySM.setServerListen(updateServerPort, updateServerPort)) {
			Rain::tsCout("Deploy server listening on port ", mp.deploySM.getListeningPort(), ".", Rain::CRLF);
		} else {
			Rain::errorAndCout(GetLastError(), "Could not setup deploy server listening.");
		}
	}

	void freeMainParam(MainParam &mp) {
		mp.project.clear();
		delete mp.config;

		std::cerr.rdbuf(mp.cerrRedirect.first);
		delete mp.cerrRedirect.second;
		CloseHandle(mp.hFMemLeak);
		mp.logGeneral->setStdHandleSrc(STD_OUTPUT_HANDLE, false);
		delete mp.logGeneral;
		delete mp.logHTTP;
		delete mp.logSMTP;
		delete mp.logDeploy;
	}

	void startServers(MainParam &mp, unsigned long long which) {
		if (which & 1) {
			if (!mp.httpSM.setServerListen(80, 80)) {
				Rain::tsCout("HTTP server listening on port ", mp.httpSM.getListeningPort(), ".", Rain::CRLF);
			} else {
				DWORD error = GetLastError();
				Rain::errorAndCout(error, "Could not setup HTTP server listening.");
			}
		}
		if (which & 2) {
			if (!mp.smtpSM.setServerListen(25, 25)) {
				Rain::tsCout("SMTP server listening on port ", mp.smtpSM.getListeningPort(), ".", Rain::CRLF);
			} else {
				DWORD error = GetLastError();
				Rain::errorAndCout(error, "Could not setup SMTP server listening.");
			}
		}
	}

	void prepRestart(std::string project, Rain::ServerManager *httpSM, Rain::ServerManager *smtpSM, std::string copySrc) {
		//when restarting, keep track of the project and currently running servers
		unsigned long long defaultServers = 0;
		if (httpSM->getListeningPort() != -1) {
			defaultServers += 1;
		}
		if (smtpSM->getListeningPort() != -1) {
			defaultServers += 2;
		}

		std::string tmpFile = "restart-script.bat";
		std::ofstream script(tmpFile);
		script << RESTART_SHELL_SCRIPT;
		script.close();

		std::string deployScript = Rain::pathToAbsolute(tmpFile),
			serverPath = "\"" + Rain::pathToAbsolute(Rain::getExePath()) + "\"";
		ShellExecute(NULL, "open", deployScript.c_str(),
			(serverPath + " " + (copySrc.size() == 0 ? serverPath : copySrc) + " " + serverPath + " \"\" \"" + project + " \" " + Rain::tToStr(defaultServers)).c_str(),
			Rain::getPathDir(serverPath).c_str(), SW_SHOWDEFAULT);
	}

	std::string getVersionStr() {
		static const std::string VERSION_RETRIEVE_ERROR = "Error: Could not retrieve version string.";
		std::string exe = Rain::getExePath();

		DWORD dummy;
		DWORD dwSize = GetFileVersionInfoSize(exe.c_str(), &dummy);
		if (dwSize == 0) {
			return VERSION_RETRIEVE_ERROR;
		}

		std::vector<BYTE> data(dwSize);
		if (!GetFileVersionInfo(exe.c_str(), NULL, dwSize, &data[0])) {
			return VERSION_RETRIEVE_ERROR;
		}

		LPVOID pvProductVersion = NULL;
		unsigned int iProductVersionLen = 0;
		if (!VerQueryValue(&data[0], _T("\\StringFileInfo\\040904E4\\ProductVersion"), &pvProductVersion, &iProductVersionLen)) {
			return VERSION_RETRIEVE_ERROR;
		}

		return std::string(reinterpret_cast<LPCSTR>(pvProductVersion), iProductVersionLen - 1);
	}

	void injectExitCommand() {
		static const std::string COMMAND = "exit\r";
		INPUT_RECORD ir[5];
		for (int a = 0; a < COMMAND.size(); a++) {
			ir[a].EventType = KEY_EVENT;
			ir[a].Event.KeyEvent.bKeyDown = TRUE;
			ir[a].Event.KeyEvent.dwControlKeyState = 0;
			ir[a].Event.KeyEvent.uChar.UnicodeChar = COMMAND[a];
			ir[a].Event.KeyEvent.wRepeatCount = 1;
			ir[a].Event.KeyEvent.wVirtualKeyCode = COMMAND[a] - 'a' + 'A';
			ir[a].Event.KeyEvent.wVirtualScanCode = MapVirtualKey(ir[a].Event.KeyEvent.wVirtualKeyCode, MAPVK_VK_TO_VSC);
		}
		ir[4].Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
		ir[4].Event.KeyEvent.wVirtualScanCode = MapVirtualKey(VK_RETURN, MAPVK_VK_TO_VSC);

		HANDLE hConIn = GetStdHandle(STD_INPUT_HANDLE);
		DWORD dwTmp;
		WriteConsoleInput(hConIn, ir, 5, &dwTmp);
	}
}