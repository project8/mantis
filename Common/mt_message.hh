/*
 * mt_message.hh
 *
 *  Created on: Jul 9, 2015
 *      Author: nsoblath
 */

#ifndef MT_MESSAGE_HH_
#define MT_MESSAGE_HH_

#include "mt_amqp.hh"
#include "mt_constants.hh"
#include "mt_param.hh"

#include <boost/uuid/random_generator.hpp>

#include <string>

namespace mantis
{

    //***********
    // Message
    //***********

    class MANTIS_API message
    {
        public:
            enum encoding
            {
                k_json,
                k_msgpack
            };

            struct sender_info
            {
                std::string f_package;
                std::string f_exe;
                std::string f_version;
                std::string f_commit;
                std::string f_hostname;
                std::string f_username;
            };

        protected:
            static boost::uuids::random_generator f_uuid_gen;

        public:
            message();
            virtual ~message();

            virtual bool is_request() const = 0;
            virtual bool is_reply() const = 0;
            virtual bool is_alert() const = 0;
            virtual bool is_info() const = 0;

        public:
            virtual bool do_publish( amqp_channel_ptr a_channel, const std::string& a_exchange, std::string& a_reply_consumer_tag ) = 0;

        protected:
            amqp_message_ptr create_amqp_message() const;

            virtual bool derived_modify_amqp_message( amqp_message_ptr t_amqp_msg ) const = 0;
            virtual bool derived_modify_message_body( param_node& a_node ) const = 0;

            bool encode_message_body( std::string& a_body ) const;
            std::string interpret_encoding() const;

        public:
            void set_routing_key( const std::string& a_rk );
            const std::string& get_routing_key() const;

            void set_correlation_id( const std::string& a_id );
            const std::string& get_correlation_id() const;

            void set_encoding( encoding a_enc );
            encoding get_encoding() const;

            virtual unsigned get_message_type() const = 0;

            void set_timestamp( const std::string& a_ts );
            const std::string& get_timestamp() const;

            void set_sender_package( const std::string& a_pkg );
            const std::string& get_sender_package() const;

            void set_sender_exe( const std::string& a_exe );
            const std::string& get_sender_exe() const;

            void set_sender_version( const std::string& a_vsn );
            const std::string& get_sender_version() const;

            void set_sender_commit( const std::string& a_cmt );
            const std::string& get_sender_commit() const;

            void set_sender_hostname( const std::string& a_host );
            const std::string& get_sender_hostname() const;

            void set_sender_username( const std::string& a_user );
            const std::string& get_sender_username() const;

            void set_payload( param_node* a_payload );
            const param_node& get_payload() const;
            param_node& get_payload();

        protected:
            std::string f_routing_key;
            std::string f_correlation_id;
            encoding f_encoding;

            std::string f_timestamp;
            sender_info f_sender_info;
            param_node* f_payload;

            std::string f_exe_name;
            std::string f_hostname;
            std::string f_username;

    };


    //***********
    // Request
    //***********

    class msg_reply;

    class MANTIS_API msg_request : public message
    {
        public:
            msg_request();
            virtual ~msg_request();

            static msg_request* create( param_node* a_payload, unsigned a_msg_op, const std::string& a_routing_key, message::encoding a_encoding );

            bool is_request() const;
            bool is_reply() const;
            bool is_alert() const;
            bool is_info() const;

            msg_reply* reply( unsigned a_ret_code, const std::string& a_ret_msg ) const;

        public:
            bool do_publish( amqp_channel_ptr a_channel, const std::string& a_exchange, std::string& a_reply_consumer_tag );

        private:
            bool derived_modify_amqp_message( amqp_message_ptr t_amqp_msg ) const;
            bool derived_modify_message_body( param_node& a_node ) const;

        public:
            unsigned get_message_type() const;
            static unsigned message_type();

            void set_reply_to( const std::string& a_rt );
            const std::string& get_reply_to() const;

            void set_message_op( unsigned a_op );
            unsigned get_message_op() const;

        private:
            std::string f_reply_to;

            static unsigned f_message_type;

            unsigned f_message_op;
    };


    //*********
    // Reply
    //*********

    class MANTIS_API msg_reply : public message
    {
        public:
            msg_reply();
            virtual ~msg_reply();

            static msg_reply* create( unsigned a_retcode, const std::string& a_ret_msg, param_node* a_payload, const std::string& a_routing_key, message::encoding a_encoding );

            bool is_request() const;
            bool is_reply() const;
            bool is_alert() const;
            bool is_info() const;

        public:
            bool do_publish( amqp_channel_ptr a_channel, const std::string& a_exchange, std::string& a_reply_consumer_tag );

        private:
            bool derived_modify_amqp_message( amqp_message_ptr t_amqp_msg ) const;
            bool derived_modify_message_body( param_node& a_node ) const;

        public:
            unsigned get_message_type() const;
            static unsigned message_type();

            void set_return_code( unsigned a_retcode );
            unsigned get_return_code() const;

            void set_return_message( const std::string& a_ret_msg );
            const std::string& get_return_message() const;

        private:
            static unsigned f_message_type;

            unsigned f_return_code;

            mutable std::string f_return_buffer;
    };

    //*********
    // Alert
    //*********

    class MANTIS_API msg_alert : public message
    {
        public:
            msg_alert();
            virtual ~msg_alert();

            static msg_alert* create( param_node* a_payload, const std::string& a_routing_key, message::encoding a_encoding );

            bool is_request() const;
            bool is_reply() const;
            bool is_alert() const;
            bool is_info() const;

        public:
            bool do_publish( amqp_channel_ptr a_channel, const std::string& a_exchange, std::string& a_reply_consumer_tag );

        private:
            bool derived_modify_amqp_message( amqp_message_ptr t_amqp_msg ) const;
            bool derived_modify_message_body( param_node& a_node ) const;

        public:
            unsigned get_message_type() const;
            static unsigned message_type();

        private:
            static unsigned f_message_type;
    };

    //********
    // Info
    //********

    class MANTIS_API msg_info : public message
    {
        public:
            msg_info();
            virtual ~msg_info();

            bool is_request() const;
            bool is_reply() const;
            bool is_alert() const;
            bool is_info() const;

        public:
            bool do_publish( amqp_channel_ptr a_channel, const std::string& a_exchange, std::string& a_reply_consumer_tag );

        private:
            bool derived_modify_amqp_message( amqp_message_ptr t_amqp_msg ) const;
            bool derived_modify_message_body( param_node& a_node ) const;

        public:
            unsigned get_message_type() const;
            static unsigned message_type();

        private:
            static unsigned f_message_type;
    };




    //***********
    // Message
    //***********

    inline void message::set_routing_key( const std::string& a_rk )
    {
        f_routing_key = a_rk;
        return;
    }

    inline const std::string& message::get_routing_key() const
    {
        return f_routing_key;
    }

    inline void message::set_correlation_id( const std::string& a_id )
    {
        f_correlation_id = a_id;
        return;
    }

    inline const std::string& message::get_correlation_id() const
    {
        return f_correlation_id;
    }

    inline void message::set_encoding( encoding a_enc )
    {
        f_encoding = a_enc;
        return;
    }

    inline message::encoding message::get_encoding() const
    {
        return f_encoding;
    }

    inline void message::set_timestamp( const std::string& a_ts )
    {
        f_timestamp = a_ts;
        return;
    }

    inline const std::string& message::get_timestamp() const
    {
        return f_timestamp;
    }


    inline void message::set_sender_package( const std::string& a_pkg )
    {
        f_sender_info.f_package = a_pkg;
        return;
    }

    inline const std::string& message::get_sender_package() const
    {
        return f_sender_info.f_package;
    }


    inline void message::set_sender_exe( const std::string& a_exe )
    {
        f_sender_info.f_exe = a_exe;
        return;
    }

    inline const std::string& message::get_sender_exe() const
    {
        return f_sender_info.f_exe;
    }


    inline void message::set_sender_version( const std::string& a_vsn )
    {
        f_sender_info.f_version = a_vsn;
        return;
    }

    inline const std::string& message::get_sender_version() const
    {
        return f_sender_info.f_version;
    }


    inline void message::set_sender_commit( const std::string& a_cmt )
    {
        f_sender_info.f_commit = a_cmt;
        return;
    }

    inline const std::string& message::get_sender_commit() const
    {
        return f_sender_info.f_commit;
    }


    inline void message::set_sender_hostname( const std::string& a_host )
    {
        f_sender_info.f_hostname = a_host;
        return;
    }

    inline const std::string& message::get_sender_hostname() const
    {
        return f_sender_info.f_hostname;
    }


    inline void message::set_sender_username( const std::string& a_user )
    {
        f_sender_info.f_username = a_user;
        return;
    }

    inline const std::string& message::get_sender_username() const
    {
        return f_sender_info.f_username;
    }


    inline void message::set_payload( param_node* a_payload )
    {
        delete f_payload;
        f_payload = a_payload;
        return;
    }

    inline const param_node& message::get_payload() const
    {
        return *f_payload;
    }

    inline param_node& message::get_payload()
    {
        return *f_payload;
    }

    //***********
    // Request
    //***********

    inline bool msg_request::is_request() const
    {
        return true;
    }
    inline bool msg_request::is_reply() const
    {
        return false;
    }
    inline bool msg_request::is_alert() const
    {
        return false;
    }
    inline bool msg_request::is_info() const
    {
        return false;
    }

    inline bool msg_request::derived_modify_amqp_message( amqp_message_ptr a_amqp_msg ) const
    {
        a_amqp_msg->ReplyTo( f_reply_to );
        return true;
    }

    inline bool msg_request::derived_modify_message_body( param_node& a_node ) const
    {
        a_node.add( "msgop", new param_value( f_message_op ) );
        return true;
    }

    inline msg_reply* msg_request::reply( unsigned a_ret_code, const std::string& a_ret_msg ) const
    {
        msg_reply* t_reply = new msg_reply();
        t_reply->set_return_code( a_ret_code );
        t_reply->set_return_message( a_ret_msg );
        t_reply->set_correlation_id( f_correlation_id );
        t_reply->set_routing_key( f_reply_to );
        return t_reply;
    }

    inline void msg_request::set_reply_to( const std::string& a_rt )
    {
        f_reply_to = a_rt;
        return;
    }

    inline const std::string& msg_request::get_reply_to() const
    {
        return f_reply_to;
    }

    inline void msg_request::set_message_op( unsigned a_op )
    {
        f_message_op = a_op;
        return;
    }

    inline unsigned msg_request::get_message_op() const
    {
        return f_message_op;
    }


    //*********
    // Reply
    //*********

    inline bool msg_reply::is_request() const
    {
        return false;
    }
    inline bool msg_reply::is_reply() const
    {
        return true;
    }
    inline bool msg_reply::is_alert() const
    {
        return false;
    }
    inline bool msg_reply::is_info() const
    {
        return false;
    }

    inline bool msg_reply::derived_modify_amqp_message( amqp_message_ptr /*a_amqp_msg*/ ) const
    {
        return true;
    }

    inline bool msg_reply::derived_modify_message_body( param_node& a_node ) const
    {
        a_node.add( "retcode", new param_value( f_return_code ) );
        a_node.add( "return_msg", new param_value( f_return_msg ) );
        return true;
    }

    inline void msg_reply::set_return_code( unsigned a_retcode )
    {
        f_return_code = a_retcode;
        return;
    }

    inline unsigned msg_reply::get_return_code() const
    {
        return f_return_code;
    }

    inline void msg_reply::set_return_message( const std::string& a_ret_msg )
    {
        if( f_payload == NULL ) f_payload = new param_node();
        f_payload->replace( "return-msg", new param_value( a_ret_msg ) );
        return;
    }

    inline const std::string& msg_reply::get_return_message() const
    {
        f_return_buffer = f_payload->get_value( "return-msg" );
        return f_return_buffer;
    }


    //*********
    // Alert
    //*********

    inline bool msg_alert::is_request() const
    {
        return false;
    }
    inline bool msg_alert::is_reply() const
    {
        return false;
    }
    inline bool msg_alert::is_alert() const
    {
        return true;
    }
    inline bool msg_alert::is_info() const
    {
        return false;
    }

    inline bool msg_alert::derived_modify_amqp_message( amqp_message_ptr /*a_amqp_msg*/ ) const
    {
        return true;
    }

    inline bool msg_alert::derived_modify_message_body( param_node& /*a_node*/ ) const
    {
        return true;
    }



    //********
    // Info
    //********

    inline bool msg_info::is_request() const
    {
        return false;
    }
    inline bool msg_info::is_reply() const
    {
        return false;
    }
    inline bool msg_info::is_alert() const
    {
        return false;
    }
    inline bool msg_info::is_info() const
    {
        return true;
    }

    inline bool msg_info::derived_modify_amqp_message( amqp_message_ptr /*a_amqp_msg*/ ) const
    {
        return true;
    }

    inline bool msg_info::derived_modify_message_body( param_node& /*a_node*/ ) const
    {
        return true;
    }



} /* namespace mantis */

#endif /* MT_MESSAGE_HH_ */
