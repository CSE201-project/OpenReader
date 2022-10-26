#ifndef SOLARIUM_HTTPROUTESTUDYCOMMENT_H
#define SOLARIUM_HTTPROUTESTUDYCOMMENT_H

#include <Poco/Net/HTTPRequestHandler.h>

class HTTPRouteStudyComment : public Poco::Net::HTTPRequestHandler  {
public:

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

};


#endif //SOLARIUM_HTTPROUTESTUDYCOMMENT_H