#include "event.mgr.h"

event_mgr::event_mgr(lua_State* L, int max_event) : 
    m_lvm(L), m_max_event(max_event) {}

event_mgr::~event_mgr() {}

int event_mgr::connect(lua_State* L)
{

}

int event_mgr::listen(lua_State* L)
{

}

int event_mgr::wait(lua_State* L)
{

}

int event_mgr::send(lua_State* L)
{
    
}