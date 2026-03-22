// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// Custom Script Commands for 2025+ rAthena
// COMPATIBLE VERSION - Uses look[] array for older rAthena
// UPDATED: Added mob_id parameter support (30200-30249)

#include "clif.hpp"
#include "mob.hpp"
#include "map.hpp"
#include "script.hpp"
#include "status.hpp"

namespace buildin_fakeplayer_playerdata {
    struct player_data {
        int class_id;
        uint8 sex;
        uint8 hair;
        uint8 hair_color;
        uint16 weapon;
        uint16 shield;
        uint16 head_top;
        uint16 head_mid;
        uint16 head_bottom;
        uint8 cloth_color;
        uint8 option;
        uint8 sit;
        uint8 direction;
    };
}

// ===== BUILDIN_FUNC(fakeplayer) =====
// Updated syntax: fakeplayer("map", x, y, "name", mob_id, job, sex, hair, h_color, weap, shie, top, mid, bot, opt, cloth, sit, direction)
BUILDIN_FUNC(fakeplayer)
{
    const char* mapname = script_getstr(st, 2);
    int x = script_getnum(st, 3);
    int y = script_getnum(st, 4);
    const char* name = script_getstr(st, 5);
    int mob_id = script_getnum(st, 6);  // NEW: mob_id parameter (30201-30250)

    buildin_fakeplayer_playerdata::player_data pd;
    pd.class_id   = script_getnum(st, 7);   // Changed from 6 to 7
    pd.sex        = script_getnum(st, 8);   // Changed from 7 to 8
    pd.hair       = script_getnum(st, 9);   // Changed from 8 to 9
    pd.hair_color = script_getnum(st, 10);  // Changed from 9 to 10
    pd.weapon     = script_getnum(st, 11);  // Changed from 10 to 11
    pd.shield     = script_getnum(st, 12);  // Changed from 11 to 12
    pd.head_top   = script_getnum(st, 13);  // Changed from 12 to 13
    pd.head_mid   = script_getnum(st, 14);  // Changed from 13 to 14
    pd.head_bottom= script_getnum(st, 15);  // Changed from 14 to 15
    pd.option     = script_getnum(st, 16);  // Changed from 15 to 16
    pd.cloth_color= script_getnum(st, 17);  // Changed from 16 to 17
    pd.sit        = script_getnum(st, 18);  // Changed from 17 to 18
    pd.direction  = script_getnum(st, 19);  // Changed from 18 to 19

    // Validate mob_id range (30201-30250)
    if (mob_id < 30201 || mob_id > 30250) {
        ShowWarning("fakeplayer: mob_id %d out of range (30201-30250), using 30201\n", mob_id);
        mob_id = 30201;
    }

    int16 mapid = map_mapname2mapid(mapname);
    if (mapid < 0) return SCRIPT_CMD_FAILURE;

    // Use custom mob_id instead of hardcoded 1900
    struct mob_data* md = mob_once_spawn_sub(
        NULL, mapid, x, y, name, mob_id, "", 0, AI_NONE
    );
    if (!md) return SCRIPT_CMD_FAILURE;

    mob_set_dynamic_viewdata(md);

    // Set player appearance
    md->vd->look[LOOK_BASE] = pd.class_id;
    
    md->vd->look[LOOK_WEAPON]        = pd.weapon;
    md->vd->look[LOOK_SHIELD]        = pd.shield;
    md->vd->look[LOOK_HEAD_TOP]      = pd.head_top;
    md->vd->look[LOOK_HEAD_MID]      = pd.head_mid;
    md->vd->look[LOOK_HEAD_BOTTOM]   = pd.head_bottom;
    md->vd->look[LOOK_HAIR]          = pd.hair;
    md->vd->look[LOOK_HAIR_COLOR]    = pd.hair_color;
    md->vd->look[LOOK_CLOTHES_COLOR] = pd.cloth_color;
    md->vd->sex                      = pd.sex;

    md->sc.option = pd.option;
    if (pd.sit) md->vd->dead_sit = 2;

    // Set direction (0-7, where 0=North, 2=West, 4=South, 6=East)
    if (pd.direction >= 0 && pd.direction <= 7) {
        md->ud.dir = pd.direction;
    }

    md->status.hp = md->status.max_hp = 100000;
    md->status.sp = md->status.max_sp = 1000;

    mob_spawn(md);

    script_pushint(st, md->id);
    return SCRIPT_CMD_SUCCESS;
}


BUILDIN_FUNC(player_mbase)
{
    const char* mapname = script_getstr(st, 2);
    int x = script_getnum(st, 3);
    int y = script_getnum(st, 4);
    const char* name = script_getstr(st, 5);
    int mob_class = script_getnum(st, 6);
    
    // Player appearance data
    buildin_fakeplayer_playerdata::player_data pd;
    pd.class_id    = script_getnum(st, 7);
    pd.sex         = script_getnum(st, 8);
    pd.hair        = script_getnum(st, 9);
    pd.hair_color  = script_getnum(st, 10);
    pd.weapon      = script_getnum(st, 11);
    pd.shield      = script_getnum(st, 12);
    pd.head_top    = script_getnum(st, 13);
    pd.head_mid    = script_getnum(st, 14);
    pd.head_bottom = script_getnum(st, 15);
    int robe_id    = script_getnum(st, 16);
    pd.option      = script_getnum(st, 17);
    pd.cloth_color = script_getnum(st, 18);
    
    int ai_type = script_getnum(st, 19);
    const char* event_name = script_getstr(st, 20);
    
    // Validate map
    int16 mapid = map_mapname2mapid(mapname);
    if (mapid < 0) {
        ShowWarning("player_mbase: Invalid map '%s'\n", mapname);
        script_pushint(st, 0);
        return SCRIPT_CMD_FAILURE;
    }
    
    // Spawn monster with custom appearance
    struct mob_data* md = mob_once_spawn_sub(
        NULL, mapid, x, y, name, mob_class, event_name, 0, (enum mob_ai)ai_type
    );
    
    if (!md) {
        ShowError("player_mbase: Failed to spawn '%s' (class %d)\n", name, mob_class);
        script_pushint(st, 0);
        return SCRIPT_CMD_FAILURE;
    }
    
    // Set dynamic view data for custom appearance
    mob_set_dynamic_viewdata(md);
    
    // CRITICAL: Apply player appearance to monster
    md->vd->look[LOOK_BASE]          = pd.class_id;
    md->vd->look[LOOK_HAIR]          = pd.hair;
    md->vd->look[LOOK_HAIR_COLOR]    = pd.hair_color;
    md->vd->look[LOOK_WEAPON]        = pd.weapon;
    md->vd->look[LOOK_SHIELD]        = pd.shield;
    md->vd->look[LOOK_HEAD_TOP]      = pd.head_top;
    md->vd->look[LOOK_HEAD_MID]      = pd.head_mid;
    md->vd->look[LOOK_HEAD_BOTTOM]   = pd.head_bottom;
    md->vd->look[LOOK_ROBE]          = robe_id;
    md->vd->look[LOOK_CLOTHES_COLOR] = pd.cloth_color;
    md->vd->sex                      = pd.sex;
    
    // Set options (cart, riding, etc)
    md->sc.option = pd.option;
    
    // Random direction
    md->ud.dir = rand() % 8;
    
    // ========== SET PROPERTIES FOR FAKE PLAYER ==========
    // Set race to DemiHuman (same as real players)
    md->status.race = RC_PLAYER_HUMAN;
    
    // Set mode to allow normal behavior
    md->status.mode = MD_CANMOVE | MD_CANATTACK | MD_ASSIST;
    // ====================================================
    
    // Spawn the monster
    mob_spawn(md);
    
    // Return monster GID
    script_pushint(st, md->id);
    return SCRIPT_CMD_SUCCESS;
}