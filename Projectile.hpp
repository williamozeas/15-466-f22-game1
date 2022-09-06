//
// Created by William Ozeas on 9/5/22.
//

#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "PPU466.hpp"
#include "Direction.hpp"


struct Projectile {
    Projectile(PPU466::Sprite *sprite);
    ~Projectile();

    const float PROJECTILE_SPEED = 2.0f;

    void update(float elapsed);
    void draw();
    bool check_collision(PPU466::Sprite *coll);

    void spawn(PPU466 *ppu, glm::vec2 *center, Direction dir);
    void despawn();

    PPU466::Sprite *sprite;
    float x;
    float y;

    const float RAD_2_INV = 1 / sqrt(2.0f);

    Direction direction;
};

#endif
