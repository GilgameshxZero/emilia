#include "smtp-internal.h"

namespace Emilia {
namespace SMTPServer {
int onInternalConnect(void *param) {
    Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam *>(param);
    InternalConnectionParam &icParam = *reinterpret_cast<InternalConnectionParam *>(csmdhParam.delegateParam);
    return 0;
}
int onInternalMessage(void *param) {
    Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam *>(param);
    InternalConnectionParam &icParam = *reinterpret_cast<InternalConnectionParam *>(csmdhParam.delegateParam);

    icParam.request += *csmdhParam.message;

    return 0;
}
int onInternalDisconnect(void *param) {
    Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam *>(param);
    InternalConnectionParam &icParam = *reinterpret_cast<InternalConnectionParam *>(csmdhParam.delegateParam);

    //output status of the request
    //the server will send a 220 on a line, then the actual status; ignore the first line
    std::size_t linebreak = icParam.request.find("\r\n");
    int status = -2;
    if (linebreak == std::string::npos)
        status = -1;
    else
        status = Rain::strToT<int>(icParam.request.substr(linebreak + 2, icParam.request.length()));

    if (status == 0)
        Rain::tsCout("Success: Sent email to ", icParam.toAddress, ".\r\n");
    else
        Rain::tsCout("Failure: Didn't send email to ", icParam.toAddress, ".\r\n");
    fflush(stdout);

    //set event for waiting threads
    SetEvent(icParam.hFinish);

    return 0;
}
}  // namespace SMTPServer
}  // namespace Emilia