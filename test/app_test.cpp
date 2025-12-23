#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <chrono>

#include "oatpp/core/base/Environment.hpp"
#include "oatpp/web/client/HttpRequestExecutor.hpp"
#include "oatpp/network/tcp/client/ConnectionProvider.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/network/Server.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include "phonebook_test_client.hpp"
#include "controller/phonebook_controller.hpp"
#include "dto/phonebook_dto.hpp"
#include "app_component.hpp" 

class PhonebookTest : public ::testing::Test {
protected:
    void SetUp() override {
        components = std::make_unique<AppComponent>();
        mapper = components->apiObjectMapper.getObject();

        auto router = oatpp::web::server::HttpRouter::createShared();
        router->addController(std::make_shared<PhonebookController>(mapper));

        auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(router);
        auto serverProv = oatpp::network::tcp::server::ConnectionProvider::createShared({"127.0.0.1", 8001});
        
        server = std::make_unique<oatpp::network::Server>(serverProv, connectionHandler);

        serverThread = std::thread([this] {
            server->run();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        auto clientProv = oatpp::network::tcp::client::ConnectionProvider::createShared({"127.0.0.1", 8001});
        auto requestExecutor = oatpp::web::client::HttpRequestExecutor::createShared(clientProv);
        
        client = PhonebookTestClient::createShared(requestExecutor, mapper);
    }

    void TearDown() override {
        if(server) server->stop();
        if (serverThread.joinable()) serverThread.join();
        components.reset();
    }

    std::unique_ptr<AppComponent> components;
    std::shared_ptr<PhonebookTestClient> client;
    std::shared_ptr<oatpp::data::mapping::ObjectMapper> mapper;
    std::unique_ptr<oatpp::network::Server> server;
    std::thread serverThread;
};

TEST_F(PhonebookTest, FullContactLifecycle) {
    auto payload = ContactPayloadDto::createShared();
    payload->name = "Egor Gribanov";
    payload->phone_number = "+375293451267";
    payload->address = "Minsk";

    auto resCreate = client->create_contact(payload);
    ASSERT_EQ(resCreate->getStatusCode(), 200);
    auto created = resCreate->template readBodyToDto<oatpp::Object<ContactDto>>(mapper);
    v_int64 id = created->id;

    auto resGet = client->get_contact_by_id(id);
    ASSERT_EQ(resGet->getStatusCode(), 200);
    ASSERT_EQ(resGet->template readBodyToDto<oatpp::Object<ContactDto>>(mapper)->name, "Egor Gribanov");

    payload->name = "Egor Updated";
    auto resUpdate = client->update_contact(id, payload);
    ASSERT_EQ(resUpdate->getStatusCode(), 200);
    ASSERT_EQ(resUpdate->template readBodyToDto<oatpp::Object<ContactDto>>(mapper)->name, "Egor Updated");

    ASSERT_EQ(client->delete_contact(id)->getStatusCode(), 200);
    ASSERT_EQ(client->get_contact_by_id(id)->getStatusCode(), 404);
}

TEST_F(PhonebookTest, GetAllContactsCheckExactSize) {
    auto resInitial = client->get_all_contacts();
    ASSERT_EQ(resInitial->getStatusCode(), 200);
    auto listInitial = resInitial->template readBodyToDto<oatpp::List<oatpp::Object<ContactDto>>>(mapper);
    v_int64 initialSize = listInitial->size();
    ASSERT_GE(initialSize, 3);

    auto payload = ContactPayloadDto::createShared();
    payload->name = "List Tester";
    payload->phone_number = "+375291234567"; 
    payload->address = "Test Addr";
    auto resCreate = client->create_contact(payload);
    ASSERT_EQ(resCreate->getStatusCode(), 200);

    auto resAfter = client->get_all_contacts();
    auto listAfter = resAfter->template readBodyToDto<oatpp::List<oatpp::Object<ContactDto>>>(mapper);
    ASSERT_EQ(listAfter->size(), initialSize + 1);
}


TEST_F(PhonebookTest, UpdateWithIdenticalDataShouldNotFail) {
    auto payload = ContactPayloadDto::createShared();
    payload->name = "Optimization Test";
    payload->phone_number = "+375441112233";
    payload->address = "Minsk";
    
    auto created = client->create_contact(payload)->template readBodyToDto<oatpp::Object<ContactDto>>(mapper);
    v_int64 id = created->id;
    auto updatePayload = ContactPayloadDto::createShared();
    updatePayload->name = "Optimization Test";
    updatePayload->phone_number = "+375441112233";
    updatePayload->address = "Minsk";

    auto resUpdate = client->update_contact(id, updatePayload);
    ASSERT_EQ(resUpdate->getStatusCode(), 200);
    auto updated = resUpdate->template readBodyToDto<oatpp::Object<ContactDto>>(mapper);
    
    ASSERT_EQ(updated->name, "Optimization Test");
    ASSERT_EQ(updated->id, id);
}

TEST_F(PhonebookTest, StrictRegexValidation) {
    auto payload = ContactPayloadDto::createShared();
    payload->name = "Regex Tester";
    payload->address = "Test";
    payload->phone_number = "+123456789012"; 
    ASSERT_EQ(client->create_contact(payload)->getStatusCode(), 400);
    payload->phone_number = "+375991234567"; 
    ASSERT_EQ(client->create_contact(payload)->getStatusCode(), 400);
    payload->phone_number = "+37529123"; 
    ASSERT_EQ(client->create_contact(payload)->getStatusCode(), 400);
    payload->phone_number = "+37529123456789"; 
    ASSERT_EQ(client->create_contact(payload)->getStatusCode(), 400);

    payload->phone_number = "+375299999999"; 
    ASSERT_EQ(client->create_contact(payload)->getStatusCode(), 200);
}

TEST_F(PhonebookTest, RequiredFieldsValidation) {
    auto payload = ContactPayloadDto::createShared();
    payload->phone_number = "+3752912233";
    payload->address = "Address";
    ASSERT_EQ(client->create_contact(payload)->getStatusCode(), 400);
    payload = ContactPayloadDto::createShared();
    payload->name = "Name";
    payload->phone_number = "+375291112233";
    ASSERT_EQ(client->create_contact(payload)->getStatusCode(), 400);
}

TEST_F(PhonebookTest, EnhancedNotFound) {
    ASSERT_EQ(client->get_contact_by_id(99999)->getStatusCode(), 404);
    auto payload = ContactPayloadDto::createShared();
    payload->name = "Ghost";
    payload->phone_number = "+375291112233"; 
    payload->address = "Addr";
    ASSERT_EQ(client->update_contact(99999, payload)->getStatusCode(), 404);
    ASSERT_EQ(client->delete_contact(99999)->getStatusCode(), 404);
}

TEST_F(PhonebookTest, DeleteIdempotency) {
    auto payload = ContactPayloadDto::createShared();
    payload->name = "One-timer";
    payload->phone_number = "+375293343396"; 
    payload->address = "Static addr";

    auto res = client->create_contact(payload);
    ASSERT_EQ(res->getStatusCode(), 200); 
    
    auto created = res->template readBodyToDto<oatpp::Object<ContactDto>>(mapper);

    ASSERT_EQ(client->delete_contact(created->id)->getStatusCode(), 200);
    ASSERT_EQ(client->delete_contact(created->id)->getStatusCode(), 404);
}

TEST_F(PhonebookTest, ConcurrentCreation) {
    const int numThreads = 10;
    std::vector<std::thread> threads;

    for(int i = 0; i < numThreads; i++) {
        threads.push_back(std::thread([this, i] {
            auto p = ContactPayloadDto::createShared();
            p->name = "User " + std::to_string(i);
            p->phone_number = "+37529" + std::to_string(1000000 + i); 
            p->address = "Addr";
            auto res = client->create_contact(p);
            EXPECT_EQ(res->getStatusCode(), 200);
        }));
    }

    for(auto& t : threads) t.join();

    auto resAll = client->get_all_contacts();
    auto list = resAll->template readBodyToDto<oatpp::List<oatpp::Object<ContactDto>>>(mapper);
    ASSERT_GE(list->size(), 13);
}

int main(int argc, char **argv) {
    oatpp::base::Environment::init();
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    oatpp::base::Environment::destroy();
    return result;
}