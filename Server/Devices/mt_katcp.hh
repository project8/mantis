#ifndef MT_KATCP_HH_
#define MT_KATCP_HH_

#include <string>


struct katcl_line;

namespace mantis
{
    
    class katcp
    {
        public:
            katcp();
            virtual ~katcp();

            int program_bof( const std::string& a_bof_file );

            int write_to_reg( const std::string& a_regname, int a_buffer, int a_length);
            int read_from_reg( const std::string& a_regname, void* a_buffer, int a_length );

            /* add other katcp functions here */

            const std::string& get_server_ip() const;
            void set_server_ip( const std::string& a_ip );

            unsigned get_timeout() const;
            void set_timeout( unsigned timeout_ms );

        private:
            int dispatch_client( const char* msgname, int verbose );

            std::string f_server_ip;
            struct katcl_line* f_cmd_line;
            int f_file_desc;

            unsigned f_timeout; // = 5000; /*Time out in ms*/

    };

}

#endif
