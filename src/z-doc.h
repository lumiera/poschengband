#ifndef INCLUDED_Z_DOC_H
#define INCLUDED_Z_DOC_H

#include "h-basic.h"

#include "str-map.h"
#include "int-map.h"
#include "c-vec.h"

/* Utilities for Formatted Text Processing
   We support the ability to render rich text to a "virtual terminal",
   and then rapidly transfer a region of rendered text to the actual
   terminal for display. This is useful any time you want to support
   scrolling, wordwrapping, or color coded text (e.g. messages or
   the help system). We also support linked navigation, named styles and
   named bookmarks (called topics).

   The virtual terminal is a fixed width beast, but can grow its height
   dynamically. This is the way a normal "document" behaves: It grows as
   you type in new content.

   We support tags to control output and provide help features:
     <color:x> where x is one of the color codes:dwsorgbuDWvyRGBU
     <style:name>
     <topic:name>
     <link:file#topic>
     <$:var> where var is a valid variable reference (TODO: e.g. <$:version>)
*/

/* The Datatypes
   Key concepts are a position (for the cursor), a region (for transfers),
   a character (for contents and color), and a buffer chain.
*/

struct doc_pos_s
{
    int x;
    int y;
};
typedef struct doc_pos_s doc_pos_t;

bool doc_pos_is_valid(doc_pos_t pos);
int  doc_pos_compare(doc_pos_t left, doc_pos_t right);


struct doc_region_s
{/* [start, stop) */
    doc_pos_t start;
    doc_pos_t stop;
};
typedef struct doc_region_s doc_region_t, *doc_region_ptr;

bool doc_region_is_valid(doc_region_ptr region);
bool doc_region_contains(doc_region_ptr region, doc_pos_t pos);

struct doc_char_s
{
    char c;
    byte a;
};
typedef struct doc_char_s doc_char_t, *doc_char_ptr;

struct doc_style_s
{
    byte a;
    int  indent;
};
typedef struct doc_style_s doc_style_t, *doc_style_ptr;


struct doc_s
{
    doc_pos_t      cursor;
    int            width;
    doc_style_t    current_style;
    vec_ptr        pages;
    str_map_ptr    styles; /* name -> style */
    str_map_ptr    topics; /* name -> region */
    int_map_ptr    links;  /* char -> file#topic */
};
typedef struct doc_s doc_t, *doc_ptr;


/* Document API
   All text is added at the current location. The intention is that you build your
   document up front (by reading a help file, for example), and then use the
   copy_to_term() method to render to the display one screen at a time, depending
   on the current scroll position.
*/

doc_ptr      doc_alloc(int width);
void         doc_free(doc_ptr doc);

doc_pos_t    doc_cursor(doc_ptr doc);

doc_pos_t    doc_current_topic(doc_ptr doc, doc_pos_t pos);
doc_pos_t    doc_next_topic(doc_ptr doc, doc_pos_t pos);
doc_pos_t    doc_prev_topic(doc_ptr doc, doc_pos_t pos);
doc_pos_t    doc_find_topic(doc_ptr doc, cptr name);

doc_pos_t    doc_insert(doc_ptr doc, cptr text);
doc_pos_t    doc_newline(doc_ptr doc);
doc_pos_t    doc_measure(doc_ptr doc, cptr text);

doc_pos_t    doc_read_file(doc_ptr doc, FILE *fp);
void         doc_write_file(doc_ptr doc, FILE *fp);

doc_char_ptr doc_char(doc_ptr doc, doc_pos_t pos);
void         doc_copy_to_term(doc_ptr doc, doc_pos_t term_pos, int row, int ct);

/* Parsing */
enum doc_tag_e
{
    DOC_TAG_NONE,
    DOC_TAG_COLOR,
    DOC_TAG_STYLE,
    DOC_TAG_TOPIC,
    DOC_TAG_LINK,
};
struct doc_tag_s
{
    int  type;
    cptr arg;
    int  arg_size;
};
typedef struct doc_tag_s doc_tag_t, *doc_tag_ptr;

enum doc_token_e
{
    DOC_TOKEN_EOF,
    DOC_TOKEN_TAG,
    DOC_TOKEN_WHITESPACE,
    DOC_TOKEN_NEWLINE,
    DOC_TOKEN_WORD,
};
struct doc_token_s
{
    int       type;
    cptr      pos;
    int       size;
    doc_tag_t tag;
};
typedef struct doc_token_s doc_token_t, *doc_token_ptr;

cptr doc_parse_tag(cptr pos, doc_tag_ptr tag);
cptr doc_lex(cptr pos, doc_token_ptr token);

#endif