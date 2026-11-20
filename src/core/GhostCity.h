#ifndef __GHOSTCITY_H__
#define __GHOSTCITY_H__

class CGhostCity
{
public:
    static void Initialise(void);
    static void Process(void);
    static void Shutdown(void);
private:
    static bool ms_initialised;
};

#endif
