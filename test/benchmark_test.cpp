#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <iostream>
#include <iomanip>

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

class BenchmarkTest : public ::testing::Test {
protected:
    void SetUp() override {
        components = std::make_unique<AppComponent>();
        mapper = components->apiObjectMapper.getObject();
        auto router = oatpp::web::server::HttpRouter::createShared();
        router->addController(std::make_shared<PhonebookController>(mapper));
        auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(router);
        auto serverProv = oatpp::network::tcp::server::ConnectionProvider::createShared({"127.0.0.1", 8002});
        
        server = std::make_unique<oatpp::network::Server>(serverProv, connectionHandler);
        serverThread = std::thread([this] {
            server->run();
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        auto clientProv = oatpp::network::tcp::client::ConnectionProvider::createShared({"127.0.0.1", 8002});
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


TEST_F(BenchmarkTest, FullCrudThroughput) {
    const int numThreads = 100;           
    const int operationsPerThread = 500; 
    const int totalRequests = numThreads * operationsPerThread * 4;

    std::atomic<long> successCount{0};
    std::atomic<long> failCount{0};
    
    std::vector<std::thread> threads;
    
    std::cout << "\n===========================================================" << std::endl;
    std::cout << "[ BENCHMARK ] Setup: " << numThreads << " threads, " 
              << operationsPerThread << " cycles per thread." << std::endl;
    std::cout << "[ BENCHMARK ] Total HTTP Requests expected: " << totalRequests << std::endl;
    std::cout << "===========================================================\n" << std::endl;

    auto startTime = std::chrono::high_resolution_clock::now();

    for(int i = 0; i < numThreads; i++) {
        threads.push_back(std::thread([this, i, operationsPerThread, &successCount, &failCount] {
            for(int j = 0; j < operationsPerThread; j++) {
                std::string uniqueSuffix = std::to_string(i) + "_" + std::to_string(j);
                
                auto payload = ContactPayloadDto::createShared();
                payload->name = "BenchUser " + uniqueSuffix;
                payload->phone_number = "+37529" + std::to_string(1000000 + (i*1000) + j); 
                payload->address = "Benchmark St";

                auto resCreate = client->create_contact(payload);
                if(resCreate->getStatusCode() == 200) {
                    successCount++;
                    auto created = resCreate->template readBodyToDto<oatpp::Object<ContactDto>>(mapper);
                    v_int64 id = created->id;

                    auto resGet = client->get_contact_by_id(id);
                    if(resGet->getStatusCode() == 200) successCount++;
                    else failCount++;

                    payload->address = "Updated Addr";
                    auto resUpdate = client->update_contact(id, payload);
                    if(resUpdate->getStatusCode() == 200) successCount++;
                    else failCount++;

                    auto resDel = client->delete_contact(id);
                    if(resDel->getStatusCode() == 200) successCount++;
                    else failCount++;

                } else {
                    failCount++; 
                }
            }
        }));
    }

    for(auto& t : threads) t.join();

    auto endTime = std::chrono::high_resolution_clock::now();
    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    double seconds = durationMs / 1000.0;
    double rps = totalRequests / seconds;

    std::cout << "\n================ [ RESULTS ] ==============================" << std::endl;
    std::cout << " Time elapsed: " << std::fixed << std::setprecision(3) << seconds << " sec" << std::endl;
    std::cout << " Successful Req: " << successCount << std::endl;
    std::cout << " Failed Req:     " << failCount << std::endl;
    std::cout << " ----------------------------------------------------------" << std::endl;
    std::cout << " TOTAL RPS (Requests/sec): " << (int)rps << std::endl;
    std::cout << "===========================================================\n" << std::endl;
    ASSERT_EQ(failCount, 0);
}

int main(int argc, char **argv) {
    oatpp::base::Environment::init();
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    oatpp::base::Environment::destroy();
    return result;
}