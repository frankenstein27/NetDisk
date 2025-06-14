#pragma once 


#include "./httpresponse.h"
#include "./httprequest.h"
#include "./user.h"

class FIleService
{
public:
    FIleService();
    ~FIleService();

    HttpResponse *UploadFile(User* user, HttpRequest* req);
    HttpResponse *DownloadFile(User *user, HttpRequest *req);
    HttpResponse *ResumeUpload(User *user, HttpRequest *req);

private:

};

FIleService::FIleService(/* args */)
{
}

FIleService::~FIleService()
{
}
