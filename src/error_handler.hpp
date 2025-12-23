#pragma once

#include "dto/phonebook_dto.hpp"
#include "oatpp/web/server/handler/ErrorHandler.hpp"
#include "oatpp/web/protocol/http/outgoing/ResponseFactory.hpp"

class ErrorHandler : public oatpp::web::server::handler::ErrorHandler {
private:
  typedef oatpp::web::protocol::http::outgoing::Response Response;
  typedef oatpp::web::protocol::http::Status Status;
  typedef oatpp::web::protocol::http::outgoing::ResponseFactory ResponseFactory;

  std::shared_ptr<oatpp::data::mapping::ObjectMapper> m_objectMapper;

public:
  ErrorHandler(const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& objectMapper)
    : m_objectMapper(objectMapper)
  {}

  std::shared_ptr<Response> handleError(const Status& status, const oatpp::String& message, const Headers& headers) override {
    auto errorDto = StatusDto::createShared();
    errorDto->status = "ERROR";
    errorDto->code = status.code;
    errorDto->message = message; 

    auto response = ResponseFactory::createResponse(status, errorDto, m_objectMapper);
    for(const auto& pair : headers.getAll()) {
      response->putHeader(pair.first.toString(), pair.second.toString());
    }
    
    return response;
  }
};