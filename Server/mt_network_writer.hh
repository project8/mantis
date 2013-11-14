#ifndef MT_NETWORK_WRITER_HH_
#define MT_NETWORK_WRITER_HH_

#include "mt_writer.hh"

#include "mt_record_dist.hh"

namespace mantis
{

    class network_writer :
        public writer
    {
        public:
            network_writer( buffer* a_buffer, condition* a_condition );
            virtual ~network_writer();

            void initialize( request* a_response );

        private:
            record_dist* f_record_dist;

            bool write( block* a_block );
    };

}

#endif
