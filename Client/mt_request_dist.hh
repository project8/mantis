#ifndef MT_REQUEST_DIST_HH_
#define MT_REQUEST_DIST_HH_

#include "mt_distribution.hh"
#include "request.pb.h"
#include "status.pb.h"
#include "response.pb.h"

namespace mantis
{

    class request_dist : public distribution
    {
        public:
            request_dist();
            virtual ~request_dist();

            request* get_request();
            bool push_request();
            bool pull_request();

            status* get_status();
            bool push_status();
            bool pull_status();

            response* get_response();
            bool push_response();
            bool pull_response();

        private:
            request f_request;
            status f_status;
            response f_response;
    };

}

#endif
