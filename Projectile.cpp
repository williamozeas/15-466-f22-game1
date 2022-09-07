//
// Created by William Ozeas on 9/5/22.
//

#include <iostream>
#include "Projectile.hpp"

Projectile::Projectile(PPU466::Sprite *sprite_in) {
    sprite = sprite_in;
    sprite->index = 1;
    sprite->attributes = 1;
}

Projectile::~Projectile() {
    sprite->y = 250;
}

void Projectile::update(float elapsed) {
    switch (direction) {
        case (downRightDir): {
            x += RAD_2_INV * PROJECTILE_SPEED;
            y -= RAD_2_INV * PROJECTILE_SPEED;
            break;
        }
        case (upRightDir): {
            x += RAD_2_INV * PROJECTILE_SPEED;
            y += RAD_2_INV * PROJECTILE_SPEED;
            break;
        }
        case (downLeftDir): {
            x -= RAD_2_INV * PROJECTILE_SPEED;
            y -= RAD_2_INV * PROJECTILE_SPEED;
            break;
        }
        case (upLeftDir): {
            x -= RAD_2_INV * PROJECTILE_SPEED;
            y += RAD_2_INV * PROJECTILE_SPEED;
            break;
        }
        case (rightDir): {
            x += PROJECTILE_SPEED;
            break;
        }
        case (leftDir): {
            x -= PROJECTILE_SPEED;
            break;
        }
        case (upDir): {
            y += PROJECTILE_SPEED;
            break;
        }
        case (downDir): {
            y -= PROJECTILE_SPEED;
            break;
        }
    }
}

void Projectile::draw() {
    sprite->x = (uint8_t)std::round(x);
    sprite->y = (uint8_t)std::round(y);
}

bool Projectile::check_collision(PPU466::Sprite *coll) {
    if(((int16_t)coll->x - 5 <= sprite->x && (int16_t)coll->x + 5 >= sprite->x)
        && ((int16_t)coll->y - 5 <= sprite->y && (int16_t)coll->y + 5 >= sprite->y)) {
        return true;
    }

    return false;
}

void Projectile::spawn(PPU466 *ppu, glm::vec2 *center, Direction dir) {
    sprite->index = 1;
    sprite->attributes = 1;

    direction = dir;

    x = center->x;
    y = center->y;
    glm::uvec2 spawnDist(ppu->ScreenWidth/2 - 4, ppu->ScreenHeight/2 - 4);
    switch (dir) {
        case (downRightDir): {
            x -= RAD_2_INV * spawnDist.x;
            y += RAD_2_INV * spawnDist.y;
            break;
        }
        case (upRightDir): {
            x -= RAD_2_INV * spawnDist.x;
            y -= RAD_2_INV * spawnDist.y;
            break;
        }
        case (downLeftDir): {
            x += RAD_2_INV * spawnDist.x;
            y += RAD_2_INV * spawnDist.y;
            break;
        }
        case (upLeftDir): {
            x += RAD_2_INV * spawnDist.x;
            y -= RAD_2_INV * spawnDist.y;
            break;
        }
        case (rightDir): {
            x -= spawnDist.x;
            break;
        }
        case (leftDir): {
            x += spawnDist.x;
            break;
        }
        case (upDir): {
            y -= spawnDist.y;
            break;
        }
        case (downDir): {
            y += spawnDist.y;
            break;
        }
    }
}

void Projectile::despawn() {
    sprite->y = 250;
}