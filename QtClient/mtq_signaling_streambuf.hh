#ifndef MTQ_SIGNALING_STREAMBUF_HH
#define MTQ_SIGNALING_STREAMBUF_HH

#include <QObject>
#include <QString>

#include <streambuf>
#include <iosfwd>
#include <cstdlib>
#include <vector>

class q_signaling_streambuf : public QObject, public std::streambuf
{
    Q_OBJECT

public:
    explicit q_signaling_streambuf( QObject *parent = 0, std::size_t buff_sz = 256 );
    virtual ~q_signaling_streambuf();

protected:
    bool flush();

private:
    int_type overflow( int_type ch );
    int sync();

    q_signaling_streambuf( const q_signaling_streambuf& );
    q_signaling_streambuf& operator=( const q_signaling_streambuf& );

private:
    std::vector< char > f_buffer;

signals:
    void print_message( const QString& );

};

#endif // MTQ_SIGNALING_STREAMBUF_HH
