/*
 * mt_acq_request.hh
 *
 *  Created on: Mar 20, 2014
 *      Author: nsoblath
 */

#ifndef MT_ACQ_REQUEST_HH_
#define MT_ACQ_REQUEST_HH_

#include "mt_param.hh"

#include <boost/uuid/uuid.hpp>

namespace mantis
{

    class MANTIS_API acq_request : public param_node
    {
        public:
            enum status
            {
                created = 0,
                acknowledged = 1,
                waiting = 2, // acquisition is in the queue
                started = 3,
                running = 4,
                stopped = 5, // acquisition completed normally
                error = 6,
                canceled = 7, // acquisition was started and stopped abnormally
                revoked = 8, // acquisition will not be performed
            };

            static std::string interpret_status( status a_status );

        public:
            acq_request( boost::uuids::uuid an_id );
            acq_request( const acq_request& orig );
            virtual ~acq_request();

            acq_request& operator=( const acq_request& rhs );

            void set_id( const boost::uuids::uuid id );
            boost::uuids::uuid get_id() const;
            std::string get_id_string() const;

            void set_status( status a_status );
            status get_status() const;

            void set_client_exe( const std::string& a_exe );
            void set_client_version( const std::string& a_ver );
            void set_client_commit( const std::string& a_commit );
            void set_client_package( const std::string& a_pkg );

            void set_mantis_server_exe( const std::string& a_exe );
            void set_mantis_server_version( const std::string& a_ver );
            void set_mantis_server_commit( const std::string& a_commit );

            void set_monarch_version( const std::string& a_ver );
            void set_monarch_commit( const std::string& a_ver );

            void set_file_config( const param_value& a_config );
            void set_description_config( const param_value& a_config );

            void set_acquisition_config( const param_node& a_config );

            void set_response( const param_node& a_response );

        private:
            boost::uuids::uuid f_id;
    };

} /* namespace mantis */
#endif /* MT_ACQ_REQUEST_HH_ */
