#ifndef MT_BIT_SHIFT_MODIFIER_HH_
#define MT_BIT_SHIFT_MODIFIER_HH_

#include "mt_modifier.hh"

#include "mt_logger.hh"

#include <string>

namespace mantis
{
    MTLOGGER( mtlog_bsm, "bit_shift_modifier" );

    /*!
     @class bit_shift_modifier
     @author N. Oblath

     @brief Modifier class for shifting data bits

     @details
    */
    template< typename DataType >
    class bit_shift_modifier : public modifier
    {
        public:
            typedef DataType data_type;

        public:
            bit_shift_modifier();
            virtual ~bit_shift_modifier();

            void set_bit_shift( unsigned a_shift );

            bool initialize_derived( param_node* a_node );

        private:
            unsigned f_bit_shift;

            block_view< DataType > f_view;

            bool modify( block* a_block );
    };

    template< typename DataType >
    bit_shift_modifier< DataType >::bit_shift_modifier() :
            modifier(),
            f_bit_shift( 0 ),
            f_view()
    {
    }

    template< typename DataType >
    bit_shift_modifier< DataType >::~bit_shift_modifier()
    {
    }

    template< typename DataType >
    void bit_shift_modifier< DataType >::configure( const param_node* a_config )
    {
        return;
    }

    template< typename DataType >
    void bit_shift_modifier< DataType >::set_bit_shift( unsigned a_shift )
    {
        f_bit_shift = a_shift;
        return;
    }

    template< typename DataType >
    bool bit_shift_modifier< DataType >::initialize_derived( request* a_request )
    {
        return true;
    }

    template< typename DataType >
    bool bit_shift_modifier< DataType >::modify( block* a_block )
    {
        //::memcpy( f_record->fData, a_block->data_bytes(), a_block->get_data_nbytes() );
        f_view.set_viewed( a_block );
        size_t t_data_size = f_view.get_data_view_size();
        for( unsigned index = 0; index < t_data_size; ++index )
        {
            //unsigned temp = f_view.data_view()[index] >> f_bit_shift;
            //if( index < 100 ) MTDEBUG( mtlog_bsm, "shifting at [" << index << "]: " << f_view.data_view()[index] << " to " << temp );
            f_view.data_view()[index] = f_view.data_view()[index] >> f_bit_shift;
        }
        return true;
    }

}

#endif
