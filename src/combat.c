#include "angband.h"
#include <assert.h>

int hit_chance_innate(int to_h, int ac)
{
    int chance = p_ptr->skills.thn + to_h * BTH_PLUS_ADJ;
    int odds;

    if (chance <= 0) return 0;
    
    odds = 95*(chance - ac*3/4)*1000/(chance*100);
    if (p_ptr->personality == PERS_LAZY) odds = (19*odds+10)/20;
    if (odds < 50) odds = 50;
    return (odds+5)/10;
}

int hit_chance(int hand, int to_h, int ac)
{
    int chance = p_ptr->skills.thn + (p_ptr->weapon_info[hand].to_h + to_h) * BTH_PLUS_ADJ;
    int odds;

    chance = chance * p_ptr->weapon_info[hand].dual_wield_pct / 1000;
    chance += virtue_current(VIRTUE_VALOUR) / 10;
    if (chance <= 0) return 0;

    odds = 95*(chance - ac*3/4)*1000/(chance*100);
    if (p_ptr->personality == PERS_LAZY) odds = (19*odds+10)/20;
    if (odds < 50) odds = 50;
    return (odds+5)/10;
}

int bow_hit_chance(int sval, int to_h, int ac)
{
    int chance;
    int odds;

    if (sval == SV_LIGHT_XBOW || sval == SV_HEAVY_XBOW)
        chance = (p_ptr->skills.thb + (p_ptr->weapon_exp[0][sval] / 400 + to_h) * BTH_PLUS_ADJ);
    else
        chance = (p_ptr->skills.thb + ((p_ptr->weapon_exp[0][sval] - (WEAPON_EXP_MASTER / 2)) / 200 + to_h) * BTH_PLUS_ADJ);

    if (chance <= 0) return 0;

    odds = 95*(chance - ac*3/4)*1000/(chance*100);
    if (p_ptr->personality == PERS_LAZY) odds = (19*odds+10)/20;
    if (odds < 50) odds = 50;
    return (odds+5)/10;
}

/**********************************************************************
 * Number of Blows
 **********************************************************************/
typedef struct { int num; int wgt; int mul; } _blow_info_t;

static _blow_info_t _get_blow_info(int hand)
{
    _blow_info_t result = {0};
    int          arm = hand / 2;
    object_type *o_ptr = equip_obj(p_ptr->weapon_info[hand].slot);

    if (!o_ptr) return result;

    /* TODO: Use race_ptr and class_ptr instead of this giant switch ... */
    switch (p_ptr->pclass)
    {
    case CLASS_WARRIOR:
        result.num = 600; result.wgt = 70; result.mul = 50 + p_ptr->lev/2;
        break;

    case CLASS_MAULER:
        result.num = 300; result.wgt = 280; result.mul = 125; break;

    case CLASS_BERSERKER:
        result.num = 600; result.wgt = 70; result.mul = 75; break;

    case CLASS_RAGE_MAGE:
        result.num = 300; result.wgt = 70; result.mul = 30; break;
                    
    case CLASS_MAGE:
    case CLASS_NECROMANCER:
    case CLASS_BLOOD_MAGE:
    case CLASS_HIGH_MAGE:
    case CLASS_BLUE_MAGE:
        result.num = 400; result.wgt = 100; result.mul = 20; break;

    case CLASS_WARLOCK:
        result.num = 400; result.wgt = 100; result.mul = 35;
        switch (p_ptr->psubclass)
        {
        case WARLOCK_DRAGONS:
            if ( p_ptr->riding
              && (object_is_(o_ptr, TV_POLEARM, SV_LANCE) || object_is_(o_ptr, TV_POLEARM, SV_HEAVY_LANCE)) )
            {
                result.mul = 65;
            }
            break;
        case WARLOCK_ANGELS:
        case WARLOCK_DEMONS:
            result.num = 450;
            break;
        case WARLOCK_HOUNDS:
            result.num = 475;
            break;
        case WARLOCK_GIANTS:
            result.wgt = 200;
            result.mul = 50 + p_ptr->lev/5;
            result.num = 500;
            break;
        }
        break;

    case CLASS_PSION:
        result.num = 400; result.wgt = 100; result.mul = 30; break;

    case CLASS_PRIEST:
    case CLASS_MAGIC_EATER:
    case CLASS_MINDCRAFTER:
        result.num = 500; result.wgt = 100; result.mul = 35; break;

    case CLASS_DEVICEMASTER:
        result.num = 400; result.wgt = 100; result.mul = 35;
        if (p_ptr->psubclass == DEVICEMASTER_POTIONS || p_ptr->psubclass == DEVICEMASTER_SCROLLS)
            result.num = 500;
        break;

    case CLASS_ROGUE:
        result.num = 525; result.wgt = 40; result.mul = 30;
        if (o_ptr->weight < 50) result.num = 600;
        break;

    case CLASS_SCOUT:
        result.num = 400; result.wgt = 70; result.mul = 25; break;

    case CLASS_RANGER:
        result.num = 500; result.wgt = 70; result.mul = 40; break;

    case CLASS_PALADIN:
    case CLASS_SAMURAI:
        result.num = 550; result.wgt = 70; result.mul = 45; break;

    case CLASS_MYSTIC:
        result.num = 100; result.wgt = 100; result.mul = 10; break;

    case CLASS_WEAPONSMITH:
    case CLASS_RUNE_KNIGHT:
        result.num = 525; result.wgt = 150; result.mul = 55; break;

    case CLASS_WEAPONMASTER:
        result.num = weaponmaster_get_max_blows(o_ptr, hand); 
        result.wgt = 70; result.mul = 50; break;

    case CLASS_WARRIOR_MAGE:
    case CLASS_RED_MAGE:
        result.num = 525; result.wgt = 70; result.mul = 30; break;

    case CLASS_CHAOS_WARRIOR:
        result.num = 550; result.wgt = 70; result.mul = 45; break;

    case CLASS_MONK:
        result.num = 500; result.wgt = 60; result.mul = 30; break;

    case CLASS_TOURIST:
    case CLASS_TIME_LORD:
        result.num = 400; result.wgt = 100; result.mul = 30; break;

    case CLASS_ARCHAEOLOGIST:
        result.num = 400; result.wgt = 70; result.mul = 30;
        if (archaeologist_is_favored_weapon(o_ptr))
        {
            result.num = 500;
            result.mul = 40;
        }
        break;

    case CLASS_BLOOD_KNIGHT:
        result.num = 300; result.wgt = 150; result.mul = 30; break;

    case CLASS_DUELIST:
        result.num = 100; result.wgt = 70; result.mul = 40; break;
                    
    case CLASS_IMITATOR:
        result.num = 550; result.wgt = 70; result.mul = 40; break;

    case CLASS_WILD_TALENT:
        result.num = 450; result.wgt = 70; result.mul = 40; break;

    case CLASS_BEASTMASTER:
        result.num = 500; result.wgt = 70; result.mul = 35; break;

    case CLASS_CAVALRY:
    {
        u32b flgs[OF_ARRAY_SIZE];
        obj_flags(o_ptr, flgs);
        if (p_ptr->riding && have_flag(flgs, OF_RIDING)) {result.num = 550; result.wgt = 70; result.mul = 65;}
        else {result.num = 500; result.wgt = 100; result.mul = 35;}
        break;
    }
    case CLASS_SORCERER:
        result.num = 100; result.wgt = 1; result.mul = 10; break;

    case CLASS_ARCHER:
    case CLASS_BARD:
        result.num = 450; result.wgt = 70; result.mul = 20; break;

    case CLASS_FORCETRAINER:
        result.num = 400; result.wgt = 60; result.mul = 20; break;

    case CLASS_MIRROR_MASTER:
    case CLASS_SNIPER:
        result.num = 400; result.wgt = 100; result.mul = 30; break;

    case CLASS_NINJA:
        result.num = 425; result.wgt = 20; result.mul = 10; break;

    case CLASS_MONSTER:
        result.num = 500; result.wgt = 70; result.mul = 50;
        if (prace_is_(RACE_MON_LICH)) 
        {
            result.num = 400;
            result.mul = 30;
        }
        else if (prace_is_(RACE_MON_POSSESSOR))
        {
            result.num = 400;
        }
        else if (prace_is_(RACE_MON_MIMIC)) 
        {
            result.num = 400;
        }
        else if (prace_is_(RACE_MON_TROLL))
        {
            result.num = 550;
        }
        else if (prace_is_(RACE_MON_GIANT))
        {
            result.num = 550;
            result.mul = 50 + p_ptr->lev/5;
            result.wgt = 200;
            if (giant_is_(GIANT_HRU) && p_ptr->lev >= 40)
                result.mul = 80;
        }
        else if ( prace_is_(RACE_MON_JELLY)
               || demon_is_(DEMON_KHORNE) )  
        {
            result.num = 600;
            result.mul = 50 + p_ptr->lev/5;
        }
        else if (prace_is_(RACE_MON_LEPRECHAUN)) 
        {
            result.num = 300;
            result.mul = 20;
        }
        else if (prace_is_(RACE_MON_SWORD))
        {
            result.num = 525;
            if (p_ptr->lev >= 45) /* Death Scythes retaliate! */
                result.num = 300;
        }
        else if (prace_is_(RACE_MON_GOLEM))
        {
            result.num = 100;
        }
        break;
    }

    if (hex_spelling(HEX_XTRA_MIGHT) || hex_spelling(HEX_BUILDING) || p_ptr->tim_building_up)
    {
        result.wgt /= 2;
        result.mul += 20;
    }

    /* Xorns and Mariliths have multiple sets of arms */
    if (arm > 0)
        result.num -= 100;
    if (result.num < 100)
        result.num = 100;

    if (o_ptr->tval == TV_SWORD && o_ptr->sval == SV_POISON_NEEDLE) 
        result.num = 100;

    return result;
}

/* New Blows Calc
   Dex (old dex_index) determines the min/max range of the number of blows (scaled by 100).
   Str*Mul/Div (old str_index) interpolates this range linearly (110 steps)
   We now support fractional blows (e.g. 2.75 is stored as 275 and gives 3 blows 75% of the time)

   This is not exactly the same as before, but I think the differences are minor enough.
*/
typedef struct {
    int min;
    int max;
} _range_t;

static _range_t _blows_range[] =
{
    {   0, 200}       /* 3 */,
    {   0, 200}       /* 4 */,
    {  10, 200}       /* 5 */,
    {  20, 210}       /* 6 */,
    {  30, 220}       /* 7 */,
    {  40, 230}       /* 8 */,
    {  50, 240}       /* 9 */,
    {  60, 250}       /* 10 */,
    {  70, 260}       /* 11 */,
    {  80, 270}       /* 12 */,
    {  90, 280}       /* 13 */,
    { 100, 290}       /* 14 */,
    { 110, 300}       /* 15 */,
    { 120, 350}       /* 16 */,
    { 130, 400}       /* 17 */,
    { 140, 450}       /* 18/00-18/09 */,
    { 150, 460}       /* 18/10-18/19 */,
    { 160, 470}       /* 18/20-18/29 */,
    { 170, 480}       /* 18/30-18/39 */,
    { 180, 490}       /* 18/40-18/49 */,
    { 190, 500}       /* 18/50-18/59 */,
    { 200, 520}       /* 18/60-18/69 */,
    { 210, 540}       /* 18/70-18/79 */,
    { 220, 560}       /* 18/80-18/89 */,
    { 230, 580}       /* 18/90-18/99 */,
    { 240, 600}       /* 18/100-18/109 */,
    { 250, 610}       /* 18/110-18/119 */,
    { 260, 620}       /* 18/120-18/129 */,
    { 280, 630}       /* 18/130-18/139 */,
    { 300, 640}       /* 18/140-18/149 */,
    { 320, 650}       /* 18/150-18/159 */,
    { 340, 660}       /* 18/160-18/169 */,
    { 350, 670}       /* 18/170-18/179 */,
    { 360, 680}       /* 18/180-18/189 */,
    { 370, 690}       /* 18/190-18/199 */,
    { 380, 700}       /* 18/200-18/209 */,
    { 390, 725}       /* 18/210-18/219 */,
    { 400, 750}       /* 18/220+ */
};

int calculate_base_blows(int hand, int str_idx, int dex_idx)
{
    int            result = 0;
    weapon_info_t *info_ptr = &p_ptr->weapon_info[hand];
    object_type   *o_ptr = NULL;
    int            blow_str_idx;
    int            div = 0;
    int            mul = 0;
    int            wgt = 0;
    _blow_info_t   blow_info = {0};
    _range_t       rng = _blows_range[dex_idx];

    if (info_ptr->wield_how == WIELD_NONE) return 0;

    o_ptr = equip_obj(info_ptr->slot);
    if (!o_ptr) return 0;
    if (info_ptr->heavy_wield) return 100;

    blow_info = _get_blow_info(hand);

    wgt = o_ptr->weight;
    div = (wgt < blow_info.wgt) ? blow_info.wgt : wgt;
    mul = blow_info.mul + info_ptr->giant_wield * 10;

    blow_str_idx = adj_str_blow[str_idx] * mul / div; /* Scaled by 10 */
    if (info_ptr->wield_how == WIELD_TWO_HANDS)
    {
        if (!info_ptr->omoi)
            blow_str_idx += 10;
        if (prace_is_(RACE_MON_GIANT) && giant_is_favorite(o_ptr)) 
            blow_str_idx += 10;
    }
    if (p_ptr->pclass == CLASS_NINJA) blow_str_idx = MAX(0, blow_str_idx-10);
    if (blow_str_idx > 110) blow_str_idx = 110;

    result = rng.min + (rng.max - rng.min) * blow_str_idx / 110;

    if (p_ptr->pclass == CLASS_MAULER)
        result = 100 + (result - 100)*2/5;
        
    if (result > blow_info.num) 
        result = blow_info.num;

    if (result < 100)
        result = 100;

    return result;
}

static int _calc_innate_blows_aux(innate_attack_ptr a, int max, int str_idx, int dex_idx)
{
    int      result = 0;
    int      blow_str_idx;
    int      mul = 55, div = MAX(70, a->weight);
    _range_t rng = _blows_range[dex_idx];

    blow_str_idx = (adj_str_blow[str_idx] * mul / div);
    if (blow_str_idx > 110) blow_str_idx = 110;

    result = rng.min + (rng.max - rng.min) * blow_str_idx / 110;

    if (prace_is_(RACE_MON_LEPRECHAUN))
        result /= 2;

    if (prace_is_(RACE_MON_HYDRA))
        result *= 2;

    if (result < 100)
        result = 100;
    if (result > max)
        result = max;

    return result;
}

void calc_innate_blows(innate_attack_ptr a, int max)
{
    a->blows = _calc_innate_blows_aux(a, max, p_ptr->stat_ind[A_STR], p_ptr->stat_ind[A_DEX]);
}

/**********************************************************************
 * Display Weapon Information to the Player
 **********************************************************************/
static void _display_weapon_slay(int base_mult, int slay_mult, bool force, int blows, 
                                 int dd, int ds, int to_d, cptr name, int color, doc_ptr doc)
{
    int mult, min, max;

    mult = slay_mult;
    if (force)
        mult = mult * 3/2 + 150;
    mult = mult * base_mult / 100;

    min = blows * (mult*dd/100 + to_d) / 100;
    max = blows * (mult*dd*ds/100 + to_d) / 100;
    if (weaponmaster_get_toggle() == TOGGLE_ORDER_BLADE)
        min = max;

    doc_printf(doc, "<color:%c> %-7.7s</color>", attr_to_attr_char(color), name);
    doc_printf(doc, ": %d [%d.%02dx]\n",
                    (min + max)/2,
                    mult/100, mult%100);
}

int display_weapon_mode = 0;

void display_weapon_info(doc_ptr doc, int hand)
{
    object_type *o_ptr = equip_obj(p_ptr->weapon_info[hand].slot);
    char o_name[MAX_NLEN];
    u32b flgs[OF_ARRAY_SIZE];
    int dd;
    int ds;
    int to_d = 0;
    int to_h = 0;
    int mult;
    critical_t crit = {0};
    int crit_pct = 0;
    int num_blow = NUM_BLOWS(hand);
    bool force = FALSE;
    doc_ptr cols[2] = {0};

    if (p_ptr->weapon_info[hand].wield_how == WIELD_NONE) return;
    if (!o_ptr) return;

    dd = o_ptr->dd + p_ptr->weapon_info[hand].to_dd;
    ds = o_ptr->ds + p_ptr->weapon_info[hand].to_ds;
    if (object_is_known(o_ptr))
    {
        to_d = o_ptr->to_d;
        to_h = o_ptr->to_h;
    }

    switch (display_weapon_mode)
    {
    case MAULER_CRITICAL_BLOW:
        if (num_blow > 100)
            num_blow = 100 + (num_blow - 100) / 2;
        break;
    case MAULER_STUNNING_BLOW:
    case MAULER_CRUSHING_BLOW:
    case MAULER_KNOCKBACK:
        num_blow = 100;
        break;
    case PY_POWER_ATTACK:
        to_h += 10;
        to_d += p_ptr->lev / 2;
        break;
    }

    weapon_flags_known(hand, flgs);
    if ( (have_flag(flgs, OF_BRAND_MANA) || p_ptr->tim_force) 
      && (p_ptr->csp > o_ptr->dd*o_ptr->ds/5) )
    {
        force = TRUE;
    }

    if (weaponmaster_get_toggle() == TOGGLE_SHIELD_BASH && object_is_shield(o_ptr))
    {
        dd = 3 + p_ptr->weapon_info[hand].to_dd;
        ds = o_ptr->ac + p_ptr->weapon_info[hand].to_ds;
        if (object_is_known(o_ptr))
        {
            to_h = o_ptr->to_a;
            to_d = o_ptr->to_a;
            to_h += 2*o_ptr->to_h;
            to_d += 2*o_ptr->to_d;
        }
    }

    mult = 100;
    if (have_flag(flgs, OF_VORPAL2))
        mult = mult * 5 / 3;  /* 1 + 1/3(1 + 1/2 + ...) = 1.667x */
    else if (have_flag(flgs, OF_VORPAL) && p_ptr->vorpal)
        mult = mult * 11 / 8; /* 1 + 1/4(1 + 1/3 + ...) = 1.375x */
    else if (have_flag(flgs, OF_VORPAL) || p_ptr->vorpal)
        mult = mult * 11 / 9; /* 1 + 1/6(1 + 1/4 + ...) = 1.222x */

    if (display_weapon_mode == MAULER_CRUSHING_BLOW)
        mult = mult * NUM_BLOWS(hand) / 50;

    if (!have_flag(flgs, OF_BRAND_ORDER)
        && weaponmaster_get_toggle() != TOGGLE_ORDER_BLADE)
    {
        const int attempts = 10 * 1000;
        int i;
        int crits = 0;
        /* Compute Average Effects of Criticals by sampling */
        for (i = 0; i < attempts; i++)
        {
            critical_t tmp = critical_norm(o_ptr->weight, to_h, p_ptr->weapon_info[hand].to_h, display_weapon_mode, hand);
            if (tmp.desc)
            {
                crit.mul += tmp.mul;
                crit.to_d += tmp.to_d;
                crits++;
            }
            else
                crit.mul += 100;
        }
        crit.mul = crit.mul / attempts;
        crit.to_d = crit.to_d * 100 / attempts;
        crit_pct = crits * 1000 / attempts;
    }
    else
        crit.mul = 100;


    /* Display in 2 columns, side by side */
    cols[0] = doc_alloc(60);
    cols[1] = doc_alloc(10);

    /* Column #1 */
    object_desc(o_name, o_ptr, OD_COLOR_CODED | OD_NAME_AND_ENCHANT);
    if (prace_is_(RACE_MON_SWORD))
        doc_printf(cols[0], "<color:y> You    :</color> <indent><style:indent>%s</style></indent>\n", o_name);
    else
        doc_printf(cols[0], "<color:y> Hand #%d:</color> <indent><style:indent>%s</style></indent>\n", hand+1, o_name);

    doc_printf(cols[0], " %-7.7s: %d.%d lbs\n", "Weight", o_ptr->weight/10, o_ptr->weight%10);

    if (weaponmaster_get_toggle() == TOGGLE_SHIELD_BASH)
    {
        assert(o_ptr->tval == TV_SHIELD);
        doc_printf(cols[0], " %-7.7s: %dd%d (%+d,%+d)\n", "Bash", dd, ds, to_h, to_d);
        doc_printf(cols[0], " %-7.7s: %s (%+d To Hit)\n",
                    "Profic",
                    skills_shield_describe_current(o_ptr->sval),
                    skills_shield_calc_bonus(o_ptr->sval));
    }
    else
    {
        doc_printf(cols[0], " %-7.7s: %s (%+d To Hit)\n",
                    "Profic",
                    skills_weapon_describe_current(o_ptr->tval, o_ptr->sval),
                    skills_weapon_calc_bonus(o_ptr->tval, o_ptr->sval));
    }
    doc_printf(cols[0], " %-7.7s: %d + %d = %d\n", "To Hit", to_h, p_ptr->weapon_info[hand].to_h, to_h + p_ptr->weapon_info[hand].to_h);
    doc_printf(cols[0], " %-7.7s: %d + %d = %d\n", "To Dam", to_d, p_ptr->weapon_info[hand].to_d, to_d + p_ptr->weapon_info[hand].to_d);
    doc_printf(cols[0], " %-7.7s: %d.%2.2d\n", "Blows", num_blow/100, num_blow%100);

    if (p_ptr->weapon_info[hand].dual_wield_pct < 1000)
    {
        doc_printf(cols[0], " %-7.7s: %d.%d%%\n", "Skill",
            p_ptr->weapon_info[hand].dual_wield_pct/ 10,
            p_ptr->weapon_info[hand].dual_wield_pct % 10);
    }

    mult = mult * crit.mul / 100;
    to_d = to_d + crit.to_d/100 + p_ptr->weapon_info[hand].to_d;

    doc_printf(cols[0], "<color:G> %-7.7s</color>\n", "Damage");

    if (!have_flag(flgs, OF_BRAND_ORDER)
        && weaponmaster_get_toggle() != TOGGLE_ORDER_BLADE)
    {
        if (crit.to_d)
        {
            doc_printf(cols[0], " %-7.7s: %d.%02dx + %d.%02d\n", "Crits",
                            crit.mul/100, crit.mul%100, crit.to_d/100, crit.to_d%100);
        }
        else
        {
            doc_printf(cols[0], " %-7.7s: %d.%02dx (%d.%d%%)\n", "Crits",
                            crit.mul/100, crit.mul%100, crit_pct / 10, crit_pct % 10);
        }
    }

    _display_weapon_slay(mult, 100, FALSE, num_blow, dd, ds, to_d, "Normal", TERM_WHITE, cols[0]);
    if (force)
        _display_weapon_slay(mult, 100, force, num_blow, dd, ds, to_d, "Force", TERM_L_BLUE, cols[0]);

    if (p_ptr->tim_slay_sentient)
        _display_weapon_slay(mult, 200, force, num_blow, dd, ds, to_d, "Sent.", TERM_YELLOW, cols[0]);
    
    if (have_flag(flgs, OF_KILL_ANIMAL)) 
        _display_weapon_slay(mult, 400, force, num_blow, dd, ds, to_d, "Animals", TERM_YELLOW, cols[0]);
    else if (have_flag(flgs, OF_SLAY_ANIMAL))
        _display_weapon_slay(mult, 250, force, num_blow, dd, ds, to_d, "Animals", TERM_YELLOW, cols[0]);
    
    if (have_flag(flgs, OF_KILL_EVIL))   
        _display_weapon_slay(mult, 350, force, num_blow, dd, ds, to_d, "Evil", TERM_YELLOW, cols[0]);
    else if (have_flag(flgs, OF_SLAY_EVIL) || weaponmaster_get_toggle() == TOGGLE_HOLY_BLADE)
        _display_weapon_slay(mult, 200, force, num_blow, dd, ds, to_d, "Evil", TERM_YELLOW, cols[0]);

    if (have_flag(flgs, OF_SLAY_GOOD))
        _display_weapon_slay(mult, 200, force, num_blow, dd, ds, to_d, "Good", TERM_YELLOW, cols[0]);

    if (have_flag(flgs, OF_SLAY_LIVING))
        _display_weapon_slay(mult, 200, force, num_blow, dd, ds, to_d, "Living", TERM_YELLOW, cols[0]);

    if (have_flag(flgs, OF_KILL_HUMAN))
        _display_weapon_slay(mult, 400, force, num_blow, dd, ds, to_d, "Human", TERM_YELLOW, cols[0]);
    else if (have_flag(flgs, OF_SLAY_HUMAN))
        _display_weapon_slay(mult, 250, force, num_blow, dd, ds, to_d, "Human", TERM_YELLOW, cols[0]);

    if (have_flag(flgs, OF_KILL_UNDEAD))
        _display_weapon_slay(mult, 500, force, num_blow, dd, ds, to_d, "Undead", TERM_YELLOW, cols[0]);
    else if (have_flag(flgs, OF_SLAY_UNDEAD))
        _display_weapon_slay(mult, 300, force, num_blow, dd, ds, to_d, "Undead", TERM_YELLOW, cols[0]);
    
    if (have_flag(flgs, OF_KILL_DEMON))  
        _display_weapon_slay(mult, 500, force, num_blow, dd, ds, to_d, "Demons", TERM_YELLOW, cols[0]);
    else if (have_flag(flgs, OF_SLAY_DEMON))
        _display_weapon_slay(mult, 300, force, num_blow, dd, ds, to_d, "Demons", TERM_YELLOW, cols[0]);

    if (have_flag(flgs, OF_KILL_ORC))  
        _display_weapon_slay(mult, 500, force, num_blow, dd, ds, to_d, "Orcs", TERM_YELLOW, cols[0]);
    else if (have_flag(flgs, OF_SLAY_ORC))
        _display_weapon_slay(mult, 300, force, num_blow, dd, ds, to_d, "Orcs", TERM_YELLOW, cols[0]);

    if (have_flag(flgs, OF_KILL_TROLL))  
        _display_weapon_slay(mult, 500, force, num_blow, dd, ds, to_d, "Trolls", TERM_YELLOW, cols[0]);
    else if (have_flag(flgs, OF_SLAY_TROLL))
        _display_weapon_slay(mult, 300, force, num_blow, dd, ds, to_d, "Trolls", TERM_YELLOW, cols[0]);

    if (have_flag(flgs, OF_KILL_GIANT))  
        _display_weapon_slay(mult, 500, force, num_blow, dd, ds, to_d, "Giants", TERM_YELLOW, cols[0]);
    else if (have_flag(flgs, OF_SLAY_GIANT))
        _display_weapon_slay(mult, 300, force, num_blow, dd, ds, to_d, "Giants", TERM_YELLOW, cols[0]);

    if (have_flag(flgs, OF_KILL_DRAGON))  
        _display_weapon_slay(mult, 500, force, num_blow, dd, ds, to_d, "Dragons", TERM_YELLOW, cols[0]);
    else if (have_flag(flgs, OF_SLAY_DRAGON))
        _display_weapon_slay(mult, 300, force, num_blow, dd, ds, to_d, "Dragons", TERM_YELLOW, cols[0]);
    
    if (have_flag(flgs, OF_BRAND_ACID))
        _display_weapon_slay(mult, 250, force, num_blow, dd, ds, to_d, "Acid", TERM_RED, cols[0]);

    if (have_flag(flgs, OF_BRAND_ELEC))
        _display_weapon_slay(mult, 250, force, num_blow, dd, ds, to_d, "Elec", TERM_RED, cols[0]);

    if (have_flag(flgs, OF_BRAND_FIRE))
        _display_weapon_slay(mult, 250, force, num_blow, dd, ds, to_d, "Fire", TERM_RED, cols[0]);

    if (have_flag(flgs, OF_BRAND_COLD))
        _display_weapon_slay(mult, 250, force, num_blow, dd, ds, to_d, "Cold", TERM_RED, cols[0]);

    if (have_flag(flgs, OF_BRAND_POIS))
        _display_weapon_slay(mult, 250, force, num_blow, dd, ds, to_d, "Poison", TERM_RED, cols[0]);

    if (p_ptr->weapon_info[hand].wield_how == WIELD_TWO_HANDS)
    {
        if (p_ptr->weapon_info[hand].omoi)
            doc_insert(cols[0], " Your weapon requires two hands to wield properly.\n");
    }

    if (p_ptr->weapon_info[hand].info)
    {
        byte a = p_ptr->weapon_info[hand].info_attr;
        if (!a) a = TERM_WHITE; /* uninitialized is TERM_DARK???! */
        doc_printf(cols[0], " <color:%c>%s</color>\n", attr_to_attr_char(a), p_ptr->weapon_info[hand].info);
    }

    /* Column #1 */
    doc_insert(cols[1], "<color:G>Accuracy</color>\n");
    doc_insert(cols[1], " AC Hit\n");

    doc_printf(cols[1], "%3d %2d%%\n", 25, hit_chance(hand, to_h, 25));
    doc_printf(cols[1], "%3d %2d%%\n", 50, hit_chance(hand, to_h, 50));
    doc_printf(cols[1], "%3d %2d%%\n", 75, hit_chance(hand, to_h, 75));
    doc_printf(cols[1], "%3d %2d%%\n", 100, hit_chance(hand, to_h, 100));
    doc_printf(cols[1], "%3d %2d%%\n", 125, hit_chance(hand, to_h, 125));
    doc_printf(cols[1], "%3d %2d%%\n", 150, hit_chance(hand, to_h, 150));
    doc_printf(cols[1], "%3d %2d%%\n", 175, hit_chance(hand, to_h, 175));
    doc_printf(cols[1], "%3d %2d%%\n", 200, hit_chance(hand, to_h, 200));

    /* Assemble the result */
    doc_insert_cols(doc, cols, 2, 1);
    doc_free(cols[0]);
    doc_free(cols[1]);
}

/**********************************************************************
 * Innate Attacks
 **********************************************************************/
static cptr _effect_name(int which)
{
    switch (which)
    {
    case 0: case GF_MISSILE: return "Normal";
    case GF_ACID: return "Acid";
    case GF_ELEC: return "Elec";
    case GF_FIRE: return "Fire";
    case GF_COLD: return "Cold";
    case GF_POIS: return "Poison";
    case GF_NETHER: return "Nether";
    case GF_SHARDS: return "Shards";
    case GF_DISENCHANT: return "Disench";
    case GF_TIME: return "Time";
    case GF_OLD_DRAIN: return "Drain";
    case GF_OLD_CONF: return "Confuse";
    case GF_CONFUSION: return "Confuse";
    case GF_STUN: return "Stun";
    case GF_DRAIN_MANA: return "Drain Mana";
    case GF_TURN_ALL: return "Terrifies";
    }
    return "Unknown";
}

void display_innate_attack_info(doc_ptr doc, int which)
{
    innate_attack_ptr a = &p_ptr->innate_attacks[which];
    int blows, min, max, min_base, max_base, min2, max2;
    int i;
    int to_h = p_ptr->to_h_m + a->to_h;
    int to_d = p_ptr->to_d_m + a->to_d;
    int dd = a->dd + p_ptr->innate_attack_info.to_dd;
    int mult;
    doc_ptr cols[2] = {0};

    blows = a->blows;
    if (which == 0)
        blows += p_ptr->innate_attack_info.xtra_blow;

    cols[0] = doc_alloc(60);
    cols[1] = doc_alloc(10);

    /* First Column */
    if (a->flags & INNATE_NO_DAM)
        doc_printf(cols[0], "<color:y> %-7.7s</color>: Your %s\n", "Attack", a->name);
    else
        doc_printf(cols[0], "<color:y> %-7.7s</color>: Your %s (%dd%d)\n", "Attack", a->name, dd, a->ds);

    if (a->weight && !(a->flags & INNATE_NO_DAM))
        doc_printf(cols[0], " %-7.7s: %d.%d lbs\n", "Weight", a->weight/10, a->weight%10);

    {
        cptr name = skills_innate_calc_name(a);
        doc_printf(cols[0], " %-7.7s: %s (%+d To Hit)\n",
                    "Profic",
                    skills_innate_describe_current(name),
                    skills_innate_calc_bonus(name));
    }

    doc_printf(cols[0], " %-7.7s: %d + %d = %d\n", "To Hit", a->to_h, p_ptr->to_h_m, to_h);

    if (!(a->flags & INNATE_NO_DAM))
        doc_printf(cols[0], " %-7.7s: %d + %d = %d\n", "To Dam", a->to_d, p_ptr->to_d_m, to_d);

    doc_printf(cols[0], " %-7.7s: %d.%2.2d\n", "Blows", blows/100, blows%100);

    mult = 100;

    if (!(a->flags & INNATE_NO_DAM))
    {
        doc_printf(cols[0], "<color:G> %-7.7s</color>\n", "Damage");
        if (a->flags & INNATE_VORPAL)
        {
            mult = mult * 11 / 9;
            doc_printf(cols[0], " %-7.7s: %d.%02dx\n", "Vorpal", mult/100, mult%100);
        }
    }

    if (!(a->flags & INNATE_NO_DAM))
    {
        critical_t crit = {0};
        const int ct = 10 * 1000;
        for (i = 0; i < ct; i++)
        {
            critical_t tmp = critical_norm(a->weight, to_h, 0, 0, HAND_NONE);
            if (tmp.desc)
            {
                crit.mul += tmp.mul;
                crit.to_d += tmp.to_d;
            }
            else
                crit.mul += 100;
        }
        crit.mul = crit.mul / ct;
        crit.to_d = crit.to_d * 100 / ct;
        if (crit.to_d)
            doc_printf(cols[0], " %-7.7s: %d.%02dx + %d.%02d\n", "Crits", crit.mul/100, crit.mul%100, crit.to_d/100, crit.to_d%100);
        else
            doc_printf(cols[0], " %-7.7s: %d.%02dx\n", "Crits", crit.mul/100, crit.mul%100);
        crit.to_d /= 100;
        mult = mult * crit.mul / 100;
        to_d = to_d + crit.to_d;
    }

    min_base = mult * dd / 100;
    min = min_base + to_d;
    min2 = 2*(min_base + a->to_d) + p_ptr->to_d_m;
    max_base = mult * dd * a->ds / 100;
    max = max_base + to_d;
    max2 = 2*(max_base + a->to_d) + p_ptr->to_d_m;
    
    if (a->effect[0] == GF_OLD_CONF) /* Hack for Umber Hulk ... */
    {
        doc_insert(cols[0], "<tab:10><color:B>Confuses</color>\n");
    }
    else if (!(a->flags & INNATE_NO_DAM))
    {
        doc_printf(cols[0], " %-7.7s: %d\n",_effect_name(a->effect[0]), blows * (min + max)/200);
    }

    for (i = 1; i < MAX_INNATE_EFFECTS; i++)
    {
        int p = a->effect_chance[i];
        char xtra[255];
        if (!a->effect[i]) continue;
        if (!p)
            sprintf(xtra, "%s", "");
        else
            sprintf(xtra, " (%d%%)", p);

        switch (a->effect[i])
        {
        case GF_STEAL: 
            doc_printf(cols[0], "<tab:10><color:B>Steals%s</color>\n", xtra);
            break;
        case GF_OLD_SLOW: 
            doc_printf(cols[0], "<tab:10><color:B>Slows%s</color>\n", xtra);
            break;
        case GF_OLD_CONF: 
            doc_printf(cols[0], "<tab:10><color:B>Confuses%s</color>\n", xtra);
            break;
        case GF_OLD_SLEEP: 
            doc_printf(cols[0], "<tab:10><color:B>Sleeps%s</color>\n", xtra);
            break;
        case GF_STASIS:
        case GF_PARALYSIS:
            doc_printf(cols[0], "<tab:10><color:B>Paralyzes%s</color>\n", xtra);
            break;
        case GF_DRAIN_MANA:
            doc_printf(cols[0], "<tab:10><color:B>Drains Mana%s</color>\n", xtra);
            break;
        case GF_STUN:
            doc_printf(cols[0], "<tab:10><color:B>Stuns%s</color>\n", xtra);
            break;
        case GF_TURN_ALL:
            doc_printf(cols[0], "<tab:10><color:r>Terrifies%s</color>\n", xtra);
            break;
        case GF_QUAKE:
            doc_printf(cols[0], "<tab:10><color:B>Shatters%s</color>\n", xtra);
            break;
        default:
            doc_printf(cols[0], "<color:r> %-7.7s</color>: %d\n",
                    _effect_name(a->effect[i]),
                    blows * (min2 + max2)/200
            );
        }
    }

    /* Second Column */
    doc_insert(cols[1], "<color:G>Accuracy</color>\n");
    doc_insert(cols[1], " AC Hit\n");

    doc_printf(cols[1], "%3d %2d%%\n", 25, hit_chance_innate(to_h, 25));
    doc_printf(cols[1], "%3d %2d%%\n", 50, hit_chance_innate(to_h, 50));
    doc_printf(cols[1], "%3d %2d%%\n", 75, hit_chance_innate(to_h, 75));
    doc_printf(cols[1], "%3d %2d%%\n", 100, hit_chance_innate(to_h, 100));
    doc_printf(cols[1], "%3d %2d%%\n", 125, hit_chance_innate(to_h, 125));
    doc_printf(cols[1], "%3d %2d%%\n", 150, hit_chance_innate(to_h, 150));
    doc_printf(cols[1], "%3d %2d%%\n", 175, hit_chance_innate(to_h, 175));
    doc_printf(cols[1], "%3d %2d%%\n", 200, hit_chance_innate(to_h, 200));

    doc_insert_cols(doc, cols, 2, 1);
    doc_free(cols[0]);
    doc_free(cols[1]);
}

/**********************************************************************
 * Ranged Attacks
 **********************************************************************/

/* Multiplier scaled by 100 */
int bow_mult(object_type *o_ptr)
{
    int mult = o_ptr->mult;

    mult += p_ptr->shooter_info.to_mult;

    if (weaponmaster_is_(WEAPONMASTER_CROSSBOWS))
    {
        int idx = p_ptr->stat_ind[A_STR] + 4;
        if (idx > 40-3)
            idx = 40-3;
        mult = mult * (100 + (int)(adj_str_td[idx]) - 128) / 100;
    }
    else
        mult = mult * (100 + (int)(adj_str_td[p_ptr->stat_ind[A_STR]]) - 128) / 100;

    return mult;
}

int bow_range(object_type *o_ptr)
{
    int range, mult;

    switch (o_ptr->sval)
    {
    case SV_LIGHT_XBOW:
        range = 9; /* somebody is adding 1 later ... */
        range += (p_ptr->concent + 1) / 2;    /* Snipers? */
        break;

    case SV_SHORT_BOW:
        range = 9; /* somebody is adding 1 later ... */
        break;

    default:
        mult = bow_mult(o_ptr);
        range = 13 + mult/80;
        if (o_ptr->sval == SV_HEAVY_XBOW)
        {
            if (p_ptr->concent)
                range -= (5 - (p_ptr->concent + 1) / 2);
            else
                range -= 5;
        }
        break;
    }
    if (prace_is_(RACE_DEMIGOD) && p_ptr->psubrace == DEMIGOD_ARTEMIS)
        range += 1 + p_ptr->lev/12;

    return range;
}



static void _display_missile_slay(int base_mult, int slay_mult, int shots, 
                                  int dd, int ds, int to_d, int to_d_xtra, 
                                  cptr name, int color, doc_ptr doc)
{
    int mult, min, max;

    mult = slay_mult;
    mult = mult * base_mult / 100;

    if (p_ptr->concent) 
    {
        min = shots * (boost_concentration_damage(mult*(dd + to_d)/100) + to_d_xtra) / 100;
        max = shots * (boost_concentration_damage(mult*(dd*ds + to_d)/100) + to_d_xtra) / 100;
    }
    else
    {
        min = shots * (mult*(dd + to_d)/100 + to_d_xtra) / 100;
        max = shots * (mult*(dd*ds + to_d)/100 + to_d_xtra) / 100;
    }

    doc_printf(doc, " <color:%c>%-8.8s</color>", attr_to_attr_char(color), name);
    doc_printf(doc, ": %d [%d.%02dx]\n",
                    (min + max)/2,
                    mult/100, mult%100);
}


static void _shooter_info_aux(doc_ptr doc, object_type *bow, object_type *arrow, int ct)
{
    char         o_name[MAX_NLEN];
    u32b         flgs[OF_ARRAY_SIZE];
    int          mult;
    int          to_h = 0;
    int          to_d = 0;
    int          to_h_bow = 0;
    int          to_d_bow = 0;
    int          to_h_xtra = p_ptr->shooter_info.dis_to_h;
    int          to_d_xtra = p_ptr->shooter_info.dis_to_d;
    int          dd = arrow->dd;
    int          ds = arrow->ds;
    critical_t   crit = {0}; 
    int          num_fire = 0;
    doc_ptr      cols[2] = {0};

    cols[0] = doc_alloc(60);
    cols[1] = doc_alloc(10);

    missile_flags_known(arrow, flgs);
    mult = bow_mult(bow);

    if (object_is_artifact(arrow))
        num_fire = 100;
    else if (p_ptr->shooter_info.num_fire)
        num_fire = p_ptr->shooter_info.num_fire * 100 * 100 / bow_energy(bow->sval);

    if (object_is_known(bow))
    {
        to_h_bow = bow->to_h;
        to_d_bow = bow->to_d;
        if (weaponmaster_is_(WEAPONMASTER_CROSSBOWS) && p_ptr->lev >= 15)
            to_d_bow += 1 + p_ptr->lev/10;
    } 

    if (object_is_known(arrow))
    {
        to_h = arrow->to_h;
        to_d = arrow->to_d;
    }

    if (p_ptr->big_shot)
        dd *= 2;

    {
        const int ct = 10 * 1000;
        int i;
        /* Compute Average Effects of Criticals by sampling */
        for (i = 0; i < ct; i++)
        {
            critical_t tmp = critical_shot(arrow->weight, arrow->to_h);
            if (tmp.desc)
            {
                crit.mul += tmp.mul;
                crit.to_d += tmp.to_d;
            }
            else
                crit.mul += 100;
        }
        crit.mul = crit.mul / ct;
        crit.to_d = crit.to_d * 100 / ct;
    }

    /* First Column */
    object_desc(o_name, arrow, OD_OMIT_INSCRIPTION | OD_COLOR_CODED);
    doc_printf(cols[0], "<color:u> Ammo #%-2d</color>: <indent><style:indent>%s</style></indent>\n", ct, o_name);

    doc_printf(cols[0], " %-8.8s: %d.%d lbs\n", "Weight", arrow->weight/10, arrow->weight%10);
    doc_printf(cols[0], " %-8.8s: %d + %d = %d\n", "To Hit", to_h, to_h_bow + to_h_xtra, to_h + to_h_bow + to_h_xtra);
    doc_printf(cols[0], " %-8.8s: %d + %d = %d (%s)\n", "To Dam", to_d, to_d_bow, to_d + to_d_bow, "Multiplier Applies");
    doc_printf(cols[0], " <color:G>%-8.8s</color>\n", "Damage");

    if (crit.to_d)
    {
        doc_printf(cols[0], " %-8.8s: %d.%02dx + %d.%02d\n", "Crits",
                        crit.mul/100, crit.mul%100, crit.to_d/100, crit.to_d%100);
    }
    else
    {
        doc_printf(cols[0], " %-8.8s: %d.%02dx\n", "Crits",
                        crit.mul/100, crit.mul%100);
    }

    to_d = to_d + to_d_bow;
    mult = mult * crit.mul / 100;
    to_d_xtra = to_d_xtra + crit.to_d/100;

    _display_missile_slay(mult, 100, num_fire, dd, ds, to_d, to_d_xtra, "Normal", TERM_WHITE, cols[0]);

    if (p_ptr->tim_force && p_ptr->csp > (p_ptr->msp / 30))
    {
        mult = mult * 3 / 2;
        _display_missile_slay(mult, 100, num_fire, dd, ds, to_d, to_d_xtra, "Force", TERM_L_BLUE, cols[0]);
    }

    if (have_flag(flgs, OF_KILL_ANIMAL)) 
        _display_missile_slay(mult, 270, num_fire, dd, ds, to_d, to_d_xtra, "Animals", TERM_YELLOW, cols[0]);
    else if (have_flag(flgs, OF_SLAY_ANIMAL))
        _display_missile_slay(mult, 170, num_fire, dd, ds, to_d, to_d_xtra, "Animals", TERM_YELLOW, cols[0]);
    
    if (have_flag(flgs, OF_KILL_EVIL))   
        _display_missile_slay(mult, 250, num_fire, dd, ds, to_d, to_d_xtra, "Evil", TERM_YELLOW, cols[0]);
    else if (have_flag(flgs, OF_SLAY_EVIL))
        _display_missile_slay(mult, 150, num_fire, dd, ds, to_d, to_d_xtra, "Evil", TERM_YELLOW, cols[0]);

    if (have_flag(flgs, OF_KILL_HUMAN))
        _display_missile_slay(mult, 270, num_fire, dd, ds, to_d, to_d_xtra, "Human", TERM_YELLOW, cols[0]);
    else if (have_flag(flgs, OF_SLAY_HUMAN))
        _display_missile_slay(mult, 170, num_fire, dd, ds, to_d, to_d_xtra, "Human", TERM_YELLOW, cols[0]);

    if (have_flag(flgs, OF_KILL_UNDEAD))
        _display_missile_slay(mult, 300, num_fire, dd, ds, to_d, to_d_xtra, "Undead", TERM_YELLOW, cols[0]);
    else if (have_flag(flgs, OF_SLAY_UNDEAD))
        _display_missile_slay(mult, 200, num_fire, dd, ds, to_d, to_d_xtra, "Undead", TERM_YELLOW, cols[0]);
    
    if (have_flag(flgs, OF_KILL_DEMON))  
        _display_missile_slay(mult, 300, num_fire, dd, ds, to_d, to_d_xtra, "Demons", TERM_YELLOW, cols[0]);
    else if (have_flag(flgs, OF_SLAY_DEMON))
        _display_missile_slay(mult, 200, num_fire, dd, ds, to_d, to_d_xtra, "Demons", TERM_YELLOW, cols[0]);

    if (have_flag(flgs, OF_KILL_ORC))  
        _display_missile_slay(mult, 300, num_fire, dd, ds, to_d, to_d_xtra, "Orcs", TERM_YELLOW, cols[0]);
    else if (have_flag(flgs, OF_SLAY_ORC))
        _display_missile_slay(mult, 200, num_fire, dd, ds, to_d, to_d_xtra, "Orcs", TERM_YELLOW, cols[0]);

    if (have_flag(flgs, OF_KILL_TROLL))  
        _display_missile_slay(mult, 300, num_fire, dd, ds, to_d, to_d_xtra, "Trolls", TERM_YELLOW, cols[0]);
    else if (have_flag(flgs, OF_SLAY_TROLL))
        _display_missile_slay(mult, 200, num_fire, dd, ds, to_d, to_d_xtra, "Trolls", TERM_YELLOW, cols[0]);

    if (have_flag(flgs, OF_KILL_GIANT))  
        _display_missile_slay(mult, 300, num_fire, dd, ds, to_d, to_d_xtra, "Giants", TERM_YELLOW, cols[0]);
    else if (have_flag(flgs, OF_SLAY_GIANT))
        _display_missile_slay(mult, 200, num_fire, dd, ds, to_d, to_d_xtra, "Giants", TERM_YELLOW, cols[0]);

    if (have_flag(flgs, OF_KILL_DRAGON))  
        _display_missile_slay(mult, 300, num_fire, dd, ds, to_d, to_d_xtra, "Dragons", TERM_YELLOW, cols[0]);
    else if (have_flag(flgs, OF_SLAY_DRAGON))
        _display_missile_slay(mult, 200, num_fire, dd, ds, to_d, to_d_xtra, "Dragons", TERM_YELLOW, cols[0]);
    
    if (have_flag(flgs, OF_BRAND_ACID))
        _display_missile_slay(mult, 170, num_fire, dd, ds, to_d, to_d_xtra, "Acid", TERM_RED, cols[0]);

    if (have_flag(flgs, OF_BRAND_ELEC))
        _display_missile_slay(mult, 170, num_fire, dd, ds, to_d, to_d_xtra, "Elec", TERM_RED, cols[0]);

    if (have_flag(flgs, OF_BRAND_FIRE))
        _display_missile_slay(mult, 170, num_fire, dd, ds, to_d, to_d_xtra, "Fire", TERM_RED, cols[0]);

    if (have_flag(flgs, OF_BRAND_COLD))
        _display_missile_slay(mult, 170, num_fire, dd, ds, to_d, to_d_xtra, "Cold", TERM_RED, cols[0]);

    if (have_flag(flgs, OF_BRAND_POIS))
        _display_missile_slay(mult, 170, num_fire, dd, ds, to_d, to_d_xtra, "Poison", TERM_RED, cols[0]);

    /* Second Column */
    to_h = to_h + to_h_bow + to_h_xtra;

    doc_insert(cols[1], " <color:G>AC Hit</color>\n");
    doc_printf(cols[1], "%3d %2d%%\n", 25, bow_hit_chance(bow->sval, to_h, 25));
    doc_printf(cols[1], "%3d %2d%%\n", 50, bow_hit_chance(bow->sval, to_h, 50));
    doc_printf(cols[1], "%3d %2d%%\n", 100, bow_hit_chance(bow->sval, to_h, 100));
    doc_printf(cols[1], "%3d %2d%%\n", 150, bow_hit_chance(bow->sval, to_h, 150));
    doc_printf(cols[1], "%3d %2d%%\n", 175, bow_hit_chance(bow->sval, to_h, 175));
    doc_printf(cols[1], "%3d %2d%%\n", 200, bow_hit_chance(bow->sval, to_h, 200));

    doc_insert_cols(doc, cols, 2, 1);
    doc_free(cols[0]);
    doc_free(cols[1]);
}

void display_shooter_info(doc_ptr doc)
{
    object_type *bow_ptr = NULL;
    int          slot = equip_find_object(TV_BOW, SV_ANY);
    char         o_name[MAX_NLEN];
    int          mult;
    int          num_fire = 0;
    int          to_h = 0;
    int          to_d = 0;
    int          i, j;

    if (!slot || prace_is_(RACE_MON_JELLY) || p_ptr->shooter_info.tval_ammo == TV_NO_AMMO)
        return;

    bow_ptr = equip_obj(slot);
    assert(bow_ptr);

    mult = bow_mult(bow_ptr);

    if (p_ptr->shooter_info.num_fire)
        num_fire = p_ptr->shooter_info.num_fire * 100 * 100 / bow_energy(bow_ptr->sval);

    if (object_is_known(bow_ptr))
    {
        to_h = bow_ptr->to_h;
        to_d = bow_ptr->to_d;
        if (weaponmaster_is_(WEAPONMASTER_CROSSBOWS) && p_ptr->lev >= 15)
            to_d += 1 + p_ptr->lev/10;
    }

    /* Shooter */
    object_desc(o_name, bow_ptr, OD_OMIT_INSCRIPTION | OD_COLOR_CODED);
    doc_printf(doc, " <color:y>Shooting</color>: <indent><style:indent>%s</style></indent>\n", o_name);
    
    doc_printf(doc, " %-8.8s: %d'\n", "Range", (bow_range(bow_ptr) + 1) * 10);
    doc_printf(doc, " %-8.8s: %d.%02d\n", "Shots", num_fire/100, num_fire%100);
    doc_printf(doc, " %-8.8s: %d.%02dx\n", "Mult", mult/100, mult%100);
    doc_printf(doc, " %-8.8s: %d + %d = %d\n", "To Hit", to_h, p_ptr->shooter_info.dis_to_h, to_h + p_ptr->shooter_info.dis_to_h);
    doc_printf(doc, " %-8.8s: %d (%s)\n", "To Dam", to_d, "Multiplier Applies");
    doc_printf(doc, " %-8.8s: %d (%s)\n", "Xtra Dam", p_ptr->shooter_info.dis_to_d, "Multiplier Does Not Apply");
    doc_newline(doc);

    /* Ammo */
    j = 0;
    for (i = 0; i < INVEN_PACK; i++)
    {
        if (inventory[i].tval == p_ptr->shooter_info.tval_ammo)
            _shooter_info_aux(doc, bow_ptr, &inventory[i], ++j);
    }

}

