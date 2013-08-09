#ifndef WRITER_HH_
#define WRITER_HH_

#include "callable.hh"

#include "types.hh"
#include "buffer.hh"
#include "condition.hh"
#include "request.pb.h"
#include "response.pb.h"
#include "Monarch.hpp"
#include "MonarchHeader.hpp"
#include "MonarchRecord.hpp"

namespace mantis
{

    class writer :
        public callable
    {
        public:
            writer( buffer* a_buffer, condition* a_condition );
            virtual ~writer();

            void initialize( request* a_response );
            void execute();
            void finalize( response* a_response );

        private:
            buffer* f_buffer;
            condition* f_condition;
            Monarch* f_monarch;
            MonarchHeader* f_header;
            MonarchRecord* f_record;

            record_id_t f_record_count;
            acquisition_id_t f_acquisition_count;
            timestamp_t f_live_time;

            bool write( block* a_block );
    };

}

#endif
