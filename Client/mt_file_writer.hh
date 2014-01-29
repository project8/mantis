#ifndef MT_FILE_WRITER_HH_
#define MT_FILE_WRITER_HH_

#include "mt_writer.hh"

#include "Monarch.hpp"

#include <string>

namespace mantis
{

    class file_writer :
        public writer
    {
        public:
            file_writer();
            virtual ~file_writer();

            void configure( configurator* );

            bool initialize_derived( request* a_response );

        private:
            monarch::Monarch* f_monarch;
            monarch::MonarchHeader* f_header;
            monarch::MonarchRecordBytes* f_record;

            unsigned f_data_bytes;

            bool write( block* a_block );
    };
}

#endif
