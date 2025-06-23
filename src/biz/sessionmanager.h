#pragma once

#include <map>
#include "./user.h"


class SessionManager
{

public:
    SessionManager();
    ~SessionManager();

    std::string CreateSession(User *user);
    User *ValidateSession(const std::string& token);

private:
    std::map<std::string, User *> active_sessions_;
};