#pragma once

#include "iphonebook_repository.hpp"
#include <unordered_map>
#include <mutex>

class PhonebookRepository : public IPhonebookRepository {
private:
    std::unordered_map<v_int64, oatpp::Object<ContactDto>> database_;
    v_int64 id_counter_ = 0;
    std::mutex m_mutex;

public:
    PhonebookRepository() {
        addTestData("Nikita", "+375291112233", "Minsk, Belarus");
        addTestData("Artur", "+375447778899", "Gomel, Belarus");
        addTestData("Kristina", "+375251234567", "Mogilev, Belarus");
    }

    oatpp::Object<ContactDto> save(const oatpp::Object<ContactDto>& entry) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!entry->id || entry->id == 0) {
            entry->id = ++id_counter_;
        }
        database_[entry->id] = entry;
        return entry;
    }

    oatpp::Object<ContactDto> get_by_id(v_int64 id) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = database_.find(id);
        if (it != database_.end()) return it->second;
        return nullptr;
    }

    oatpp::List<oatpp::Object<ContactDto>> get_all() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto list = oatpp::List<oatpp::Object<ContactDto>>::createShared();
        for (const auto& pair : database_) {
            list->push_back(pair.second);
        }
        return list;
    }

    bool remove(v_int64 id) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        return database_.erase(id) > 0;
    }

private:
    void addTestData(const char* name, const char* phone, const char* address) {
        auto dto = ContactDto::createShared();
        dto->name = name;
        dto->phone_number = phone;
        dto->address = address;
        dto->id = ++id_counter_;
        database_[dto->id] = dto;
    }
};