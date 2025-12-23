#include "controller/phonebook_controller.hpp"
#include "app_component.hpp"
#include "oatpp-swagger/Controller.hpp"
#include "oatpp/network/Server.hpp"
#include "oatpp/web/server/api/Endpoint.hpp"

#include <iostream>
#include <memory>

void run() {
  AppComponent components; 

  OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);
  OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper);
  
  auto phonebookController = std::make_shared<PhonebookController>(objectMapper);
  router->addController(phonebookController);
  auto endpoints = std::make_shared<oatpp::web::server::api::Endpoints>();
  endpoints->append(phonebookController->getEndpoints());

  OATPP_COMPONENT(std::shared_ptr<oatpp::swagger::DocumentInfo>, docInfo);
  OATPP_COMPONENT(std::shared_ptr<oatpp::swagger::Resources>, resources);

  auto swaggerController = oatpp::swagger::Controller::createShared(*endpoints, docInfo, resources);
  
  router->addController(swaggerController);

  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);
  oatpp::network::Server server(connectionProvider, connectionHandler);

  std::cout << "\n---------------------------------------------------" << std::endl;
  std::cout << "Server running on port 8000" << std::endl;
  std::cout << "Swagger UI: http://localhost:8000/swagger/ui" << std::endl;
  std::cout << "---------------------------------------------------\n" << std::endl;

  server.run();
}

int main() {
  oatpp::base::Environment::init();
  run();
  oatpp::base::Environment::destroy();
  return 0;
}