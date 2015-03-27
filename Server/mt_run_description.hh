/*
 * mt_run_description.hh
 *
 *  Created on: Mar 20, 2014
 *      Author: nsoblath
 */

#ifndef MT_RUN_DESCRIPTION_HH_
#define MT_RUN_DESCRIPTION_HH_

#include "mt_param_json.hh"

namespace mantis
{

    class MANTIS_API run_description : public param_node
    {
        public:
            enum status
            {
                created = 0,
                acknowledged = 1,
                waiting = 2,
                started = 3,
                running = 4,
                stopped = 5, // run completed normally
                error = 6,
                canceled = 7, // run was started and stopped abnormally
                revoked = 8, // request will not be run
            };

        public:
            run_description();
            virtual ~run_description();

            void set_id( const unsigned id );
            unsigned get_id() const;

            void set_status( status a_status );
            status get_status() const;

            void set_client_exe( const std::string& a_exe );
            void set_client_version( const std::string& a_ver );
            void set_client_commit( const std::string& a_ver );

            void set_mantis_server_exe( const std::string& a_exe );
            void set_mantis_server_version( const std::string& a_ver );
            void set_mantis_server_commit( const std::string& a_ver );

            void set_monarch_version( const std::string& a_ver );
            void set_monarch_commit( const std::string& a_ver );

            void set_file_config( const param_node& a_config );
            void set_mantis_config( const param_node& a_config );

            void set_response( const param_node& a_response );
    };

} /* namespace Katydid */
#endif /* MT_RUN_DESCRIPTION_HH_ */
