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
            modifier(),
            f_bit_shift( 0 )
    {
    }
    bit_shift_modifier::~bit_shift_modifier()
    {
    }

    void bit_shift_modifier::configure( const param_node* /*a_config*/ )
    {
        return;
    }

    void bit_shift_modifier::set_run_description( run_description* a_run_desc )
    {
        delete f_run_desc;
        f_run_desc = a_run_desc;
        return;
    }

    bool bit_shift_modifier::initialize_derived( request* /*a_request*/ )
    {
        return true;
    }
    bool bit_shift_modifier::modify( block* a_block )
    {
        //::memcpy( f_record->fData, a_block->data_bytes(), a_block->get_data_nbytes() );

        return true;
    }

}
