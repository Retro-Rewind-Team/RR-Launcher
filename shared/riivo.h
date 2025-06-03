#ifndef RRC_RIIVO_H
#define RRC_RIIVO_H

#include "types.h"

#define RRC_RIIVO_XML_PATH "RetroRewind6/xml/RetroRewind6.xml"

enum rrc_riivo_disc_replacement_type
{
    RRC_RIIVO_FILE_REPLACEMENT,
    RRC_RIIVO_FOLDER_REPLACEMENT,
};

struct rrc_riivo_disc_replacement
{
    enum rrc_riivo_disc_replacement_type type;
    const char *external;
    const char *disc;
};

struct rrc_riivo_disc
{
    u32 count;
    struct rrc_riivo_disc_replacement replacements[0];
};

struct rrc_riivo_memory_patch
{
    u32 addr;
    u32 value;
    u32 original; // uninitialized if !original_init
    bool original_init;
};

#endif
