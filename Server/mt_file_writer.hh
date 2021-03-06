#ifndef MT_FILE_WRITER_HH_
#define MT_FILE_WRITER_HH_

#include "mt_writer.hh"

#include "M3Monarch.hh"

#include "thorax.hh"

#include <string>

namespace mantis
{
    class device_manager;

    /*!
     @class file_writer
     @author D. Furse, N. Oblath

     @brief Writer class for creating egg files via the Monarch library.

     @details
     Configuration (via file_writer::configure):
      * Details on the digitizer parameters are used to fill in  values in the egg-file header.
        If these are not supplied, default values are used (defaults are  the correct values for the px1500 digitizer).

     <b>Non-generic-writer usage:</b>
     This class use a acq_request object to fill in the description field in the header.  A default
     acq_request is used if the user does not supply one.  This should be set via file_writer::set_run_desciprtion
     prior to calling writer::initialize.  This class will extract the client software information and the description
     string from the request object. All other acq_request information should already be set prior to
     calling writer::initialize.
    */
    class MANTIS_API file_writer : public writer
    {
        public:
            file_writer();
            virtual ~file_writer();

            void set_device_manager( device_manager* a_dev_mgr );

            bool initialize_derived( acq_request* a_run_desc );

            void finalize_derived( param_node* a_response );

        private:
            monarch3::Monarch3* f_monarch;
            monarch3::M3Header* f_header;
            monarch3::M3Stream* f_stream;
            monarch3::M3Record* f_record;
            byte_type* f_data;
            acquisition_id_type f_last_acquisition_id;

            device_manager* f_dev_mgr;

            bool write( block* a_block );
    };
}

#endif
