#include "mtq_signaling_streambuf.hh"

q_signaling_streambuf::q_signaling_streambuf( QObject *parent, std::size_t buff_sz ) :
        QObject( parent ),
        f_buffer( buff_sz + 1 )
{
    char* t_base = &f_buffer.front();
    setp( t_base, t_base + f_buffer.size() - 1 );
}

q_signaling_streambuf::~q_signaling_streambuf()
{
}

q_signaling_streambuf::int_type q_signaling_streambuf::overflow( int_type ch )
{
    if( ch != traits_type::eof() )
    {
        *pptr() = ch;
        pbump( 1 );
        if( flush() )
        {
            return ch;
        }
    }
    return traits_type::eof();
}

int q_signaling_streambuf::sync()
{
    return flush() ? 0 : -1;
}

bool q_signaling_streambuf::flush()
{
    QString t_output;
    for( char* p = pbase(), *e = pptr(); p != e; ++p )
    {
        t_output.append( QChar( *p ) );
    }

    std::ptrdiff_t n = pptr() - pbase();
    pbump( -n );

    emit print_message( t_output );

    return true;
}
