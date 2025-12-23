#pragma once

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/web/protocol/http/Http.hpp"
#include <regex>

#include OATPP_CODEGEN_BEGIN(DTO)

class ContactBaseDto : public oatpp::DTO {
  DTO_INIT(ContactBaseDto, DTO)

  DTO_FIELD(String, name);
  DTO_FIELD(String, phone_number, "phoneNumber");
  DTO_FIELD(String, address);

  void validate() {
    if (!name || name->empty()) {
      throw oatpp::web::protocol::http::HttpError(oatpp::web::protocol::http::Status::CODE_400, "Name is required");
    }
    if (name->size() > 50) {
      throw oatpp::web::protocol::http::HttpError(oatpp::web::protocol::http::Status::CODE_400, "Name is too long (max 50)");
    }
    if (!address || address->empty()) {
      throw oatpp::web::protocol::http::HttpError(oatpp::web::protocol::http::Status::CODE_400, "Address is required");
    }
    
    if (!phone_number || phone_number->empty()) {
      throw oatpp::web::protocol::http::HttpError(oatpp::web::protocol::http::Status::CODE_400, "Phone number is required");
    }

    std::regex phone_regex(R"(^\+375(29|25|44|33|17)[0-9]{7}$)");
    if (!std::regex_match(phone_number->c_str(), phone_regex)) {
      throw oatpp::web::protocol::http::HttpError(oatpp::web::protocol::http::Status::CODE_400, 
        "Invalid phone format. Required: +375XXXXXXXXX (Codes: 29, 25, 44, 33, 17)");
    }
  }
};

class ContactPayloadDto : public ContactBaseDto {
  DTO_INIT(ContactPayloadDto, ContactBaseDto)
};

class ContactDto : public ContactBaseDto {
  DTO_INIT(ContactDto, ContactBaseDto)
  DTO_FIELD(Int64, id);
};

class StatusDto : public oatpp::DTO {
  DTO_INIT(StatusDto, DTO)
  DTO_FIELD(String, status);
  DTO_FIELD(Int32, code);
  DTO_FIELD(String, message);
};

#include OATPP_CODEGEN_END(DTO)