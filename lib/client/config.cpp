
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

#include "client.h"
#include "configEvent.h"
#include "frame.h"
#include "frameData.h"
#include "global.h"
#include "log.h"
#include "node.h"
#include "nodeFactory.h"
#include "packets.h"
#include "server.h"

#include <eq/net/command.h>
#include <eq/net/global.h>

using namespace eqBase;
using namespace std;
using eqNet::CommandFunc;

namespace eq
{

Config::Config( eqBase::RefPtr< Server > server )
        : Session( true )
        , _latency( 0 )
        , _currentFrame( 0 )
        , _unlockedFrame( 0 )
        , _finishedFrame( 0 )
        , _running( false )
{
    eqNet::CommandQueue* queue    = server->getNodeThreadQueue();
    eqNet::CommandQueue* cmdQueue = server->getCommandThreadQueue();

    registerCommand( CMD_CONFIG_CREATE_NODE,
                     CommandFunc<Config>( this, &Config::_cmdCreateNode ),
                     queue );
    registerCommand( CMD_CONFIG_DESTROY_NODE,
                     CommandFunc<Config>( this, &Config::_cmdDestroyNode ),
                     queue );
    registerCommand( CMD_CONFIG_START_INIT_REPLY, 
                     CommandFunc<Config>( this, &Config::_cmdStartInitReply ),
                     queue );
    registerCommand( CMD_CONFIG_FINISH_INIT_REPLY,
                     CommandFunc<Config>( this, &Config::_cmdFinishInitReply ),
                     queue );
    registerCommand( CMD_CONFIG_EXIT_REPLY, 
                     CommandFunc<Config>( this, &Config::_cmdExitReply ),
                     queue );
    registerCommand( CMD_CONFIG_START_FRAME_REPLY, 
                     CommandFunc<Config>( this, &Config::_cmdStartFrameReply ),
                     queue );
    registerCommand( CMD_CONFIG_FINISH_FRAME_REPLY, 
                     CommandFunc<Config>( this, &Config::_cmdFinishFrameReply ),
                     queue );
    registerCommand( CMD_CONFIG_FINISH_ALL_FRAMES_REPLY, 
                 CommandFunc<Config>( this, &Config::_cmdFinishAllFramesReply ),
                     queue );
    registerCommand( CMD_CONFIG_EVENT, 
                     CommandFunc<Config>( this, &Config::_cmdEvent ),
                     cmdQueue );
#ifdef EQ_TRANSMISSION_API
    registerCommand( CMD_CONFIG_DATA, 
                     CommandFunc<Config>( this, &Config::_cmdData ),
                     queue );
#endif
}

Config::~Config()
{
    _appNodeID = eqNet::NodeID::ZERO;
    _appNode   = 0;
}

eqBase::RefPtr<Server> Config::getServer()
{ 
    RefPtr<eqNet::Node> node = eqNet::Session::getServer();
    EQASSERT( dynamic_cast< Server* >( node.get( )));
    return RefPtr_static_cast< eqNet::Node, Server >( node );
}

eqBase::RefPtr<Client> Config::getClient()
{ 
    return getServer()->getClient(); 
}

void Config::_addNode( Node* node )
{
    EQASSERT( node->getConfig() == this );
    _nodes.push_back( node );
}

void Config::_removeNode( Node* node )
{
    vector<Node*>::iterator i = find( _nodes.begin(), _nodes.end(), node );
    EQASSERT( i != _nodes.end( ));
    _nodes.erase( i );
}

Node* Config::_findNode( const uint32_t id )
{
    for( vector<Node*>::const_iterator i = _nodes.begin(); i != _nodes.end(); 
         ++i )
    {
        Node* node = *i;
        if( node->getID() == id )
            return node;
    }
    return 0;
}

bool Config::_startInit( const uint32_t initID )
{
    _currentFrame = 0;
    _unlockedFrame = 0;
    _finishedFrame = 0;

    ConfigStartInitPacket packet;
    packet.requestID    = _requestHandler.registerRequest();
    packet.initID       = initID;

    send( packet );
    
    RefPtr< Client > client = getClient();
    while( !_requestHandler.isServed( packet.requestID ))
        client->processCommand();

    bool ret = false;
    _requestHandler.waitRequest( packet.requestID, ret );
    return ret;
}

bool Config::_finishInit()
{
    EQASSERT( !_running );
    registerObject( &_headMatrix );

    ConfigFinishInitPacket packet;
    packet.requestID    = _requestHandler.registerRequest();
    packet.headMatrixID = _headMatrix.getID();

    send( packet );
    
    RefPtr< Client > client = getClient();
    while( !_requestHandler.isServed( packet.requestID ))
        client->processCommand();

    _requestHandler.waitRequest( packet.requestID, _running );

    if( !_running )
        deregisterObject( &_headMatrix );

        handleEvents();
    return _running;
}

bool Config::exit()
{
    _running = false;

    ConfigExitPacket packet;
    packet.requestID = _requestHandler.registerRequest();
    send( packet );

    RefPtr< Client > client = getClient();
    while( !_requestHandler.isServed( packet.requestID ))
        client->processCommand();

    bool ret = false;
    _requestHandler.waitRequest( packet.requestID, ret );

    deregisterObject( &_headMatrix );
    while( _eventQueue.tryPop( )) ; // flush all pending events

    return ret;
}

uint32_t Config::startFrame( const uint32_t frameID )
{
    ConfigStartFramePacket packet;
    packet.requestID = _requestHandler.registerRequest();
    packet.frameID   = frameID;

    const uint32_t frameNumber = _currentFrame + 1;
    send( packet );

    RefPtr< Client > client = getClient();
    while( !_requestHandler.isServed( packet.requestID ))
        client->processCommand();

    _requestHandler.waitRequest( packet.requestID );
    EQASSERT( frameNumber == _currentFrame );
    EQLOG( LOG_ANY ) << "---- Started Frame ---- " << frameNumber << endl;
    return frameNumber;
}

uint32_t Config::finishFrame()
{
    ConfigFinishFramePacket packet;
    send( packet );

    RefPtr< Client > client        = getClient();
    const uint32_t   frameToFinish = (_currentFrame >= _latency) ? 
                                      _currentFrame - _latency : 0;

    while( _unlockedFrame < _currentFrame || // local sync
           _finishedFrame < frameToFinish )  // global sync

        client->processCommand();

    handleEvents();
    EQLOG( LOG_ANY ) << "----- Finish Frame ---- " << frameToFinish << " ("
                     << _currentFrame << ')' << endl;
    return frameToFinish;
}

uint32_t Config::finishAllFrames()
{
    ConfigFinishAllFramesPacket packet;
    send( packet );

    RefPtr< Client > client = getClient();
    while( _finishedFrame < _currentFrame )
        client->processCommand();

    handleEvents();
    EQLOG( LOG_ANY ) << "-- Finish All Frames --" << endl;
    return _currentFrame;
}


void Config::sendEvent( ConfigEvent& event )
{
    EQASSERT( _appNodeID );

    if( !_appNode )
    {
        RefPtr<eqNet::Node> localNode = getLocalNode();
        RefPtr<eqNet::Node> server    = eqNet::Session::getServer();
        _appNode = localNode->connect( _appNodeID, server );
    }
    EQASSERT( _appNode );

    event.sessionID = getID();
    EQLOG( LOG_EVENTS ) << "send event " << &event << endl;
    _appNode->send( event );
}

const ConfigEvent* Config::nextEvent()
{
    eqNet::Command* command = _eventQueue.pop();
    return command->getPacket<ConfigEvent>();
}

const ConfigEvent* Config::tryNextEvent()
{
    eqNet::Command* command = _eventQueue.tryPop();
    if( !command )
        return 0;
    return command->getPacket<ConfigEvent>();
}

void Config::handleEvents()
{
    for( const ConfigEvent* event = tryNextEvent(); event; 
         event = tryNextEvent( ))
    {
        if( !handleEvent( event ))
            EQVERB << "Unhandled " << event << endl;
    }
}

bool Config::handleEvent( const ConfigEvent* event )
{
    switch( event->data.type )
    {
        case Event::WINDOW_CLOSE:
            _running = false;
            return true;

        case Event::KEY_PRESS:
            if( event->data.keyPress.key == eq::KC_ESCAPE )
            {
                _running = false;
                return true;
            }    
            break;

        case Event::POINTER_BUTTON_PRESS:
            if( event->data.pointerButtonPress.buttons == 
                ( eq::PTR_BUTTON1 | eq::PTR_BUTTON2 | eq::PTR_BUTTON3 ))
            {
                _running = false;
                return true;
            }
            break;
    }

    return false;
}

void Config::setHeadMatrix( const vmml::Matrix4f& matrix )
{
    _headMatrix = matrix;
    _headMatrix.commit();
}

#ifdef EQ_TRANSMISSION_API
void Config::broadcastData( const void* data, uint64_t size )
{
    if( _clientNodeIDs.empty( ))
        return;

    if( !_connectClientNodes( ))
        return;

    ConfigDataPacket packet;
    packet.sessionID = getID();
    packet.dataSize  = size;

    for( vector< RefPtr<eqNet::Node> >::iterator i = _clientNodes.begin();
         i != _clientNodes.end(); ++i )
    {
        (*i)->send( packet, data, size );
    }
}

bool Config::_connectClientNodes()
{
    if( !_clientNodes.empty( ))
        return true;

    RefPtr< eqNet::Node > localNode = getLocalNode();
    RefPtr< eqNet::Node > server    = getServer();

    for( vector< eqNet::NodeID >::const_iterator i = _clientNodeIDs.begin();
         i < _clientNodeIDs.end(); ++i )
    {
        const eqNet::NodeID&        id   = *i;
        RefPtr< eqNet::Node > node = localNode->connect( id, server );

        if( !node.isValid( ))
        {
            EQERROR << "Can't connect node with ID " << id << endl;
            _clientNodes.clear();
            return false;
        }

        _clientNodes.push_back( node );
    }
    return true;
}
#endif // EQ_TRANSMISSION_API

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Config::_cmdCreateNode( eqNet::Command& command )
{
    const ConfigCreateNodePacket* packet = 
        command.getPacket<ConfigCreateNodePacket>();
    EQINFO << "Handle create node " << packet << endl;
    EQASSERT( packet->nodeID != EQ_ID_INVALID );

    Node* node = Global::getNodeFactory()->createNode( this );
    attachObject( node, packet->nodeID );

    ConfigCreateNodeReplyPacket reply( packet );
    send( command.getNode(), reply );
    
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdDestroyNode( eqNet::Command& command ) 
{
    const ConfigDestroyNodePacket* packet =
        command.getPacket<ConfigDestroyNodePacket>();
    EQINFO << "Handle destroy node " << packet << endl;

    Node* node = _findNode( packet->nodeID );
    if( !node )
        return eqNet::COMMAND_HANDLED;

    detachObject( node );
    delete node;

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdStartInitReply( eqNet::Command& command )
{
    const ConfigStartInitReplyPacket* packet = 
        command.getPacket<ConfigStartInitReplyPacket>();
    EQINFO << "handle start init reply " << packet << endl;

#ifdef EQ_TRANSMISSION_API
    _clientNodeIDs.clear();
    _clientNodes.clear();

    if( packet->result )
    {
        for( uint32_t i=0; i<packet->nNodeIDs; ++i )
            _clientNodeIDs.push_back( packet->data.nodeIDs[i] );
    }
    else
        _error = packet->data.error;
#else
    if( !packet->result )
        _error = packet->error;
#endif

    _latency = packet->latency;
    _requestHandler.serveRequest( packet->requestID, (void*)(packet->result) );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdFinishInitReply( eqNet::Command& command )
{
    const ConfigFinishInitReplyPacket* packet = 
        command.getPacket<ConfigFinishInitReplyPacket>();
    EQINFO << "handle finish init reply " << packet << endl;

    if( !packet->result )
    {
        _error = packet->error;
#ifdef EQ_TRANSMISSION_API
        _clientNodeIDs.clear();
        _clientNodes.clear();
#endif
    }

    _requestHandler.serveRequest( packet->requestID, (void*)(packet->result) );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdExitReply( eqNet::Command& command )
{
    const ConfigExitReplyPacket* packet = 
        command.getPacket<ConfigExitReplyPacket>();
    EQINFO << "handle exit reply " << packet << endl;

#ifdef EQ_TRANSMISSION_API
    _clientNodeIDs.clear();
    _clientNodes.clear();
#endif

    _requestHandler.serveRequest( packet->requestID, (void*)(packet->result) );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdStartFrameReply( eqNet::Command& command )
{
    const ConfigStartFrameReplyPacket* packet =
        command.getPacket<ConfigStartFrameReplyPacket>();
    EQVERB << "handle frame start reply " << packet << endl;

#if 0 // enable when list of client nodes is dynamic
    _clientNodeIDs.clear();
    _clientNodes.clear();

    for( uint32_t i=0; i<packet->nNodeIDs; ++i )
    {
        _clientNodeIDs.push_back( packet->nodeIDs[i] );
        _clientNodeIDs[i].convertToHost();
    }
#endif

    _currentFrame = packet->frameNumber;
    if( _nodes.empty( )) // no local rendering - release sync immediately
        releaseFrameLocal( packet->frameNumber );

    _requestHandler.serveRequest( packet->requestID );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdFinishFrameReply( eqNet::Command& command )
{
    const ConfigFinishFrameReplyPacket* packet = 
        command.getPacket<ConfigFinishFrameReplyPacket>();
    EQVERB << "handle frame finish reply " << packet << endl;

    _finishedFrame = packet->frameNumber;

    if( _unlockedFrame < _finishedFrame )
    {
        EQWARN << "Finished frame was not locally unlocked, enforcing unlock" 
               << endl;
        _unlockedFrame = _finishedFrame;
    }

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdFinishAllFramesReply( eqNet::Command& command )
{
    const ConfigFinishAllFramesReplyPacket* packet = 
        command.getPacket<ConfigFinishAllFramesReplyPacket>();
    EQVERB << "handle all frames finish reply " << packet << endl;

    _finishedFrame = packet->frameNumber;
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdEvent( eqNet::Command& command )
{
    EQLOG( LOG_EVENTS ) << "received event " << command.getPacket<ConfigEvent>()
                        << endl;

    _eventQueue.push( command );
    return eqNet::COMMAND_HANDLED;
}

#ifdef EQ_TRANSMISSION_API
eqNet::CommandResult Config::_cmdData( eqNet::Command& command )
{
    EQVERB << "received data " << command.getPacket<ConfigDataPacket>()
           << endl;

    // If we -for whatever reason- instantiate more than one config node per
    // client, the server has to send us a list of config node object IDs
    // together with the eqNet::NodeIDs, so that broadcastData can send the
    // packet directly to the eq::Node objects.
    EQASSERTINFO( _nodes.size() == 1, 
                  "More than one config node instantiated locally" );

    _nodes[0]->_dataQueue.push( command );
    return eqNet::COMMAND_HANDLED;
}
#endif
}
