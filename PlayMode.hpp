#ifndef PLAYMODE_H
#define PLAYMODE_H

#include "PPU466.hpp"
#include "Mode.hpp"
#include "Projectile.hpp"
#include "Load.hpp"
#include "read_write_chunk.hpp"
#include "data_path.hpp"
#include "Direction.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>



struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();


    Direction currentDirection;
    struct ProjectileSet {
        //8 bits, each representing a direction, starting from up and going clockwise
        uint8_t directions;
    };
    struct Asteroid {
        uint8_t sprite;
        uint16_t radius;
        float time_offset;
        float period;
    };

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;
    virtual void tick(); //for projectiles

    virtual void reset();

    void spawn_projectile(Direction dir);
    void despawn_projectile(Projectile *proj);

    //tick implementation from CMU CS15466 online text at https://15466.courses.cs.cmu.edu/lesson/timing
    static constexpr float Tick = 0.25f; //timestep used for tick()
    float tick_acc = 0.0f; //accumulated time toward next tick

    const int START_DIST = 20;
    const int END_TIME = 90;

	//----- game state -----

    //deque of projectiles
    std::deque< Projectile * > *unusedProjectiles;
    std::deque< Projectile * > *activeProjectiles;

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//some weird background animation:
	float projectile_fade = 0.0f;

	//player position:
	glm::vec2 player_at = glm::vec2(0.0f);

    float distance_from_center = (float)START_DIST;
    float totalTime = 0.0f;
    uint32_t tick_index = 0;

	//asteroid constants
    //if i had more time i'd make this come from a txt file or something
    const static uint16_t ASTEROID_COUNT = 16;
    const Asteroid asteroids[ASTEROID_COUNT] = {
            {8, 8, 0.0f, 6.0f},
            {9, 12, 0.8f, 6.0f},
            {6, 10, 2.4f, 6.0f},
            {9, 11, 3.1f, 6.0f},
            {5, 25, 4.0f, 5.0f},
            {8, 35, 7.0f, 9.0f},
            {7, 40, 3.1f, 7.0f},
            {9, 82, 1.0f, 9.5f},
            {5, 56, 3.1f, 10.5f},
            {6, 65, 0.0f, 18.0f},
            {7, 72, 3.1f, 13.0f},
            {5, 90, 4.0f, 12.0f},
            {9, 99, 12.0f, 17.0f},
            {8, 115, 8.0f, 26.0f},
            {7, 123, 10.0f, 20.0f},
            {9, 131, 1.0f, 25.0f},
    };

    //Math constants
    const float RAD_2_INV = 1 / sqrt(2.0f);
    glm::vec2 center;



	//----- drawing handled by PPU466 -----

	PPU466 ppu;
};

#endif
