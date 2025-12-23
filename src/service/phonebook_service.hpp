#pragma once

#include "dto/phonebook_dto.hpp"
#include "repository/iphonebook_repository.hpp"
#include "oatpp/web/protocol/http/Http.hpp"
#include <regex> 

class PhonebookService {
private:
  std::shared_ptr<IPhonebookRepository> m_repository;
  typedef oatpp::web::protocol::http::Status Status;
  typedef oatpp::web::protocol::http::HttpError HttpError;

public:
  PhonebookService(std::shared_ptr<IPhonebookRepository> repository)
      : m_repository(repository) {}

  oatpp::Object<ContactDto> createContact(const oatpp::Object<ContactPayloadDto>& payload) {
    payload->validate(); 
    
    if (m_repository->isPhoneNumberTaken(payload->phone_number)) {
        throw oatpp::web::protocol::http::HttpError(
            oatpp::web::protocol::http::Status::CODE_409, 
            "Phone number already exists"
        );
    }

    auto newContact = ContactDto::createShared();
    newContact->id = (v_int64)0;
    newContact->name = payload->name;
    newContact->phone_number = payload->phone_number;
    newContact->address = payload->address;

    return m_repository->save(newContact);
  }

  oatpp::Object<ContactDto> updateContact(v_int64 id, const oatpp::Object<ContactPayloadDto>& payload) {
    auto existing = m_repository->get_by_id(id);
    if (!existing) {
        throw HttpError(Status::CODE_404, "Contact not found");
    }

    payload->validate(); 

    if (m_repository->isPhoneNumberTaken(payload->phone_number, id)) {
        throw oatpp::web::protocol::http::HttpError(
            oatpp::web::protocol::http::Status::CODE_409, 
            "Phone number already exists"
        );
    }

    if (existing->name == payload->name && 
        existing->phone_number == payload->phone_number && 
        existing->address == payload->address) 
    {
        return existing; 
    }

    existing->name = payload->name;
    existing->phone_number = payload->phone_number;
    existing->address = payload->address;
    
    return m_repository->save(existing);
}

  oatpp::Object<ContactDto> getContactById(v_int64 id) {
    auto contact = m_repository->get_by_id(id);
    if(!contact) {
        throw HttpError(Status::CODE_404, "Contact not found");
    }
    return contact;
  }

  oatpp::List<oatpp::Object<ContactDto>> getAllContacts() {
    return m_repository->get_all();
  }

  bool deleteContact(v_int64 id) {
    bool result = m_repository->remove(id);
    if(!result) {
        throw HttpError(Status::CODE_404, "Cannot delete: Contact not found");
    }
    return result;
  }
  
};