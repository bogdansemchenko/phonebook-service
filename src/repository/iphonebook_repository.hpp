#pragma once

#include "dto/phonebook_dto.hpp"

class IPhonebookRepository {
public:
    virtual ~IPhonebookRepository() = default;

    virtual oatpp::Object<ContactDto> save(const oatpp::Object<ContactDto>& entry) = 0;
    virtual oatpp::Object<ContactDto> get_by_id(v_int64 id) = 0;
    virtual oatpp::List<oatpp::Object<ContactDto>> get_all() = 0;
    virtual bool remove(v_int64 id) = 0;
};