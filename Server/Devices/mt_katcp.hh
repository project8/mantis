#ifndef MT_KATCP_HH_
#define MT_KATCP_HH_

#include <string>

#include <stdint.h>

struct katcl_line;

namespace mantis
{
    
    class katcp
    {
        public:
            katcp();
            virtual ~katcp();

            bool connect();

        public:
            //*******************************
            // KATCP Command Wrappers
            //*******************************

            /// Program the FPGA with the bof file specified
            int program_bof( const std::string& a_bof_file );

            /// Write an integer to the specified register
            int write_uint_to_reg( const std::string& a_regname, unsigned int a_buffer, int a_length = sizeof(unsigned long) );

            /// Read an integer from the specified register
            int read_from_reg( const std::string& a_regname, void* a_buffer, int a_length );

            /// Program the 10Gbe device and start the TAP driver
            /// a_device_mac: MAC address for the 10Gbe device (e.g. 00:12:34:56:78:9a); first two bits must be zeroes to be valid
            /// a_device_ip:  IP address for the 10Gbe device (e.g. 10.0.0.2)
            /// a_device_port: Port for the 10 Gbe device (e.g. 10000)
            int tap_start( const std::string& device_name, const std::string& a_device, const std::string& a_device_ip, uint16_t a_device_port, const std::string& a_device_mac );

            /* add other katcp functions here */

        public:
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
