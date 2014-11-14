#ifndef MT_FILE_WRITER_HH_
#define MT_FILE_WRITER_HH_

#include "mt_writer.hh"

#include "Monarch.hpp"

#include "thorax.hh"

#include <string>

namespace mantis
{
    class run_description;

    /*!
     @class file_writer
     @author D. Furse, N. Oblath

     @brief Writer class for creating egg files via the Monarch library.

     @details
     Configuration (via file_writer::configure):
      * Details on the digitizer parameters are used to fill in  values in the egg-file header.
        If these are not supplied, default values are used (defaults are  the correct values for the px1500 digitizer).

     <b>Non-generic-writer usage:</b>
     This class use a run_description object to fill in the description field in the header.  A default
     run_description is used if the user does not supply one.  This should be set via file_writer::set_run_desciprtion
     prior to calling writer::initialize.  This class will extract the client software information and the description
     string from the request object. All other run_description information should already be set prior to
     calling writer::initialize.
    */
    class file_writer : public writer
    {
        public:
            file_writer();
            virtual ~file_writer();

            void configure( const param_node* );

            void set_run_description( run_description* a_run_desc );

            bool initialize_derived( request* a_response );

        private:
            monarch::Monarch* f_monarch;
            monarch::MonarchHeader* f_header;
            monarch::MonarchRecordBytes* f_record;

            struct dig_calib_params f_dig_params;

            run_description* f_run_desc;

            bool write( block* a_block );
    };
}

#endif
