#include "debug.h"
#include "hash.h"
#include "hex_dump.h"
#include "limits.h"
#include "list.h"
#include "round.h"
#include "bitmap.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char *commandlist[]={ /*명렁어를 저장한 array*/
    "create", "delete", "dumpdata", "list_push_front", "list_push_back", "list_pop_front", "list_pop_back", "list_remove", "list_insert", 
    "list_splice", "list_front", "list_back", "list_sort","list_insert_ordered", "list_unique", "list_reverse", "list_empty", "list_size",
    "list_max", "list_min", "list_swap", "list_shuffle",

    "hash_insert", "hash_apply", "hash_delete", "hash_find", "hash_empty", "hash_size","hash_clear", "hash_replace",

    "bitmap_mark", "bitmap_all", "bitmap_any", "bitmap_contains", "bitmap_count", "bitmap_flip", "bitmap_none", "bitmap_reset", "bitmap_scan",
    "bitmap_scan_and_flip", "bitmap_set_all", "bitmap_set_multiple", "bitmap_set", "bitmap_size",  "bitmap_test", "bitmap_dump", "bitmap_expand"

};

const char *category[]={
    "list", "hashtable", "bitmap"
};
void commandcheck(char * str);// command check
void del(char *str);// delete ..
void dumpdata(char *str);// list 출력

struct list *lcon [10]; // 리스트 저장소 
struct hash *hcon [10]; // hash 저장소
struct bitmap *bcon[10]; //bitmap 저장소
