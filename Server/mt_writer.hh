#ifndef MT_WRITER_HH_
#define MT_WRITER_HH_

#include "mt_callable.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "request.pb.h"
#include "response.pb.h"
#include "Monarch.hpp"
#include "MonarchHeader.hpp"
#include "MonarchRecord.hpp"
#include "thorax.hh"

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

            record_id_type f_record_count;
            acquisition_id_type f_acquisition_count;
            time_nsec_type f_live_time;

            bool write( block* a_block );
    };

}

#endif
