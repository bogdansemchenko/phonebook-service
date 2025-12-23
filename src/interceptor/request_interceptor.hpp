#pragma once

#include "oatpp/web/server/interceptor/RequestInterceptor.hpp"
#include "oatpp/core/base/Environment.hpp"


class MyRequestInterceptor : public oatpp::web::server::interceptor::RequestInterceptor {
public:
  std::shared_ptr<OutgoingResponse> intercept(const std::shared_ptr<IncomingRequest>& request) override {
    OATPP_LOGD("API_LOG", "Incoming Request: [%s] %s", 
               request->getStartingLine().method.toString()->c_str(),
               request->getStartingLine().path.toString()->c_str());

    return nullptr; 
  }
};