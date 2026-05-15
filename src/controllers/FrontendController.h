#pragma once

#include <drogon/HttpController.h>

class FrontendController : public drogon::HttpController<FrontendController>
{
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(FrontendController::index, "/", drogon::Get);
    ADD_METHOD_TO(FrontendController::asset, "/assets/{1}", drogon::Get);
    METHOD_LIST_END

    void index(const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&cb);

    void asset(const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&cb,
               std::string path);
};
