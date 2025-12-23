#pragma once

#include "service/phonebook_service.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class PhonebookController : public oatpp::web::server::api::ApiController {
private:
  PhonebookService m_service; 
public:
  PhonebookController(const std::shared_ptr<ObjectMapper>& objectMapper)
    : oatpp::web::server::api::ApiController(objectMapper)
    , m_service(OATPP_GET_COMPONENT(std::shared_ptr<IPhonebookRepository>)) 
  {}

  ENDPOINT_INFO(getAllContacts) {
    info->summary = "Get all contacts";
    info->addResponse<List<Object<ContactDto>>>(Status::CODE_200, "application/json");
  }
  ENDPOINT("GET", "/contacts", getAllContacts) {
    return createDtoResponse(Status::CODE_200, m_service.getAllContacts());
  }

  ENDPOINT_INFO(getContactById) {
    info->summary = "Get contact by ID";
    info->addResponse<Object<ContactDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<StatusDto>>(Status::CODE_404, "application/json");
  }
  ENDPOINT("GET", "/contacts/{contactId}", getContactById, PATH(Int64, contactId)) {
    return createDtoResponse(Status::CODE_200, m_service.getContactById(contactId));
  }

  ENDPOINT_INFO(createContact) {
    info->summary = "Create new contact";
    info->addConsumes<Object<ContactPayloadDto>>("application/json"); 
    info->addResponse<Object<ContactDto>>(Status::CODE_200, "application/json"); 
    info->addResponse<Object<StatusDto>>(Status::CODE_400, "application/json");
  }
  ENDPOINT("POST", "/contacts", createContact, BODY_DTO(Object<ContactPayloadDto>, payload)) {
    return createDtoResponse(Status::CODE_200, m_service.createContact(payload));
  }

  ENDPOINT_INFO(updateContact) {
    info->summary = "Update existing contact";
    info->addConsumes<Object<ContactPayloadDto>>("application/json");
    info->addResponse<Object<ContactDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<StatusDto>>(Status::CODE_404, "application/json");
    info->addResponse<Object<StatusDto>>(Status::CODE_400, "application/json");
  }
  ENDPOINT("PUT", "/contacts/{contactId}", updateContact, PATH(Int64, contactId), BODY_DTO(Object<ContactPayloadDto>, payload)) {
    return createDtoResponse(Status::CODE_200, m_service.updateContact(contactId, payload));
  }

  ENDPOINT_INFO(deleteContact) {
    info->summary = "Delete contact";
    info->addResponse<String>(Status::CODE_200, "text/plain");
    info->addResponse<Object<StatusDto>>(Status::CODE_404, "application/json");
  }
  ENDPOINT("DELETE", "/contacts/{contactId}", deleteContact, PATH(Int64, contactId)) {
    m_service.deleteContact(contactId);
    return createResponse(Status::CODE_200, "Contact deleted successfully");
  }
};

#include OATPP_CODEGEN_END(ApiController)