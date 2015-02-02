/*
 * mt_device_manager.hh
 *
 *  Created on: Jan 25, 2015
 *      Author: nsoblath
 */

#ifndef MT_DEVICE_MANAGER_HH_
#define MT_DEVICE_MANAGER_HH_

#include "mt_digitizer.hh"

#include "mt_condition.hh"

#include <string>

namespace mantis
{
#ifdef _WIN32
    MANTIS_EXPIMP_TEMPLATE template class MANTIS_API std::basic_string< char, std::char_traits< char >, std::allocator< char > >;
#endif

    class run_description;

    class MANTIS_API device_manager
    {
        public:
            device_manager();
            virtual ~device_manager();

            bool configure( run_description& a_run_desc );

            bool set_device( const std::string& a_dev, unsigned a_buffer_size, unsigned a_block_size );
            digitizer* device();

            buffer* get_buffer();
            condition* buffer_condition();

        private:
            std::string f_device_name;
            digitizer* f_device;
            condition f_buffer_condition;
            buffer* f_buffer;
            unsigned f_buffer_size;
            unsigned f_block_size;

    };

} /* namespace mantis */

#endif /* MT_DEVICE_MANAGER_HH_ */
