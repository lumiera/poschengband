#ifndef EQUIP_H
#define EQUIP_H

/* Equipment Module.

   The goal is to support different kinds of equipment "templates".
   For example, a "hydra" player race would have multiple heads and
   should allow one amulet and helmet per head. An "insect" player
   race might allow multiple pairs of boots, but body armor is 
   probably out. Dragons get multiple ring slots but not much else
   (cf Drangband which was always good fun, back in the day).

   A "slot" indexes into the global inventory array at the moment, 
   though that is subject to change should I feel ambitious. Always
   call equip_obj(slot) rather than inventory[slot]. Be prepared
   for null pointers if the slot is empty. Also, 0 is never a valid
   slot so you can treat slot values as if they were booleans.

   To enumerate the equipment use
   [1] equip_for_each_slot,
   [2] equip_for_each_obj, or
   [3] for (slot = EQUIP_BEGIN; slot < EQUIP_BEGIN + equip_count(); slot++)
       {
           object_type *o_ptr = equip_obj(slot);
           if (o_ptr)
           {
           }
       }
   [4] for (slot = equip_find_first(object_is_melee_weapon);
             slot;
             slot = equip_find_next(object_is_melee_weapon, slot))
       {
           object_type *o_ptr = equip_obj(slot);
           ...
       }

    NULL predicates are always OK and mean that no restriction should be applied.
*/

/* Had to move to defines.h ... sigh
#define EQUIP_BEGIN 24
#define EQUIP_MAX_SLOTS (INVEN_TOTAL - EQUIP_BEGIN + 1)

enum slot_e {
    EQUIP_SLOT_NONE,
    EQUIP_SLOT_GLOVES,
    EQUIP_SLOT_WEAPON_SHIELD,
    EQUIP_SLOT_RING,         
    EQUIP_SLOT_BOW,          
    EQUIP_SLOT_AMULET,       
    EQUIP_SLOT_LITE,         
    EQUIP_SLOT_BODY_ARMOR,   
    EQUIP_SLOT_CLOAK,        
    EQUIP_SLOT_BOOTS,        
    EQUIP_SLOT_HELMET,       
    EQUIP_SLOT_ANY,             
    EQUIP_SLOT_WEAPON,
    EQUIP_SLOT_CAPTURE_BALL,
    EQUIP_SLOT_MAX,
};
*/
/* Had to move to types.h ... sigh
typedef struct {
    int  type;
    cptr name;
    int  hand; 
} slot_t;

typedef struct equip_template_s {
    int        count;
    slot_t     slots[EQUIP_MAX_SLOTS];
} equip_template_t, *equip_template_ptr;
*/

typedef void (*object_fn)(object_type *o_ptr);
typedef void (*slot_fn)(int slot);

extern void               equip_wield(void); 
extern void               equip_wield_aux(object_type *o_ptr, int slot); 
extern void               equip_takeoff(void);
extern void               equip_takeoff_aux(int slot); /* takeoff, drop, throw */
extern void               equip_calc_bonuses(void);
extern void               equip_for_each_obj(object_fn f);
extern void               equip_for_each_slot(slot_fn f);
extern int                equip_count(void);
extern int                equip_count_used(void);
extern bool               equip_is_valid_slot(int slot);
extern int                equip_first_slot(object_type *o_ptr);
extern int                equip_first_empty_slot(object_type *o_ptr);
extern int                equip_next_slot(object_type *o_ptr, int last);
extern int                equip_find_first(object_p p);
extern int                equip_find_next(object_p p, int last);
extern int                equip_find_artifact(int which);
extern int                equip_find_ego(int which);
extern int                equip_find_empty_hand(void);
extern int                equip_find_object(int tval, int sval);
extern int                equip_random_slot(object_p p);
extern int                equip_slot_type(int slot);
extern cptr               equip_describe_slot(int slot);
extern int                equip_is_worn(object_type *o_ptr); /* Hack for sloppy code ... */
extern int                equip_which_hand(object_type *o_ptr); /* Hack for sloppy code ... */
extern object_type       *equip_obj(int slot);
extern int                equip_weight(object_p p);
extern void               equip_on_init(void);
extern void               equip_on_change_race(void);
extern bool               equip_can_wield_kind(int tval, int sval);

extern bool               equip_is_empty_hand(int hand);
extern bool               equip_is_valid_hand(int hand);
extern bool               equip_is_empty_two_handed_slot(int slot);

#endif