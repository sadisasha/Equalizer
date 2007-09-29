/*  
    vertexBufferState.h
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    All rights reserved.  
    
    Header file of the VertexBufferState class.
*/


#ifndef MESH_VERTEXBUFFERSTATE_H
#define MESH_VERTEXBUFFERSTATE_H


#include "typedefs.h"
#include <map>

#ifdef EQUALIZER
#   include <eq/client/objectManager.h>
#endif // EQUALIZER


namespace mesh 
{
    /*  The abstract base class for kd-tree rendering state.  */
    class VertexBufferState
    {
    public:
        enum
        {
            FAILED = 0xffffffffu //<! return value for failed operations.
        };

        virtual bool useColors() const { return _useColors; }
        virtual void setColors( const bool colors ) { _useColors = colors; }
        virtual RenderMode getRenderMode() const { return _renderMode; }
        virtual void setRenderMode( const RenderMode mode ) 
        { 
            _renderMode = mode;
        }
        
        virtual GLuint getDisplayList( const void* key ) = 0;
        virtual GLuint newDisplayList( const void* key ) = 0;
        virtual GLuint getBufferObject( const void* key ) = 0;
        virtual GLuint newBufferObject( const void* key ) = 0;
        
        virtual void deleteAll() = 0;

        virtual const GLFunctions* getGLFunctions() const 
        { 
            return _glFunctions; 
        }
        
    protected:
        VertexBufferState( const GLFunctions* glFunctions ) 
            : _glFunctions( glFunctions ), _useColors( false ), 
              _renderMode( DISPLAY_LIST_MODE ) 
        {
            MESHASSERT( glFunctions );
        } 
        
        virtual ~VertexBufferState() {}
        
        const GLFunctions*  _glFunctions;
        bool                _useColors;
        RenderMode          _renderMode;
        
    private:
    };
    
    
    /*  Simple state for stand-alone single-pipe usage.  */
    class VertexBufferStateSimple : public VertexBufferState 
    {
    public:
        VertexBufferStateSimple( const GLFunctions* glFunctions )
            : VertexBufferState( glFunctions ) {}
        
        virtual GLuint getDisplayList( const void* key )
        {
            if( _displayLists.find( key ) == _displayLists.end() )
                return FAILED;
            return _displayLists[key];
        }
        
        virtual GLuint newDisplayList( const void* key )
        {
            _displayLists[key] = glGenLists( 1 );
            return _displayLists[key];
        }
        
        virtual GLuint getBufferObject( const void* key )
        {
            if( _bufferObjects.find( key ) == _bufferObjects.end() )
                return FAILED;
            return _bufferObjects[key];
        }
        
        virtual GLuint newBufferObject( const void* key )
        {
            if( !_glFunctions->hasGenBuffers( ))
                return FAILED;
            _glFunctions->genBuffers( 1, &_bufferObjects[key] );
            return _bufferObjects[key];
        }
        
        virtual void deleteAll() { /* NOP, TBD */ }

    private:
        std::map< const void*, GLuint >  _displayLists;
        std::map< const void*, GLuint >  _bufferObjects;
    };
    
    
#ifdef EQUALIZER
    /*  State for Equalizer usage, uses Eq Object Manager.  */
    class EqVertexBufferState : public VertexBufferState 
    {
    public:
        EqVertexBufferState( const eq::GLFunctions* glFunctions ) 
                : VertexBufferState( glFunctions )
                , _objectManager( glFunctions )
            {} 
        
        virtual GLuint getDisplayList( const void* key )
            { return _objectManager.getList( key ); }
        
        virtual GLuint newDisplayList( const void* key )
            { return _objectManager.newList( key ); }
        
        virtual GLuint getTexture( const void* key )
            { return _objectManager.getTexture( key ); }
        
        virtual GLuint newTexture( const void* key )
            { return _objectManager.newTexture( key ); }
        
        virtual GLuint getBufferObject( const void* key )
            { return _objectManager.getBuffer( key ); }
        
        virtual GLuint newBufferObject( const void* key )
            { return _objectManager.newBuffer( key ); }
        
        virtual GLuint getProgram( const void* key )
            { return _objectManager.getProgram( key ); }
        
        virtual GLuint newProgram( const void* key )
            { return _objectManager.newProgram( key ); }
        
        virtual GLuint getShader( const void* key )
            { return _objectManager.getShader( key ); }
        
        virtual GLuint newShader( const void* key, GLenum type )
            { return _objectManager.newShader( key, type ); }

        virtual void deleteAll() { _objectManager.deleteAll(); }
        
    private:
        eq::ObjectManager< const void* >   _objectManager;
    };
#endif // EQUALIZER
    
    
}


#endif // MESH_VERTEXBUFFERSTATE_H
