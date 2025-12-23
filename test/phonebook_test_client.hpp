#pragma once

#include "oatpp/web/client/ApiClient.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "dto/phonebook_dto.hpp"

#include OATPP_CODEGEN_BEGIN(ApiClient)

class PhonebookTestClient : public oatpp::web::client::ApiClient {
  API_CLIENT_INIT(PhonebookTestClient)
  API_CALL("GET", "/contacts", get_all_contacts)
  API_CALL("POST", "/contacts", create_contact, BODY_DTO(Object<ContactPayloadDto>, payload))
  API_CALL("PUT", "/contacts/{contact_id}", update_contact, PATH(Int64, contact_id), BODY_DTO(Object<ContactPayloadDto>, payload))
  API_CALL("DELETE", "/contacts/{contact_id}", delete_contact, PATH(Int64, contact_id))
  API_CALL("GET", "/contacts/{contact_id}", get_contact_by_id, PATH(Int64, contact_id))

};

#include OATPP_CODEGEN_END(ApiClient)