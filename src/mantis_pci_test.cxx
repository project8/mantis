#include <cstdlib>
#include <cstdio>

#include <string>
using std::string;

#include <sstream>
using std::stringstream;

#include <iostream>
using std::cout;
using std::endl;

#include <px1500.h>
#include <pthread.h>

struct argument_data
{
    unsigned int fCount;
    unsigned int fSize;
    double fRate;
    unsigned int fLength;
    string fFile;
};

typedef enum
{
    eRun,
    eStop
} status_t;
struct shared_data
{
    HPX4 fDigitizer;
    FILE* fFile;
    unsigned int fDuration;

    size_t fBufferSize; // number of arrays in buffer
    size_t fBufferArraySize; // number of elements in array
    size_t fBufferElementSize; // number of bytes in element

    px4_sample_t** fData;
    pthread_mutex_t* fDataMutex;
    pthread_mutex_t fBufferMutex;

    status_t fRunStatus;
    unsigned long int fRunReadCount;
    unsigned long int fRunWriteCount;
    pthread_mutex_t fRunMutex;
};

void* ReadThreadFunction( void* DataPtr )
{
    shared_data* Data = (shared_data*) (DataPtr);

    int PX4Result;

    status_t Status;

    pthread_mutex_lock( &Data->fRunMutex );
    Data->fRunReadCount = 0;
    pthread_mutex_unlock( &Data->fRunMutex );

    while( true )
    {
        pthread_mutex_lock( &Data->fRunMutex );
        Status = Data->fRunStatus;
        pthread_mutex_unlock( &Data->fRunMutex );
        if( Status == eStop )
        {
            break;
        }

        pthread_mutex_lock( &Data->fBufferMutex );

        PX4Result = BeginBufferedPciAcquisitionPX4( Data->fDigitizer, PX4_FREE_RUN );
        for( size_t Index = 0; Index < Data->fBufferSize; Index++ )
        {
            pthread_mutex_lock( &Data->fRunMutex );
            Status = Data->fRunStatus;
            pthread_mutex_unlock( &Data->fRunMutex );
            if( Status == eStop )
            {
                break;
            }

            PX4Result = GetPciAcquisitionDataFastPX4( Data->fDigitizer, Data->fBufferArraySize, Data->fData[Index], 0 );
            pthread_mutex_unlock( &Data->fDataMutex[Index] );

            pthread_mutex_lock( &Data->fRunMutex );
            Data->fRunReadCount++;
            pthread_mutex_unlock( &Data->fRunMutex );
        }
        PX4Result = EndBufferedPciAcquisitionPX4( Data->fDigitizer );
    }

    return NULL;
}

void* WriteThreadFunction( void* DataPtr )
{
    shared_data* Data = (shared_data*) (DataPtr);

    int WriteResult;

    status_t Status;

    pthread_mutex_lock( &Data->fRunMutex );
    Data->fRunWriteCount = 0;
    pthread_mutex_unlock( &Data->fRunMutex );

    while( true )
    {
        pthread_mutex_lock( &Data->fRunMutex );
        Status = Data->fRunStatus;
        pthread_mutex_unlock( &Data->fRunMutex );
        if( Status == eStop )
        {
            break;
        }

        for( size_t Index = 0; Index < Data->fBufferSize; Index++ )
        {
            pthread_mutex_lock( &Data->fRunMutex );
            Status = Data->fRunStatus;
            pthread_mutex_unlock( &Data->fRunMutex );
            if( Status == eStop )
            {
                break;
            }

            pthread_mutex_lock( &Data->fDataMutex[Index] );
            WriteResult = fwrite( Data->fData[Index], Data->fBufferElementSize, Data->fBufferArraySize, Data->fFile );

            pthread_mutex_lock( &Data->fRunMutex );
            Data->fRunWriteCount++;
            pthread_mutex_unlock( &Data->fRunMutex );
        }

        pthread_mutex_unlock( &Data->fBufferMutex );
    }

    return NULL;
}

void* RunThreadFunction( void* DataPtr )
{
    shared_data* Data = (shared_data*) (DataPtr);

    for( unsigned int Seconds = 0; Seconds < Data->fDuration; Seconds++ )
    {
        sleep(1);
        pthread_mutex_lock( &Data->fRunMutex );
        cout << Data->fRunReadCount << " records acquired..." << endl;
        cout << Data->fRunWriteCount << " records written..." << endl;
        cout << Data->fDuration - Seconds << " seconds remaining..." << endl;
        pthread_mutex_unlock( &Data->fRunMutex );
    }

    pthread_mutex_lock( &Data->fRunMutex );
    Data->fRunStatus = eStop;
    pthread_mutex_unlock( &Data->fRunMutex );

    return NULL;
}


int main( int anArgc, char** anArgv )
{
    // grab arguments

    argument_data Argument;

    Argument.fCount = 643; // seems to be the maximum...
    Argument.fSize = 4; // seems to be the maximum...
    Argument.fRate = 500.0;
    Argument.fLength = 600;
    Argument.fFile = string("MantisPciTest.binary");

    if( (anArgc-1)%2 != 0 )
    {
        cout << "malformed argument_data" << endl;
        exit(-1);
    }

    stringstream Converter;
    for( int Index = 1; Index < anArgc; Index = Index + 2 )
    {
        if( anArgv[Index][0] == '-' )
        {
            switch( anArgv[Index][1] )
            {
                case 'c':
                    Converter.clear();
                    Converter.str("");
                    Converter << anArgv[Index+1];
                    Converter >> Argument.fCount;
                    break;
                case 's':
                    Converter.clear();
                    Converter.str("");
                    Converter << anArgv[Index+1];
                    Converter >> Argument.fSize;
                    break;
                case 'r':
                    Converter.clear();
                    Converter.str("");
                    Converter << anArgv[Index+1];
                    Converter >> Argument.fRate;
                    break;
                case 'l':
                    Converter.clear();
                    Converter.str("");
                    Converter << anArgv[Index+1];
                    Converter >> Argument.fLength;
                    break;
                case 'o':
                    Converter.clear();
                    Converter.str("");
                    Converter << anArgv[Index+1];
                    Converter >> Argument.fFile;
                    break;
                default:
                    cout << "unknown option: -" << anArgv[Index][1] << endl;
                    exit(-1);
            }
        }
        else
        {
            cout << "unknown argument: " << anArgv[Index] << endl;
        }
    }

    cout << "block count: " << Argument.fCount << endl;
    cout << "block size: " << Argument.fSize << endl;
    cout << "digitization rate: " << Argument.fRate << endl;
    cout << "run length in seconds: " << Argument.fLength << endl;
    cout << "run file name: " << Argument.fFile << endl;


    // make shared data

    shared_data Shared;
    Shared.fDuration = Argument.fLength;
    pthread_mutex_init( &Shared.fRunMutex, NULL );


    // initialize digitizer

    int PX4Result; //this reused variable holds the results of px4 library function calls

    cout << "connecting to digitizer..." << endl;
    PX4Result = ConnectToDevicePX4( &Shared.fDigitizer, 1 );
    if( PX4Result != SIG_SUCCESS )
    {
        DumpLibErrorPX4( PX4Result, "failed to connect to digitizer card: " );
        return -1;
    }

    cout << "setting digizer defaults..." << endl;
    PX4Result = SetPowerupDefaultsPX4( Shared.fDigitizer );
    if( PX4Result != SIG_SUCCESS )
    {
        DumpLibErrorPX4( PX4Result, "failed to enter default state: " );
        return -1;
    }

    cout << "enabling channel one..." << endl;
    PX4Result = SetActiveChannelsPX4( Shared.fDigitizer, PX4CHANSEL_SINGLE_CH1 );
    if( PX4Result != SIG_SUCCESS )
    {
        DumpLibErrorPX4( PX4Result, "failed to activate channel 1: " );
        return -1;
    }

    cout << "setting sample rate..." << endl;
    PX4Result = SetInternalAdcClockRatePX4( Shared.fDigitizer, Argument.fRate );
    if( PX4Result != SIG_SUCCESS )
    {
        DumpLibErrorPX4( PX4Result, "failed to set sampling rate: " );
        return -1;
    }


    // allocate arrays

    cout << "allocating dma buffers..." << endl;

    Shared.fBufferSize = Argument.fCount;
    Shared.fBufferArraySize = Argument.fSize * 1048576 ;
    Shared.fBufferElementSize = sizeof( px4_sample_t );

    Shared.fData = new px4_sample_t*[Shared.fBufferSize];
    Shared.fDataMutex = new pthread_mutex_t[Shared.fBufferSize];

    for( size_t Index = 0; Index < Shared.fBufferSize; Index++ )
    {
        cout << "calling: AllocateDmaBufferPX4( " << Shared.fDigitizer << ", " << Shared.fBufferArraySize << ", " << &(Shared.fData[Index]) << " )" << endl;
        PX4Result = AllocateDmaBufferPX4( Shared.fDigitizer, Shared.fBufferArraySize, &(Shared.fData[Index]) );
        if( PX4Result != SIG_SUCCESS )
        {
            cout << "error on buffer " << Index << endl;
            DumpLibErrorPX4( PX4Result, "failed to allocate DMA buffer: " );
            return -1;
        }
        pthread_mutex_init( &Shared.fDataMutex[Index], NULL );
        pthread_mutex_lock( &Shared.fDataMutex[Index] );
    }
    pthread_mutex_init( &Shared.fBufferMutex, NULL );


    // initialize file

    cout << "creating output file..." << endl;

    Shared.fFile = fopen( Argument.fFile.c_str() , "wb");
    if( Shared.fFile == NULL )
    {
        cout << "failed to open output file for write..." << endl;
        return -1;
    }


    // start threads

    cout << "starting acquisition..." << endl;

    pthread_t WriteThread;
    pthread_t ReadThread;
    pthread_t RunThread;

    pthread_create( &WriteThread, NULL, &WriteThreadFunction, &Shared );
    pthread_create( &ReadThread, NULL, &ReadThreadFunction, &Shared );
    pthread_create( &RunThread, NULL, &RunThreadFunction, &Shared );

    pthread_join( WriteThread, NULL );
    pthread_join( ReadThread, NULL );
    pthread_join( RunThread, NULL );


    // finalize file

    cout << "closing output file..." << endl;

    fclose( Shared.fFile );


    // deallocate arrays

    cout << "deallocating dma buffers..." << endl;

    for( size_t Index = 0; Index < Shared.fBufferSize; Index++ )
    {
        PX4Result = FreeDmaBufferPX4( Shared.fDigitizer, Shared.fData[Index] );
        if( PX4Result != SIG_SUCCESS )
        {
            cout << "error on buffer " << Index << endl;
            DumpLibErrorPX4( PX4Result, "failed to deallocate DMA buffer: " );
            return -1;
        }
        pthread_mutex_destroy( &Shared.fDataMutex[Index] );
    }
    pthread_mutex_destroy( &Shared.fBufferMutex );

    delete[] Shared.fData;
    delete[] Shared.fDataMutex;


    // finalize digitizer

    cout << "disconnecting from digitizer..." << endl;

    PX4Result = DisconnectFromDevicePX4( Shared.fDigitizer );
    if( PX4Result != SIG_SUCCESS )
    {
        DumpLibErrorPX4( PX4Result, "failed to disconnect from digitizer card: " );
        return -1;
    }


    // clean up run mutex

    pthread_mutex_destroy( &Shared.fRunMutex );

    return 0;
}
