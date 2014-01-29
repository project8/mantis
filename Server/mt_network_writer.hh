#ifndef MT_NETWORK_WRITER_HH_
#define MT_NETWORK_WRITER_HH_

#include "mt_writer.hh"

#include "mt_client.hh"
#include "mt_record_dist.hh"

namespace mantis
{

    class network_writer :
        public writer
    {
        public:
            network_writer();
            virtual ~network_writer();

            void configure( configurator* a_config );

            bool initialize_derived( request* a_request );
            void finalize( response* a_response );

            size_t get_data_chunk_size() const;
            void set_data_chunk_size( size_t size );

        private:
            record_dist* f_record_dist;
            client* f_client;

            size_t f_data_chunk_size;

            bool write( block* a_block );
    };
}

#endif
