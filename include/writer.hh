#ifndef WRITER_HH_
#define WRITER_HH_

#include "callable.hh"

#include "types.hh"
#include "buffer.hh"
#include "mutex.hh"
#include "px1500.h"

namespace mantis
{

    class writer :
        public callable
    {
        public:
            writer( buffer* a_buffer, condition* a_condition );
            virtual ~writer();

            void Initialize( run* a_run );
            void Execute();
            void Finalize( run* a_run );

        private:
            Monarch* fMonarch;
            MonarchRecord* fMonarchRecordInterleaved;
            unsigned int fPciRecordSize;
            unsigned long fRecordCount;
            long long int fLiveTime;

            string fFileName;
            unsigned int fRunDuration;
            double fAcquisitionRate;
            unsigned int fRecordSize;
            unsigned int fChannelMode;
            long long int fStartTimeMonotonic;
            string fDescription;

            bool Flush( MantisBufferRecord* aBufferRecord );
    };

}

#endif
