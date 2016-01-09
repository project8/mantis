/*
 * mt_run_client.hh
 *
 *  Created on: Mar 7, 2014
 *      Author: nsoblath
 */

#ifndef MT_RUN_CLIENT_HH_
#define MT_RUN_CLIENT_HH_

//#include "mt_callable.hh"

//#include "mt_atomic.hh"
#include "mt_amqp.hh"
#include "mt_constants.hh"

#include "param.hh"

using scarab::param_node;

namespace mantis
{
    class msg_request;

    // run_client was formerly used in a separate thread, hence the previous use of the callable base class

    class MANTIS_API run_client// : public callable
    {
        public:
            run_client( const param_node& a_node, const std::string& a_exchange, amqp_channel_ptr a_channel );
            ~run_client();

            void execute();
            //void cancel();

            int get_return();

        private:
            msg_request* create_run_request( const std::string& a_routing_key );
            msg_request* create_get_request( const std::string& a_routing_key );
            msg_request* create_set_request( const std::string& a_routing_key );
            msg_request* create_cmd_request( const std::string& a_routing_key );

            //param_node* create_sender_info() const;

            param_node f_config;
            std::string f_exchange;
            amqp_channel_ptr f_channel;
            //atomic_bool f_canceled;
            int f_return;
    };

} /* namespace mantis */

#endif /* MT_RUN_CLIENT_HH_ */
