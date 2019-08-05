#include <sys/epoll.h>

class event_mgr final {
  event_mgr(lua_State* L, int max_event);
  ~event_mgr();
  int connect(lua_State* L);
  int listen(lua_State* L);
  int wait(lua_State* L);
  int send(lua_State* L);

private:
  std::vector<epoll_event> events;
  int m_max_event = 0;
  lua_State* m_lvm = nullptr;
}