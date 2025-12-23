#pragma once

#include "service/phonebook_service.hpp"
#include "repository/iphonebook_repository.hpp"
#include "repository/phonebook_repository.hpp"
#include "error_handler.hpp"

#include "interceptor/request_interceptor.hpp" 

#include "oatpp-swagger/Model.hpp"
#include "oatpp-swagger/Resources.hpp"
#include "oatpp/network/Server.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/component.hpp"

#include <memory>

class AppComponent {
public:
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::DocumentInfo>, swaggerDocumentInfo)([] {
    oatpp::swagger::DocumentInfo::Builder builder;
    builder.setTitle("Phonebook API")
           .setDescription("Phonebook Service")
           .setVersion("1.0");
    return builder.build();}());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::Resources>, swaggerResources)([] {
    return oatpp::swagger::Resources::loadResources(SWAGGER_RES_PATH);}());

  OATPP_CREATE_COMPONENT(std::shared_ptr<IPhonebookRepository>, repository)([] {
    return std::make_shared<PhonebookRepository>();}());

  OATPP_CREATE_COMPONENT(std::shared_ptr<PhonebookService>, service)([] {
    OATPP_COMPONENT(std::shared_ptr<IPhonebookRepository>, repository);
    return std::make_shared<PhonebookService>(repository);}());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)([] {
    return oatpp::parser::json::mapping::ObjectMapper::createShared();}());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)([] {
    return oatpp::network::tcp::server::ConnectionProvider::createShared({"0.0.0.0", 8000, oatpp::network::Address::IP_4});}());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)([] {
    return oatpp::web::server::HttpRouter::createShared();}());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)([] {
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);
    OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper);

    auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(router);
    connectionHandler->setErrorHandler(std::make_shared<ErrorHandler>(objectMapper));
    connectionHandler->addRequestInterceptor(std::make_shared<MyRequestInterceptor>());

    return connectionHandler;
  }());
};