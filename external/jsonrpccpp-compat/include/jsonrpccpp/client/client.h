/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    client.h
 * @date    03.01.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_CLIENT_H_
#define JSONRPC_CPP_CLIENT_H_

#include "iclientconnector.h"
#include "batchcall.h"
#include "batchresponse.h"
#include <jsonrpccpp/common/jsonparser.h>

#include <vector>
#include <map>

namespace jsonrpc
{
    class RpcProtocolClient;

    typedef enum {JSONRPC_CLIENT_V1, JSONRPC_CLIENT_V2} clientVersion_t;

    class Client
    {
        public:
            Client(IClientConnector &connector, clientVersion_t version = JSONRPC_CLIENT_V2);
            virtual ~Client();

            void        CallMethod          (const std::string &name, const Json::Value &parameter, Json::Value& result) noexcept(false);
            Json::Value CallMethod          (const std::string &name, const Json::Value &parameter) noexcept(false);

            void           CallProcedures      (const BatchCall &calls, BatchResponse &response) noexcept(false);
            BatchResponse  CallProcedures      (const BatchCall &calls) noexcept(false);

            void        CallNotification    (const std::string& name, const Json::Value& parameter) noexcept(false);

        private:
           IClientConnector  &connector;
           RpcProtocolClient *protocol;

    };

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_CLIENT_H_ */
