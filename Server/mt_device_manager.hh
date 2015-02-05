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

//#ifdef _WIN32
//MANTIS_EXPIMP_TEMPLATE template class MANTIS_API std::basic_string< char, std::char_traits< char >, std::allocator< char > >;
//#endif

namespace mantis
{
    class run_description;

    class MANTIS_API device_manager
    {
        public:
            device_manager();
            virtual ~device_manager();

            bool configure( run_description& a_run_desc );

            bool set_device( const std::string& a_dev );
            digitizer* device();

            buffer* get_buffer();
            condition* buffer_condition();

        private:
            std::string f_device_name;
            digitizer* f_device;

    };

} /* namespace mantis */

#endif /* MT_DEVICE_MANAGER_HH_ */
