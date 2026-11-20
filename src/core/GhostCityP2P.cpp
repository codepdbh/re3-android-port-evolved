#include "common.h"
#include "GhostCityP2P.h"
#include "World.h"
#include "PlayerPed.h"
#include "ModelIndices.h"
#include "Timer.h"

#ifdef ANDROID
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace {
#ifdef ANDROID
static constexpr uint32 GHOSTCITY_MAGIC = 0x47435031; // GCP1
static constexpr uint16 GHOSTCITY_PORT = 47821;
struct GhostCityPacket {
    uint32 magic;
    uint32 peer;
    float x, y, z, heading;
    uint32 tick;
};
struct GhostCityPeer {
    uint32 id;
    uint32 lastSeen;
    CPed *ped;
};
static int gSocket = -1;
static uint32 gPeerId = 0;
static uint32 gLastSend = 0;
static GhostCityPeer gPeers[4] = {};
#endif
static bool gInitialised = false;
}

void CGhostCityP2P::Initialise(void)
{
    if (gInitialised)
        return;
    gInitialised = true;
#ifdef ANDROID
    gPeerId = (uint32)(getpid() ^ (uint32)CTimer::GetTimeInMilliseconds());
    gSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (gSocket < 0) {
        gInitialised = false;
        return;
    }
    int yes = 1;
    setsockopt(gSocket, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes));
    setsockopt(gSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    fcntl(gSocket, F_SETFL, O_NONBLOCK);
    sockaddr_in local = {};
    local.sin_family = AF_INET;
    local.sin_port = htons(GHOSTCITY_PORT);
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(gSocket, (sockaddr*)&local, sizeof(local)) < 0) {
        close(gSocket);
        gSocket = -1;
        gInitialised = false;
    }
#endif
}

void CGhostCityP2P::Process(void)
{
    if (!gInitialised)
        return;
#ifdef ANDROID
    const uint32 now = CTimer::GetTimeInMilliseconds();
    CPed *local = CWorld::Players[CWorld::PlayerInFocus].m_pPed;
    if (local != nil && now - gLastSend >= 100 && gSocket >= 0) {
        GhostCityPacket packet = {};
        packet.magic = htonl(GHOSTCITY_MAGIC);
        packet.peer = htonl(gPeerId);
        packet.x = local->GetPosition().x;
        packet.y = local->GetPosition().y;
        packet.z = local->GetPosition().z;
        packet.heading = local->GetForward().Heading();
        packet.tick = htonl(now);
        sockaddr_in target = {};
        target.sin_family = AF_INET;
        target.sin_port = htons(GHOSTCITY_PORT);
        target.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        sendto(gSocket, &packet, sizeof(packet), 0, (sockaddr*)&target, sizeof(target));
        gLastSend = now;
    }

    for (;;) {
        GhostCityPacket packet = {};
        sockaddr_in from = {};
        socklen_t fromLen = sizeof(from);
        const ssize_t received = recvfrom(gSocket, &packet, sizeof(packet), 0,
            (sockaddr*)&from, &fromLen);
        if (received != sizeof(packet))
            break;
        if (ntohl(packet.magic) != GHOSTCITY_MAGIC || ntohl(packet.peer) == gPeerId)
            continue;
        const uint32 peerId = ntohl(packet.peer);
        GhostCityPeer *peer = nil;
        for (GhostCityPeer &candidate : gPeers)
            if (candidate.id == peerId) { peer = &candidate; break; }
        if (peer == nil) {
            for (GhostCityPeer &candidate : gPeers)
                if (candidate.id == 0) { peer = &candidate; candidate.id = peerId; break; }
        }
        if (peer == nil)
            continue;
        peer->lastSeen = now;
        if (peer->ped == nil) {
            peer->ped = new CPed(PEDTYPE_CIVMALE);
            peer->ped->SetModelIndex(MI_MALE01);
            peer->ped->SetPosition(packet.x, packet.y, packet.z);
            peer->ped->SetOrientation(0.0f, 0.0f, packet.heading);
            peer->ped->GetMatrix().UpdateRW();
            peer->ped->UpdateRwFrame();
            CWorld::Add(peer->ped);
        } else {
            peer->ped->SetPosition(packet.x, packet.y, packet.z);
            peer->ped->SetOrientation(0.0f, 0.0f, packet.heading);
        }
    }
    for (GhostCityPeer &peer : gPeers) {
        if (peer.id != 0 && now - peer.lastSeen > 3000) {
            if (peer.ped != nil) {
                CWorld::Remove(peer.ped);
                delete peer.ped;
            }
            peer = {};
        }
    }
#endif
}

void CGhostCityP2P::Shutdown(void)
{
#ifdef ANDROID
    for (GhostCityPeer &peer : gPeers) {
        if (peer.ped != nil) {
            CWorld::Remove(peer.ped);
            delete peer.ped;
        }
        peer = {};
    }
    if (gSocket >= 0)
        close(gSocket);
    gSocket = -1;
#endif
    gInitialised = false;
}

bool CGhostCityP2P::IsAvailable(void)
{
#ifdef ANDROID
    return gInitialised && gSocket >= 0;
#else
    return false;
#endif
}
