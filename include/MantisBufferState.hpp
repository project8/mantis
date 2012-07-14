#ifndef MANTISBUFFERSTATE_HPP_
#define MANTISBUFFERSTATE_HPP_

class MantisBufferState
{
    public:
        MantisBufferState();
        MantisBufferState( const MantisBufferState& aCopy );
        ~MantisBufferState();

        bool IsAcquiring() const;
        void SetAcquiring();

        bool IsAcquired() const;
        void SetAcquired();

        bool IsFlushing() const;
        void SetFlushing();

        bool IsFree() const;
        void SetFree();

    private:
        enum { eAcquiring, eAcquired, eFlushing, eFree } fValue;
};

#endif
