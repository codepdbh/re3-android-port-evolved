#ifndef __GHOSTCITY_P2P_H__
#define __GHOSTCITY_P2P_H__

class CGhostCityP2P
{
public:
    static void Initialise(void);
    static void Process(void);
    static void Shutdown(void);
    static bool IsAvailable(void);
};

#endif
