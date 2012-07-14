#ifndef __env_hpp
#define __env_hpp

#include "MantisExceptions.hpp"
#include <sstream>
#include <memory>
#include <string>
#include <iostream>
#include <ostream>
#include <getopt.h>
#include <ctime>

class MantisEnv;
typedef std::auto_ptr< MantisEnv > safeEnvPtr;

class MantisEnv
{

    private:
        std::string fFileName;
        unsigned int fRunDuration;
        double fAcquisitionRate;
        unsigned int fChannelMode;
        std::size_t fRecordLength;
        std::size_t fBufferCount;

    public:
        MantisEnv();

        static safeEnvPtr parseArgs( int argc, char** argv );
        static void verifyEnvironment( safeEnvPtr );

        void setFileName( std::string );
        std::string getFileName();

        void setRunDuration( std::string );
        unsigned int getRunDuration();

        void setAcquisitionRate( std::string );
        double getAcquisitionRate();

        void setChannelMode( std::string );
        unsigned int getChannelMode();

        void setRecordLength( std::string );
        std::size_t getRecordLength();

        void setBufferCount( std::string );
        std::size_t getBufferCount();

        friend std::ostream& operator <<( std::ostream& outstream, safeEnvPtr& );
};

#endif
