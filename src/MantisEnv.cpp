#include "MantisEnv.hpp"

MantisEnv::MantisEnv() :
    fFileName( "mantis_out.egg" ), // default output name
    fRunDuration( 600000 ), // default run length in milliseconds
    fAcquisitionRate( 500.0 ), // default ADC clock rate (MHz)
    fChannelMode( 1 ), // number of active channels
    fRecordLength( 4194304 ), // number of bytes in a single record
    fBufferCount( 640 ) // number of circular buffer nodes
{
}

void MantisEnv::setFileName( std::string newOutName )
{
    (*this).fFileName = newOutName;
    return;
}
std::string MantisEnv::getFileName()
{
    return (*this).fFileName;
}

void MantisEnv::setAcquisitionRate( std::string clockRateStr )
{
    try
    {
        std::istringstream( clockRateStr ) >> (*this).fAcquisitionRate;
    }
    catch( std::exception& e )
    {
        throw new clock_rate_exception( clockRateStr );
    }
    return;
}
double MantisEnv::getAcquisitionRate()
{
    return (*this).fAcquisitionRate;
}

void MantisEnv::setRunDuration( std::string runLengthStr )
{
    try
    {
        std::istringstream( runLengthStr ) >> (*this).fRunDuration;
    }
    catch( std::exception& e )
    {
        throw new run_length_exception( runLengthStr );
    }

    return;
}
unsigned int MantisEnv::getRunDuration()
{
    return (*this).fRunDuration;
}

void MantisEnv::setRecordLength( std::string dWidthStr )
{
    std::istringstream( dWidthStr ) >> (*this).fRecordLength;
    return;
}
unsigned int MantisEnv::getRecordLength()
{
    return (*this).fRecordLength;
}

void MantisEnv::setBufferCount( std::string bufCountStr )
{
    std::istringstream( bufCountStr ) >> (*this).fBufferCount;
    return;
}
std::size_t MantisEnv::getBufferCount()
{
    return (*this).fBufferCount;
}

void MantisEnv::setChannelMode( std::string channelModeStr )
{
    try
    {
        std::istringstream( channelModeStr ) >> (*this).fChannelMode;
    }
    catch( std::exception& e )
    {
        throw new run_length_exception( channelModeStr );
    }

    if( ((*this).fChannelMode < 1) || ((*this).fChannelMode > 2) )
    {
        throw new channel_mode_exception( channelModeStr );
    }

    return;
}
unsigned int MantisEnv::getChannelMode()
{
    return (*this).fChannelMode;
}

safeEnvPtr MantisEnv::parseArgs( int argc, char** argv )
{
    safeEnvPtr result( new MantisEnv() );

    int c;
    try
    {
        while( (c = getopt( argc, argv, "o:d:r:m:l:c:" )) != -1 )
        {
            switch( c )
            {
                case 'o':
                    result->setFileName( optarg );
                    break;
                case 'd':
                    result->setRunDuration( optarg );
                    break;
                case 'r':
                    result->setAcquisitionRate( optarg );
                    break;
                case 'm':
                    result->setChannelMode( optarg );
                    break;
                case 'l':
                    result->setRecordLength( optarg );
                    break;
                case 'c':
                    result->setBufferCount( optarg );
                    break;
                default:
                    throw new argument_exception();
            }
        }
    }
    catch( argument_exception* e )
    {
        throw e;
    }
    return result;
}

void MantisEnv::verifyEnvironment( safeEnvPtr someEnvironment )
{
    return;
}

std::ostream& operator <<( std::ostream& outstream, safeEnvPtr& obj )
{
    outstream << "output file name: " << (obj.get())->getFileName() << "\n" << "digitizer rate: " << (obj.get())->getAcquisitionRate() << "(MHz)\n" << "run length: " << (obj.get())->getRunDuration() << "(ms)\n" << "record length: " << (obj.get())->getRecordLength() << "(bytes)\n" << "buffer count: " << (obj.get())->getBufferCount() << "(entries)\n";

    return outstream;
}
