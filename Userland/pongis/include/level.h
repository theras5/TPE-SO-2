// level.h
#ifndef LEVEL_H
#define LEVEL_H

#include "pongis_types.h"

extern const Level levels[];
extern const uint64_t level_count;

void load_level(GameState *state, int index);

#endif // LEVEL_H