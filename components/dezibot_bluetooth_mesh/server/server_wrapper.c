#include "server/server_events.h"
#include "server/server_wrapper.h"

void mesh_server_evt_dispatch(mesh_server_t *srv, const mesh_server_evt_t *evt)
{
    if (srv && srv->cb)
    {
        srv->cb(evt);
    }
}