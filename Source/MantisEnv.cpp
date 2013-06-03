#include "MantisEnv.hpp"

MantisEnv::MantisEnv() :
    fFileName( "mantis_out.egg" ), // default output name
    fRunDuration( 600000 ), // default run length in milliseconds
    fAcquisitionRate( 500.0 ), // default ADC clock rate (MHz)
    fChannelMode( 1 ), // number of active channels
    fRecordSize( 4194304 ), // number of bytes in a single record per channel
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

void MantisEnv::setRecordSize( std::string dWidthStr )
{
    std::istringstream( dWidthStr ) >> (*this).fRecordSize;
    return;
}
unsigned int MantisEnv::getRecordSize()
{
    return (*this).fRecordSize;
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

void MantisEnv::setDescription( std::string desc )
{
    (*this).fDescription = desc;
    return;
}

std::string MantisEnv::getDescription()
{
    return (*this).fDescription;
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
                    result->setRecordSize( optarg );
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

    char** descArray = argv + optind;
    int nWords = argc - optind;
    std::stringstream descStream;
    for (int iWord = 0; iWord < nWords; iWord++)
    {
        descStream << descArray[iWord] << " ";
    }
    result->setDescription(descStream.str());

    if( (result->fChannelMode == 1) && (result->fRecordSize > 4194304 ) )
    {
        result->fRecordSize = 4194304;
    }

    if( (result->fChannelMode == 2) && (result->fRecordSize > 2097152 ) )
    {
        result->fRecordSize = 2097152;
    }

    return result;
}

void MantisEnv::verifyEnvironment( safeEnvPtr someEnvironment )
{
    return;
}

std::ostream& operator <<( std::ostream& outstream, safeEnvPtr& obj )
{
    outstream << "mantis configuration:\n" << "  *output file name: " << (obj.get())->getFileName() << "\n" << "  *digitizer rate: " << (obj.get())->getAcquisitionRate() << "(MHz)\n" << "  *run duration: " << (obj.get())->getRunDuration() << "(ms)\n" << "  *channel mode: " << (obj.get())->getChannelMode() << "(number of channels)\n" << "  *record size: " << (obj.get())->getRecordSize() << "(bytes)\n" << "  *buffer count: " << (obj.get())->getBufferCount() << "(entries)\n";

    return outstream;
}
