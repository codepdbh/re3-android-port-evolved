#include "common.h"
#include "GhostCity.h"
#include "Game.h"
#include "ModelIndices.h"
#include "Object.h"
#include "Pools.h"
#include "World.h"
#include "Coronas.h"
#include "Timer.h"

#include <vector>

bool CGhostCity::ms_initialised = false;
static std::vector<CObject*> gGhostCityObjects;

static CObject *GhostCityObject(int model, const CVector &pos, float angle = 0.0f)
{
    CObject *object = new CObject(model, false);
    if (object == nil)
        return nil;
    object->ObjectCreatedBy = TEMP_OBJECT;
    object->SetPosition(pos);
    object->SetOrientation(0.0f, 0.0f, angle);
    object->GetMatrix().UpdateRW();
    object->UpdateRwFrame();
    object->bUsesCollision = true;
    CWorld::Add(object);
    gGhostCityObjects.push_back(object);
    return object;
}

void CGhostCity::Initialise(void)
{
    if (ms_initialised || CGame::currLevel != LEVEL_COMMERCIAL)
        return;
    ms_initialised = true;

    // Staunton Island is the main GhostCity district: it already has the
    // largest downtown skyline and supports the plaza without replacing GTA3's
    // streamed city. The plaza is deliberately built from existing GTA3 props
    // so it remains native and does not require external WebView assets.
    const CVector centre(0.0f, 0.0f, 1.0f);

    // Symmetric wet-looking promenade: lamps, trees, bollards and benches.
    for (int i = -3; i <= 3; i++) {
        const float x = i * 7.0f;
        GhostCityObject(MI_SINGLESTREETLIGHTS1, CVector(x, -7.0f, 1.0f), 0.0f);
        GhostCityObject(MI_SINGLESTREETLIGHTS1, CVector(x, 7.0f, 1.0f), HALFPI);
		const int treeVariant = (i < 0 ? -i : i) % 4;
		GhostCityObject(MI_TREE4 + treeVariant, CVector(x, -11.0f, 1.0f));
		GhostCityObject(MI_TREE4 + treeVariant, CVector(x, 11.0f, 1.0f));
    }
    for (int i = -2; i <= 2; i++) {
        GhostCityObject(MI_BOLLARDLIGHT, CVector(i * 3.0f, -3.0f, 1.0f));
        GhostCityObject(MI_BOLLARDLIGHT, CVector(i * 3.0f, 3.0f, 1.0f));
    }

    // Heart silhouette made from native light props, with coronas registered in
    // Process() to create the magenta neon focal point from the reference.
    const CVector heart(centre.x, centre.y, centre.z + 4.0f);
    const CVector points[] = {
        CVector(-2.4f, 0.0f, 4.4f), CVector(-1.2f, 0.0f, 5.5f),
        CVector( 1.2f, 0.0f, 5.5f), CVector( 2.4f, 0.0f, 4.4f),
        CVector(-1.5f, 0.0f, 3.2f), CVector( 0.0f, 0.0f, 1.8f),
        CVector( 1.5f, 0.0f, 3.2f)
    };
    for (const CVector &p : points)
        GhostCityObject(MI_BOLLARDLIGHT, CVector(centre.x + p.x, centre.y + p.y, p.z));

    // Two side pads anchor the plaza composition and create a clear approach.
    for (int side : {-1, 1}) {
        for (int i = 0; i < 3; i++) {
            GhostCityObject(MI_PARKTABLE, CVector(side * (11.0f + i * 2.0f), -1.5f, 1.0f));
            GhostCityObject(MI_PARKTABLE, CVector(side * (11.0f + i * 2.0f), 1.5f, 1.0f));
        }
    }
}

void CGhostCity::Process(void)
{
    if (!ms_initialised || CGame::currLevel != LEVEL_COMMERCIAL)
        return;

    const uint8 pulse = (uint8)(190 + 45 * Sin((float)CTimer::GetTimeInMilliseconds() * 0.004f));
    const CVector centre(0.0f, 0.0f, 1.0f);
    const CVector points[] = {
        CVector(-2.4f, 0.0f, 5.4f), CVector(-1.2f, 0.0f, 6.5f),
        CVector( 1.2f, 0.0f, 6.5f), CVector( 2.4f, 0.0f, 5.4f),
        CVector(-1.5f, 0.0f, 4.2f), CVector( 0.0f, 0.0f, 2.8f),
        CVector( 1.5f, 0.0f, 4.2f)
    };
    for (int i = 0; i < 7; i++) {
        CCoronas::RegisterCorona(0x47535400 + i, 255, 24, 150, pulse,
            centre + points[i], 1.2f, 90.0f, CCoronas::TYPE_NORMAL,
            CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
            CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
    }
}

void CGhostCity::Shutdown(void)
{
    for (CObject *object : gGhostCityObjects) {
        if (object != nil) {
            CWorld::Remove(object);
            delete object;
        }
    }
    gGhostCityObjects.clear();
    ms_initialised = false;
}
