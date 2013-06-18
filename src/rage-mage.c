#include "angband.h"

static bool _unclear_mind = TRUE;

void rage_mage_rage_fueled(int dam)
{
    int x = dam;
    int y = p_ptr->chp;
    int sp = x*(p_ptr->mhp*3/2 - y)/p_ptr->mhp;

    if (sp < 1)
        sp = 1;
    
    p_ptr->csp += sp;
    if (p_ptr->csp > p_ptr->msp)
    {
        p_ptr->csp = p_ptr->msp;
        p_ptr->csp_frac = 0;
    }
    p_ptr->redraw |= PR_MANA;

    /*_unclear_mind = FALSE;*/
}

void rage_mage_blood_lust(int dam)
{
    int sp;
    if (p_ptr->shero)
        sp = dam/8;
    else
        sp = dam/12;
        
    if (sp < 1)
        sp = 1;
        
    p_ptr->csp += sp;
    if (p_ptr->csp > p_ptr->msp)
    {
        p_ptr->csp = p_ptr->msp;
        p_ptr->csp_frac = 0;
    }
    p_ptr->redraw |= PR_MANA;            
    
    _unclear_mind = FALSE;
}

static void _anti_magic_ray_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Antimagic Ray", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Block spells from a chosen foe.", ""));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_aim_dir(&dir)) return;
        fire_ball(GF_ANTIMAGIC, dir, 1, 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _armor_of_fury_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Armor of Fury", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Whenever a monster attacks you with magic, they may become slowed and stunned.", ""));
        break;
    case SPELL_CAST:
        set_tim_armor_of_fury(25 + randint1(25), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _barbarian_lore_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Barbarian Lore");
        break;
    default:
        identify_spell(cmd, res);
        break;
    }
}

static void _barbaric_resistance_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Barbaric Resistance", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Grants temporary protection from the elements.", ""));
        break;
    case SPELL_CAST:
    {
        int base = 10;
        
        if (p_ptr->shero)
            base = 20;

        set_oppose_acid(randint1(base) + base, FALSE);
        set_oppose_elec(randint1(base) + base, FALSE);
        set_oppose_fire(randint1(base) + base, FALSE);
        set_oppose_cold(randint1(base) + base, FALSE);
        set_oppose_pois(randint1(base) + base, FALSE);
    
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _crude_mapping_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Crude Mapping");
        break;
    case SPELL_DESC:
        var_set_string(res, "Maps the dungeon in your vicinity.");
        break;
    case SPELL_CAST:
        map_area(14);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static bool _detect_objects_ego(int range)
{
    int i, y, x;

    bool detect = FALSE;

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS) range /= 3;

    /* Scan all objects */
    for (i = 1; i < o_max; i++)
    {
        object_type *o_ptr = &o_list[i];

        if (!o_ptr->k_idx) continue;
        if (o_ptr->held_m_idx) continue;

        y = o_ptr->iy;
        x = o_ptr->ix;

        if (distance(py, px, y, x) > range) continue;

        if (object_is_artifact(o_ptr) ||
            object_is_ego(o_ptr) )
        {
            o_ptr->marked |= OM_FOUND;
            lite_spot(y, x);
            detect = TRUE;
        }
    }

    if (detect)
        msg_print("You sense the presence of magic objects!");

    return detect;
}

static void _detect_magic_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Detect Magic", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Detects nearby magic users and items.", ""));
        break;
    case SPELL_CAST:
        detect_monsters_magical(DETECT_RAD_DEFAULT);
        _detect_objects_ego(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _detect_magical_foes_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Detect Magical Foes", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Detects nearby magic users.", ""));
        break;
    case SPELL_CAST:
        detect_monsters_magical(DETECT_RAD_DEFAULT);
        if (p_ptr->shero)
            set_tim_esp_magical(20 + randint1(20), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _evasive_leap_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Evasive Leap", ""));
        break;
    case SPELL_ENERGY:
        if (p_ptr->shero)
        {
            var_set_int(res, 30);
            break;
        }
    default:
        strafing_spell(cmd, res);
        break;
    }
}

static void _focus_rage_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Focus Rage", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Damage yourself and regain spell points.", ""));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, 10 + p_ptr->lev/2));
        break;
    case SPELL_FAIL:
    {
        int hp = 10 + p_ptr->lev/2;
        take_hit(DAMAGE_NOESCAPE, hp, "Rage", -1);
        break;
    }
    case SPELL_CAST:
    {
        int hp = 10 + p_ptr->lev/2;

        var_set_bool(res, FALSE);

        if (p_ptr->chp < hp)
        {
            if (!get_check("Really? This will kill you!")) return;
        }

        take_hit(DAMAGE_NOESCAPE, hp, "Rage", -1);
        sp_player(hp);

        _unclear_mind = FALSE; /* Hack to avoid automatic mana drain for this action */
        var_set_bool(res, TRUE);
        break;    
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _force_brand_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Force Brand", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Temporarily brands your weapon with force.", ""));
        break;
    case SPELL_CAST:
    {
        int base = 4;
        if (p_ptr->shero)
            base = 10;
        set_tim_force(base + randint1(base), FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _greater_focus_rage_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Focus *Rage*", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Damage yourself and regain spell points.", ""));
        break;
    case SPELL_INFO:
        if (p_ptr->shero)
            var_set_string(res, info_damage(0, 0, 2 * p_ptr->lev));
        else
            var_set_string(res, info_damage(0, 0, 10 + p_ptr->lev));
        break;
    case SPELL_FAIL:
    {
        int hp = 10 + p_ptr->lev;
        if (p_ptr->shero)
            hp = 2 * p_ptr->lev;
        take_hit(DAMAGE_NOESCAPE, hp, "Rage", -1);
        break;
    }
    case SPELL_CAST:
    {
        int hp = 10 + p_ptr->lev;

        var_set_bool(res, FALSE);
        
        if (p_ptr->shero)
            hp = 2 * p_ptr->lev;

        if (p_ptr->chp < hp)
        {
            if (!get_check("Really? This will kill you!")) return;
        }

        take_hit(DAMAGE_NOESCAPE, hp, "Rage", -1);
        sp_player(hp * 2);

        _unclear_mind = FALSE; /* Hack to avoid automatic mana drain for this action */
        var_set_bool(res, TRUE);
        break;    
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _greater_shout_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Greater Shout", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Projects a cone of sound at a chosen foe.", ""));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(p_ptr->lev - 10, 8, 0));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_aim_dir(&dir)) return;
        fire_ball(GF_SOUND, dir, damroll(p_ptr->lev - 10, 8), -3);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mana_clash_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Mana Clash", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Fires a ball at chosen target.  Only spellcasters will be damaged.", ""));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_aim_dir(&dir)) return;
        fire_ball(GF_MANA_CLASH, dir, 24 * p_ptr->lev, 2); /* damage later divided by Y + 1 in spells1!project_m where Y is spell freq */
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static int _rage_strike_dam(void)
{
    int sp = p_ptr->csp;
    int z = sp*sp/100;
    return 1200*z/(1000+z);
}

static void _rage_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Ragestrike", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Fire a ball of pure rage at chosen foe, striking with everything you've got!", ""));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _rage_strike_dam()));
        break;
    case SPELL_FAIL:
        sp_player(-p_ptr->csp);
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);

        if (p_ptr->chp < 100)
        {
            if (!get_check("Really? This will kill you!")) return;
        }

        if (!get_aim_dir(&dir)) return;

        fire_ball(GF_MISSILE, dir, _rage_strike_dam(), 0);
        take_hit(DAMAGE_NOESCAPE, 100, "Rage", -1);
        if (!p_ptr->shero)
            set_stun(99, FALSE); /* 100 is Knocked Out */

        sp_player(-p_ptr->csp); /* Don't use SPELL_COST_EXTRA since we pay mana up front these days! */
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rage_sustenance_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Rage Sustenance");
        break;
    default:
        satisfy_hunger_spell(cmd, res);
        break;
    }
}

static void _resist_curses_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Resist Curses", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Grants temporary magical resistance.", ""));
        break;
    case SPELL_CAST:
        set_tim_resist_curses(20 + randint1(20), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _resist_disenchantment_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Resist Disenchantment", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Grants temporary resistance to disenchantment.", ""));
        break;
    case SPELL_CAST:
        set_tim_res_disenchantment(10 + randint1(10), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static bool _object_is_(object_type *o_ptr, int tv, int sv)
{
    if (o_ptr->tval == tv && o_ptr->sval == sv) return TRUE;
    return FALSE;
}

static int _object_dam_type(object_type *o_ptr)
{            
    if (_object_is_(o_ptr, TV_STAFF, SV_STAFF_DARKNESS)) return GF_DARK;
    if (_object_is_(o_ptr, TV_STAFF, SV_STAFF_LITE)) return GF_LITE;
    if (_object_is_(o_ptr, TV_STAFF, SV_STAFF_STARLITE)) return GF_LITE;
    if (_object_is_(o_ptr, TV_STAFF, SV_STAFF_SLOWNESS)) return GF_INERT;
    if (_object_is_(o_ptr, TV_STAFF, SV_STAFF_HASTE_MONSTERS)) return GF_INERT;
    if (_object_is_(o_ptr, TV_STAFF, SV_STAFF_SLOW_MONSTERS)) return GF_INERT;
    if (_object_is_(o_ptr, TV_STAFF, SV_STAFF_SPEED)) return GF_INERT;

    if (_object_is_(o_ptr, TV_ROD, SV_ROD_ILLUMINATION)) return GF_LITE;
    if (_object_is_(o_ptr, TV_ROD, SV_ROD_LITE)) return GF_LITE;
    if (_object_is_(o_ptr, TV_ROD, SV_ROD_SPEED)) return GF_INERT;
    if (_object_is_(o_ptr, TV_ROD, SV_ROD_SLOW_MONSTER)) return GF_INERT;
    if (_object_is_(o_ptr, TV_ROD, SV_ROD_PESTICIDE)) return GF_POIS;
    if (_object_is_(o_ptr, TV_ROD, SV_ROD_ACID_BOLT)) return GF_ACID;
    if (_object_is_(o_ptr, TV_ROD, SV_ROD_ACID_BALL)) return GF_ACID;
    if (_object_is_(o_ptr, TV_ROD, SV_ROD_ELEC_BOLT)) return GF_ELEC;
    if (_object_is_(o_ptr, TV_ROD, SV_ROD_ELEC_BALL)) return GF_ELEC;
    if (_object_is_(o_ptr, TV_ROD, SV_ROD_FIRE_BOLT)) return GF_FIRE;
    if (_object_is_(o_ptr, TV_ROD, SV_ROD_FIRE_BALL)) return GF_FIRE;
    if (_object_is_(o_ptr, TV_ROD, SV_ROD_COLD_BOLT)) return GF_COLD;
    if (_object_is_(o_ptr, TV_ROD, SV_ROD_COLD_BALL)) return GF_COLD;
    return GF_MANA;
}

static void _shatter_device_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Shatter Device", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Destroy a magical device in your inventory for various effects.", ""));
        break;
    case SPELL_CAST:
    {
        int item;
        object_type *o_ptr;
        
        var_set_bool(res, FALSE);
        item_tester_hook = item_tester_hook_recharge;
        if (!get_item(&item, "Shatter which device?", "You have nothing to shatter.", USE_INVEN)) return;
        o_ptr = &inventory[item];
        var_set_bool(res, TRUE);
        
        if (_object_is_(o_ptr, TV_STAFF, SV_STAFF_DESTRUCTION))
        {
            if (destroy_area(py, px, 15 + p_ptr->lev + randint0(11), 4 * p_ptr->lev))
                msg_print("The dungeon collapses...");
            else
                msg_print("The dungeon trembles.");
        }
        else if ( _object_is_(o_ptr, TV_STAFF, SV_STAFF_HEALING)
               || _object_is_(o_ptr, TV_ROD, SV_ROD_HEALING)
               || _object_is_(o_ptr, TV_ROD, SV_ROD_RESTORATION) )
        {
            msg_print("You feel life flow through your body!");
            restore_level();
            (void)set_poisoned(0, TRUE);
            (void)set_blind(0, TRUE);
            (void)set_confused(0, TRUE);
            (void)set_image(0, TRUE);
            (void)set_stun(0, TRUE);
            (void)set_cut(0, TRUE);
            (void)do_res_stat(A_STR);
            (void)do_res_stat(A_CON);
            (void)do_res_stat(A_DEX);
            (void)do_res_stat(A_WIS);
            (void)do_res_stat(A_INT);
            (void)do_res_stat(A_CHR);
            update_stuff();
            hp_player(5000);
        }
        else if (_object_is_(o_ptr, TV_ROD, SV_ROD_TELEPORT_AWAY))
        {
            banish_monsters(p_ptr->lev * 4);
        }
        else
        {
            project(0, 5, py, px, 
                k_info[o_ptr->k_idx].level * 16, 
                _object_dam_type(o_ptr), 
                PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL, -1);
        }
        inven_item_increase(item, -1);
        inven_item_describe(item);
        inven_item_optimize(item);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _shout_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Shout", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Projects a cone of sound at a chosen foe.", ""));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(3 + (p_ptr->lev-1)/5, 4, 0));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_aim_dir(&dir)) return;
        fire_ball(GF_SOUND, dir, damroll(3 + (p_ptr->lev-1)/5, 4), -2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _smash_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Smash", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Destroys adjacent door, trap or wall.", ""));
        break;
    case SPELL_CAST:
    {
        int y, x, dir;
        
        var_set_bool(res, FALSE);
        if (!get_rep_dir2(&dir)) return;
        if (dir == 5) return;

        y = py + ddy[dir];
        x = px + ddx[dir];
        
        if (!in_bounds(y, x)) return;

        if (cave_have_flag_bold(y, x, FF_HURT_ROCK))
        {
            cave_alter_feat(y, x, FF_HURT_ROCK);
            p_ptr->update |= PU_FLOW;
        }
        else
        {
            int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
            project(0, 0, y, x, 0, GF_KILL_DOOR, flg, -1);
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _spell_reaction_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Spell Reaction", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Grants temporary speed whenever you are targetted by a magical attack.", ""));
        break;
    case SPELL_CAST:
        set_tim_spell_reaction(30 + randint1(30), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _spell_turning_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Spell Turning", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Whenever you are the target of magic there is a chance of returning the spell to the caster.", ""));
        break;
    case SPELL_CAST:
        set_tim_spell_turning(20 + randint1(20), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _summon_commando_team_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Summon Commando Team", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Summons Grand Master Mystics for assistance.", ""));
        break;
    case SPELL_CAST:
    {
        int num = 1 + randint1(2);
        int mode = PM_FORCE_PET;
        int i, x, y;

        var_set_bool(res, FALSE);

        if (p_ptr->shero)
            mode |= PM_HASTE;

        if (!target_set(TARGET_KILL)) return;
        x = target_col;
        y = target_row;
            
        for (i = 0; i < num; i++)
        {
            summon_named_creature(-1, y, x, MON_G_MASTER_MYS, mode);
        }                
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _summon_horde_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, T("Summon Horde", ""));
        break;
    case SPELL_DESC:
        var_set_string(res, T("Summons Warriors of the Dawn for assistance.", ""));
        break;
    case SPELL_CAST:
    {
        int num = 3 + randint1(3);
        int mode = PM_FORCE_PET;
        int i;

        if (p_ptr->shero)
            mode |= PM_HASTE;
            
        for (i = 0; i < num; i++)
        {
            summon_named_creature(-1, py, px, MON_DAWN, mode);
        }                
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _veterans_blessing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Veteran's Blessing");
        break;
    default:
        heroism_spell(cmd, res);
        break;
    }
}

static void _whirlwind_attack_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Whirlwind Attack");
        break;
    default:
        massacre_spell(cmd, res);
        break;
    }
}

/* The Rage Mage uses spellbooks to learn spells
   like other magic classes.  However, learning a 
   spell destroys the book, and casting a spell
   does not require the book (cf The Samurai).
   Rage is a class specific realm.
*/
#define _SPELLS_PER_BOOK 8

typedef struct {
    cptr name;
    spell_info spells[_SPELLS_PER_BOOK];
} book_t;

static book_t _books[4] = {
    { "Anger Management", 
        {{ 1,  2, 30, _shout_spell},
         { 2,  2, 25, _detect_magical_foes_spell},
         { 3,  3, 30, _smash_spell},
         { 5,  5, 25, _evasive_leap_spell},
         { 5,  5, 35, light_area_spell},
         { 7,  0, 50, _focus_rage_spell},
         { 8, 10, 50, _rage_sustenance_spell},
         {12,  6, 35, _veterans_blessing_spell}}
    },
    { "Northern Frights",
        {{15,  8, 45, _crude_mapping_spell},
         {18, 18, 50, _resist_disenchantment_spell},
         {20, 30, 55, awesome_blow_spell},
         {22, 15, 60, _spell_reaction_spell},
         {23, 21, 60, _greater_shout_spell},
         {25, 18, 60, _whirlwind_attack_spell},
         {27, 20, 55, _resist_curses_spell},
         {28, 23, 70, _detect_magic_spell}}
    },
    { "The Sound and the Fury",
        {{10, 12, 35, berserk_spell},
         {25, 16, 60, sterility_spell},
         {26, 20, 80, _barbaric_resistance_spell},
         {28, 22, 55, _summon_horde_spell},
         {32, 28, 75, _armor_of_fury_spell},
         {35, 55, 70, _force_brand_spell},
         {38, 30, 50, dispel_magic_spell},
         {40, 60, 85, _mana_clash_spell}}
    },
    { "Dire Ire",
        {{30, 25, 75, _barbarian_lore_spell},
         {32, 15, 65, earthquake_spell},
         {35,  0, 90, _greater_focus_rage_spell},
         {38, 60, 95, _spell_turning_spell},
         {40, 55, 80, _shatter_device_spell},
         {42, 40, 50, _summon_commando_team_spell},
         {43, 70, 80, _anti_magic_ray_spell},
         {47,  0, 80, _rage_strike_spell}}
    },
};
                                 
static int _spell_index(int book, int spell)
{
    return book * _SPELLS_PER_BOOK + spell;
}

static bool _is_spell_known(int book, int spell)
{
    int idx = _spell_index(book, spell);
    if (p_ptr->spell_learned1 & (1L << idx)) return TRUE;
    return FALSE;
}

static void _learn_spell(int book, int spell)
{
    int idx = _spell_index(book, spell);
    int i;
    
    p_ptr->spell_learned1 |= (1L << idx);

    /* Find the next open entry in "p_ptr->spell_order[]" */
    for (i = 0; i < 64; i++)
    {
        /* Stop at the first empty space */
        if (p_ptr->spell_order[i] == 99) break;
    }

    /* Add the spell to the known list */
    p_ptr->spell_order[i++] = spell;
    
    msg_format("You have learned the technique of %s.", get_spell_name(_books[book].spells[spell].fn));
}

static bool _gain_spell(int book)
{
    spell_info     spells[_SPELLS_PER_BOOK];
    int            indices[_SPELLS_PER_BOOK];
    int            which;
    int         ct = 0, i;

    /* Build a list of learnable spells.  Spells can only be
       learned once (no spell skills) and we only display spells
       if the user is of high enough level.  This is rather 
       different than how the system normally behaves, but why spoil
       the nature of future higher level spells to the player?
    */    
    for (i = 0; i < _SPELLS_PER_BOOK; i++)
    {
        spell_info *src = &_books[book].spells[i];
        
        if (!_is_spell_known(book, i) && src->level <= p_ptr->lev)
        {
            spell_info *dest = &spells[ct];
            
            dest->level = src->level;
            dest->cost = src->cost;
            dest->fail = calculate_fail_rate(
                src->level, 
                src->fail, 
                p_ptr->stat_ind[A_STR]
            );
            dest->fn = src->fn;
            indices[ct] = i;
            
            ct++;
        }
    }
    
    if (ct == 0)
    {
        msg_print("You may not learn any spells in that book.");
        return FALSE;
    }
    
    which = choose_spell(spells, ct, "rage", 1000);
    if (which >= 0 && which < ct)
    {
        _learn_spell(book, indices[which]);
        return TRUE;
    }
    
    return FALSE;
}

void rage_mage_gain_spell(void)
{
    int item;

    if (p_ptr->blind || no_lite())
    {
        msg_print("You cannot see!");
        return;
    }

    if (p_ptr->confused)
    {
        msg_print("You are too confused!");
        return;
    }
    
    if (!p_ptr->new_spells)
    {
        msg_print("You cannot learn any new techniques!");
        return;
    }

    msg_format("You can learn %d new technique%s.", p_ptr->new_spells, (p_ptr->new_spells == 1 ? "" : "s"));
    msg_print(NULL);

    item_tester_tval = TV_RAGE_BOOK;
    if (get_item(&item, "Study which book?", "You have no books that you can read.", USE_INVEN))
    {
        int book = inventory[item].sval;
        if (_gain_spell(book))
        {
            char o_name[MAX_NLEN];
            int old_amt = inventory[item].number;

            inventory[item].number = 1;
            object_desc(o_name, &inventory[item], 0);
            inventory[item].number = old_amt;
            
            msg_format("%^s is destroyed.", o_name);
            inven_item_increase(item, -1);
            inven_item_describe(item);
            inven_item_optimize(item);
            
            energy_use = 100;
        }
    }
}

void rage_mage_browse_spell(void)
{
    /* TODO: Perhaps browse should display contents of rage 
       spellbooks in inventory rather than already known spells? */
    do_cmd_spell_browse();
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "rage";
        me.weight = 1000;
        init = TRUE;
    }
    return &me;
}

static void _player_action(int energy_use)
{
    /* Unclear Mind */
    if (_unclear_mind)    /* Hack for Focus Rage spell to bypass sp loss for one action */
    {
        int loss;
        loss = p_ptr->csp/8 + p_ptr->lev/10 + 1;
        loss = loss * energy_use / 100; /* Prorata normal action energy */
    
        p_ptr->csp -= loss;
        if (p_ptr->csp < 0)
        {
            p_ptr->csp = 0;
            p_ptr->csp_frac = 0;
        }
        p_ptr->redraw |= PR_MANA;
    }
    else
        _unclear_mind = TRUE; /* Resume normal sp loss */
}

static void _calc_bonuses(void)
{
    p_ptr->spell_cap += 3;

    /* Squishy */
    p_ptr->to_a -= p_ptr->lev + 10;
    p_ptr->dis_to_a -= p_ptr->lev + 10;
    
    if (p_ptr->tim_resist_curses)
    {
        p_ptr->skills.sav += 20;
        if (p_ptr->shero)
            p_ptr->skills.sav += 20;
    }
}

static int _get_spells_imp(spell_info* spells, int max, int book)
{
    int ct = 0, i;
    for (i = 0; i < _SPELLS_PER_BOOK; i++)
    {
        spell_info *src, *dest;
        
        if (ct >= max) break;
        src = &_books[book].spells[i];
        
        if (_is_spell_known(book, i))
        {            
            dest = &spells[ct++];
            dest->level = src->level;
            dest->cost = src->cost;
            dest->fail = calculate_fail_rate(
                src->level, 
                src->fail, 
                p_ptr->stat_ind[A_STR]
            );
            dest->fn = src->fn;
        }
    }
    return ct;
}

static void _book_menu_fn(int cmd, int which, vptr cookie, variant *res)
{
    switch (cmd)
    {
    case MENU_TEXT:
        var_set_string(res, _books[which].name);
        break;
    default:
        default_menu(cmd, which, cookie, res);
    }
}

static int _get_spells(spell_info* spells, int max)
{
    int idx = -1;
    int ct = 0;
    menu_t menu = { "Use which group?", NULL, NULL,
                    _book_menu_fn, _books, 4 };
    
    idx = menu_choose(&menu);
    if (idx < 0) return 0;

    ct = _get_spells_imp(spells, max, idx);
    if (ct == 0)
        msg_print("You don't know any of those techniques yet!");
    return ct;
}

class_t *rage_mage_get_class_t(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 20,  24,  40,  -1,  12,   2,  50,  30 };
    skills_t xs = {  7,  10,  15,   0,   0,   0,  15,  15 };

        me.name = "Rage-Mage";
        me.desc = "The Rage Mage is part of a secret sect descending from the Barbarians "
                    "in response to their natural foes, the mages. As time passed, other "
                    "races have also begun to learn their arts. ";
        me.stats[A_STR] =  3;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] = -2;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  1;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 106;
        me.base_hp = 6;
        me.exp = 150;
        me.pets = 40;

        me.calc_bonuses = _calc_bonuses;
        me.get_spells = _get_spells;
        me.caster_info = _caster_info;
        me.player_action = _player_action;
        init = TRUE;
    }

    return &me;
}