#pragma once



#include "./dbconnector.h"
#include "./user.h"

class AuthManager
{

public:
    AuthManager();
    ~AuthManager();

    User* Authenticate(const std::string& user, const std::string& pass);
    User* RefreshToken(const std::string& token);

private:
    DBConnector *db_;
};