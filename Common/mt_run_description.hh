/*
 * mt_run_description.hh
 *
 *  Created on: Mar 20, 2014
 *      Author: nsoblath
 */

#ifndef MT_RUN_DESCRIPTION_HH_
#define MT_RUN_DESCRIPTION_HH_

#include "mt_param.hh"

namespace mantis
{

    class run_description : public param_node
    {
        public:
            run_description();
            virtual ~run_description();

            void set_mantis_client_exe( const std::string& a_exe );
            void set_mantis_server_exe( const std::string& a_exe );

            void set_mantis_client_version( const std::string& a_ver );
            void set_mantis_server_version( const std::string& a_ver );
            void set_monarch_version( const std::string& a_ver );

            void set_mantis_client_commit( const std::string& a_ver );
            void set_mantis_server_commit( const std::string& a_ver );
            void set_monarch_commit( const std::string& a_ver );

            void set_description( const std::string& a_desc );

            void set_client_config( const param_node& a_config );
            void set_server_config( const param_node& a_config );
    };

} /* namespace Katydid */
#endif /* MT_RUN_DESCRIPTION_HH_ */
