#include "mt_bit_shift_modifier.hh"

#include "mt_configurator.hh"
#include "mt_exception.hh"
#include "mt_factory.hh"
#include "mt_logger.hh"
#include "mt_run_description.hh"

#include <cstring> // for memcpy()
using std::stringstream;

namespace mantis
{
    MTLOGGER( mtlog, "bit_shift_modifier" );

    MT_REGISTER_MODIFIER( bit_shift_modifier, "bit-shift" );

    bit_shift_modifier::bit_shift_modifier() :
            writer(),
            f_dig_params(),
            f_run_desc( NULL )
    {
        // give some reasonable digitizer parameter defaults (these are from the px1500)
        get_calib_params( 8, 1, -0.25, 0.5, &f_dig_params );
    }
    bit_shift_modifier::~bit_shift_modifier()
    {
        delete f_run_desc;
    }

    void bit_shift_modifier::configure( const param_node* a_config )
    {
        // this should be present whether the configuration is from the client or from the server
        get_calib_params(
                a_config->get_value< unsigned >( "bit-depth",      f_dig_params.bit_depth ),
                a_config->get_value< unsigned >( "data-type-size", f_dig_params.data_type_size ),
                a_config->get_value< double   >( "voltage-min",    f_dig_params.v_min ),
                a_config->get_value< double   >( "voltage-range",  f_dig_params.v_range ),
                &f_dig_params );
        return;
    }

    void bit_shift_modifier::set_run_description( run_description* a_run_desc )
    {
        delete f_run_desc;
        f_run_desc = a_run_desc;
        return;
    }

    bool bit_shift_modifier::initialize_derived( request* a_request )
    {

        return true;
    }
    bool bit_shift_modifier::modify( block* a_block )
    {
        //::memcpy( f_record->fData, a_block->data_bytes(), a_block->get_data_nbytes() );

        return true;
    }

}
