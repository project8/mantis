/*
 * mt_run_client.hh
 *
 *  Created on: Mar 7, 2014
 *      Author: nsoblath
 */

#ifndef MT_RUN_CLIENT_HH_
#define MT_RUN_CLIENT_HH_

//#include "mt_callable.hh"

#include "mt_api.hh"

#include "param.hh"

#include "message.hh"

namespace mantis
{
    using scarab::param_node;
    using dripline::request_ptr_t;

    // run_client was formerly used in a separate thread, hence the previous use of the callable base class

    class MANTIS_API run_client// : public callable
    {
        public:
            run_client( const param_node& a_node );
            ~run_client();

            void execute();
            //void cancel();

            int get_return();

        private:
            request_ptr_t create_run_request( const std::string& a_routing_key );
            request_ptr_t create_get_request( const std::string& a_routing_key );
            request_ptr_t create_set_request( const std::string& a_routing_key );
            request_ptr_t create_cmd_request( const std::string& a_routing_key );

            //param_node* create_sender_info() const;

            param_node f_config;
            //atomic_bool f_canceled;
            int f_return;
    };

} /* namespace mantis */

#endif /* MT_RUN_CLIENT_HH_ */
