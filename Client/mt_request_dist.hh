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
            bool push_request( int flags = 0 );
            bool pull_request( int flags = 0 );

            status* get_status();
            bool push_status( int flags = 0 );
            bool pull_status( int flags = 0 );

            response* get_response();
            bool push_response( int flags = 0 );
            bool pull_response( int flags = 0 );

        private:
            request f_request;
            status f_status;
            response f_response;
    };

}

#endif
