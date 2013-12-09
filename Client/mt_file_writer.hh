#ifndef MT_FILE_WRITER_HH_
#define MT_FILE_WRITER_HH_

#include "mt_writer.hh"

#include "Monarch.hpp"
#include "MonarchHeader.hpp"
#include "MonarchRecord.hpp"

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
            Monarch* f_monarch;
            MonarchHeader* f_header;
            MonarchRecord* f_record;

            bool write( block* a_block );
    };
}

#endif
