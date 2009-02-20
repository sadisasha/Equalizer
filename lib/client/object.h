
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_OBJECT_H
#define EQ_OBJECT_H

#include <eq/net/object.h>        // base class

namespace eq
{
    /**
     * Base class for all distributed, inheritable objects in the eq namespace.
     *
     * This class implements one usage pattern of net::Object, which allows
     * subclassing and serialization of distributed Objects used by
     * Equalizer. The inheritance Object -> Frustum -> View illustrates
     * the usage of eq::Object.
     */
    class Object : public net::Object
    {
    public:
        /** Construct a new Object. */
        EQ_EXPORT Object();
        
        /** Destruct the object. */
        EQ_EXPORT virtual ~Object();

        /** Set the name of the object. */
        EQ_EXPORT void setName( const std::string& name );

        /** @return the name of the object. */
        EQ_EXPORT const std::string& getName() const;

    protected:
        /** @return true if the view has to be committed. */
        virtual bool isDirty() const { return ( _dirty != DIRTY_NONE ); }

        /** 
         * Worker for pack() and getInstanceData().
         *
         * Override this and deserialize() if you want to distribute subclassed
         * data.
         *
         * This method is called with DIRTY_ALL from getInstanceData() and with
         * the actual dirty bits from pack(), which also resets the dirty state
         * afterwards. The dirty bits are transmitted beforehand, and do not
         * need to be transmitted by the overriding method.
         */
        EQ_EXPORT virtual void serialize( net::DataOStream& os,
                                          const uint64_t dirtyBits );

        /** 
         * Worker for unpack() and applyInstanceData().
         * 
         * This function is called with the dirty bits send by the master
         * instance. The dirty bits are received beforehand, and do not need to
         * be transmitted by the overriding method.
         * 
         * @sa serialize()
         */
        EQ_EXPORT virtual void deserialize( net::DataIStream& is, 
                                            const uint64_t dirtyBits );

        virtual ChangeType getChangeType() const { return UNBUFFERED; }

        /** 
         * The changed parts of the object since the last pack().
         *
         * Subclasses should define their own enum, starting at DIRTY_CUSTOM.
         */
        enum DirtyBits
        {
            DIRTY_NONE       = 0,
            DIRTY_NAME       = 1 << 0,
            DIRTY_FILL1      = 1 << 1, // Leave room for binary-compatible patch
            DIRTY_FILL2      = 1 << 2,
            DIRTY_CUSTOM     = 1 << 3,
            DIRTY_ALL        = 0xffffffffffffffffull
        };

        /** Set dirty flags to mark data for distribution. */
        EQ_EXPORT void setDirty( const uint64_t bits );

        /** @sa eq::net::Object::attachToSession. */
        EQ_EXPORT virtual void attachToSession( const uint32_t id, 
                                                const uint32_t instanceID, 
                                                net::Session* session );

    private:
        EQ_EXPORT virtual void getInstanceData( net::DataOStream& os );
        EQ_EXPORT virtual void pack( net::DataOStream& os );
        EQ_EXPORT virtual void applyInstanceData( net::DataIStream& is );

        /** The current dirty bits. */
        uint64_t _dirty;

        /** The application-defined name of the object. */
        std::string _name;
    };
}
#endif // EQ_OBJECT_H
