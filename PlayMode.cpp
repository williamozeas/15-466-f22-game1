
#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <fstream>
#include <cmath>
#include <deque>

//struct ProjectileSet;

//Loading main tile set from dist/assets/main.tiles

Load < std::vector< PPU466::Tile > > main_tiles( LoadTagDefault,  []() {
    std::ifstream ifs(data_path("main.tiles").c_str(), std::ios::binary);
    if(!ifs.is_open()) {
        std::cerr << "File open failed\n";
    }
    auto *tiles = new std::vector< PPU466::Tile >();
    read_chunk(ifs, "til0", tiles);
    return tiles;
});

Load< std::vector < PlayMode::ProjectileSet >> projectile_map(LoadTagDefault, []() {
    std::ifstream ifs(data_path("projectile.map").c_str(), std::ios::binary);
    if(!ifs.is_open()) {
        std::cerr << "File open failed\n";
    }
    auto *maps = new std::vector < PlayMode::ProjectileSet >();
    read_chunk(ifs, "map0", maps);
    return maps;
});

PlayMode::PlayMode() {
    glm::i8vec2 centers(int8_t(PPU466::ScreenWidth/2 - 4), int8_t(PPU466::ScreenHeight/2 - 4));
    center = centers;

    unusedProjectiles = new std::deque< Projectile * >();
    activeProjectiles = new std::deque< Projectile * >();

    for(uint16_t i = 20; i < ppu.sprites.size(); i++) {
        unusedProjectiles->push_back(new Projectile(&ppu.sprites.at(i)));
    }


    //Use read_chunk to get tile objects from binaries
    for(uint32_t i = 0; i < main_tiles->size(); i++) {
        ppu.tile_table[i] = main_tiles->at(i);
    }
    //Sprite numbers
    //player sprite = 0
    //projectile sprite = 1
    //black hole sprites = 2 - 3
    //empty tile = 4
    //asteroids = 5 - 9
    //background centers = 10 - 13

	//black hole:
	ppu.palette_table[0] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	};

	//projectiles:
	ppu.palette_table[1] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0xff, 0xff, 0xff, 0xff),
		glm::u8vec4(0xfc, 0xe7, 0x7d, 0xff),
		glm::u8vec4(0xff, 0xff, 0xff, 0xff),
	};

    //asteroids
    ppu.palette_table[2] = {
            glm::u8vec4(0x00, 0x00, 0x00, 0x00),
            glm::u8vec4(0xa0, 0xa0, 0xa0, 0xff),
            glm::u8vec4(0x00, 0x00, 0x00, 0x00),
            glm::u8vec4(0xa0, 0xa0, 0xa0, 0xff),
    };

    //background
    ppu.palette_table[3] = {
            glm::u8vec4(0x00, 0x00, 0x00, 0x00),
            glm::u8vec4(0xe0, 0xe0, 0xe0, 0xff),
            glm::u8vec4(0x00, 0x00, 0x00, 0x00),
            glm::u8vec4(0x00, 0xff, 0x00, 0xff),
    };

	//used for the player:
	ppu.palette_table[7] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
        glm::u8vec4(0xff, 0xff, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	};

    //for end screen
	ppu.palette_table[6] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
        glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0xfa, 0xcb, 0xde, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	};

    ppu.background_color = glm::vec3(0x00, 0x00, 0x00);

    for (uint32_t i = 0; i < ppu.background.size(); ++i) {
        int rand = std::rand() % 40;
        if(rand == 0) {
            ppu.background[i] = 0x030e;
        } else if(rand <= 4) {
            ppu.background[i] = 0x030f;
        } else {
            ppu.background[i] = 0x0004;
        }
    }
    ppu.background[(ppu.BackgroundWidth) * (ppu.ScreenHeight/16)     + ppu.ScreenWidth/16 - 1] = 0x010a;
    ppu.background[(ppu.BackgroundWidth) * (ppu.ScreenHeight/16 - 1) + ppu.ScreenWidth/16 - 1] = 0x010b;
    ppu.background[(ppu.BackgroundWidth) * (ppu.ScreenHeight/16 - 1) + ppu.ScreenWidth/16] = 0x010c;
    ppu.background[(ppu.BackgroundWidth) * (ppu.ScreenHeight/16)     + ppu.ScreenWidth/16] = 0x010d;

    //set player start position
    currentDirection = upDir;


    //asteroids sprites 4-19
    for(int i = 0; i < ASTEROID_COUNT; i++) {
        ppu.sprites[i + 4].attributes = 0x02;
        ppu.sprites[i + 4].index = asteroids[i].sprite;
    }

}

PlayMode::~PlayMode() {
    for(Projectile *proj : *activeProjectiles) {
        free(proj);
    }
    for(Projectile *proj : *unusedProjectiles) {
        free(proj);
    }

    free(activeProjectiles);
    free(unusedProjectiles);
}

void PlayMode::reset() {
    tick_index = 0;
    totalTime = 0;
    distance_from_center = (float)START_DIST;
    currentDirection = upDir;

    for(Projectile *proj : *activeProjectiles) {
        despawn_projectile(proj);
    }
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
    bool ret = false;
	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			ret = true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			ret = true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			ret = true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			ret = true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			ret = true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			ret = true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			ret = true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			ret = true;
		}
	}

    if (right.pressed && down.pressed) {
        currentDirection = downRightDir;
    }
    else if (right.pressed && up.pressed) {
        currentDirection = upRightDir;
    }
    else if (left.pressed && down.pressed) {
        currentDirection = downLeftDir;
    }
    else if (left.pressed && up.pressed) {
        currentDirection = upLeftDir;
    }
    else if (right.pressed) {
        currentDirection = rightDir;
    }
    else if (left.pressed) {
        currentDirection = leftDir;
    }
    else if (up.pressed) {
        currentDirection = upDir;
    }
    else if (down.pressed) {
        currentDirection = downDir;
    }

	return ret;
}

void PlayMode::spawn_projectile(Direction dir) {
    if(!unusedProjectiles->empty()) {
        Projectile *proj = unusedProjectiles->front();
        unusedProjectiles->pop_front();
        activeProjectiles->push_back(proj);
        proj->spawn(&ppu, &center, dir);
    } else {
        std::cout << "ran out of projectiles!\n"; //will continue running
    }
}
void PlayMode::despawn_projectile(Projectile *proj) {
    assert(proj);
    assert(activeProjectiles->size() > 0 && "activeProjectiles Empty!");

    proj->despawn();
    unusedProjectiles->push_back(proj);
    activeProjectiles->erase(std::find(activeProjectiles->begin(), activeProjectiles->end(), proj));
}

void PlayMode::tick() {
    //handles spawning of projectiles
    if(tick_index + 1 >= projectile_map->size()) {
        return;
    }
    tick_index++;

    //spawn projectiles
    ProjectileSet set = projectile_map->at(tick_index);
    if (set.directions & 0x80) {
        //spawn top middle projectile etc.
        spawn_projectile(downDir);
    }
    if (set.directions & 0x40) {
        spawn_projectile(downLeftDir);
    }
    if (set.directions & 0x20) {
        spawn_projectile(leftDir);
    }
    if (set.directions & 0x10) {
        spawn_projectile(upLeftDir);
    }
    if (set.directions & 0x08) {
        spawn_projectile(upDir);
    }
    if (set.directions & 0x04) {
        spawn_projectile(upRightDir);
    }
    if (set.directions & 0x02) {
        spawn_projectile(rightDir);
    }
    if (set.directions & 0x01) {
        spawn_projectile(downRightDir);
    }
}

void PlayMode::update(float elapsed) {
    //tick implementation from CMU CS15466 online text at https://15466.courses.cs.cmu.edu/lesson/timing
    tick_acc += elapsed;
    while (tick_acc > Tick) {
        tick_acc -= Tick;
        tick();
    }

    //color update
    if(totalTime > 0.5f) {
        ppu.background_color = glm::vec3(
                0x00,
                0x00 + ((totalTime / END_TIME) * 0x80),
                0x00
                );
    }

    //win state
    if(totalTime > END_TIME) {
        //win state
        ppu.sprites[7].index = 4;
        ppu.sprites[1].index = 4;
        for(uint16_t i = 0; i < ppu.background.size(); i++) {
            ppu.background[i] = 7;
            if( i % 2 == 0 ) {
                ppu.background[i] = 0x0700;
            } else {
                ppu.background[i] = 0x0600;
            }
        }
    } else {
        totalTime += elapsed;
        distance_from_center += elapsed * 0.5f;
    }
	//slowly rotates through [0,1):
	// (used for projectiles)
	projectile_fade += elapsed / 10.0f;
    projectile_fade -= std::floor(projectile_fade);

    //saving projectiles to delete so that activeProjectiles isnt edited while iterating through it
    std::vector< Projectile * > projectiles_to_delete;

    //check collision
    for(Projectile *proj : *activeProjectiles) {
        //check player sprite & black hole
        if (proj->check_collision(&ppu.sprites[0])) {
            ppu.background_color = glm::vec3(0xff, 0x40, 0x40);
            projectiles_to_delete.clear();
            reset();
            break;
        } else if(proj->check_collision(&ppu.sprites[1])) {
            //black hole collision
            projectiles_to_delete.push_back(proj);
        }
    }

    //could be sped up by saving iterators and deleting that instead
    for(Projectile *proj : projectiles_to_delete) {
        despawn_projectile(proj);
    }


    //move projectiles
    for(Projectile *proj : *activeProjectiles) {
        proj->update(elapsed);
    }

    //update movement
    //i forgot switches exist, yes
    if (currentDirection == downRightDir) {
        player_at.x = center.x + RAD_2_INV * distance_from_center;
        player_at.y = center.y - RAD_2_INV * distance_from_center;
    }
    else if (currentDirection == upRightDir) {
        player_at.x = center.x + RAD_2_INV * distance_from_center;
        player_at.y = center.y + RAD_2_INV * distance_from_center;
    }
    else if (currentDirection == downLeftDir) {
        player_at.x = center.x - RAD_2_INV * distance_from_center;
        player_at.y = center.y - RAD_2_INV * distance_from_center;
    }
    else if (currentDirection == upLeftDir) {
        player_at.x = center.x - RAD_2_INV * distance_from_center;
        player_at.y = center.y + RAD_2_INV * distance_from_center;
    }
    else if (currentDirection == rightDir) {
        player_at.x = center.x + distance_from_center;
        player_at.y = center.y;
    }
    else if (currentDirection == leftDir) {
        player_at.x = center.x - distance_from_center;
        player_at.y = center.y;
    }
    else if (currentDirection == upDir) {
        player_at.x = center.x;
        player_at.y = center.y + distance_from_center;
    }
    else if (currentDirection == downDir) {
        player_at.x = center.x;
        player_at.y = center.y - distance_from_center;
    }

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

    //inside of projectile is a raaaainbowwwww
    ppu.palette_table[1][2] = glm::u8vec4(
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (projectile_fade + 0.0f / 3.0f) ) ) ))),
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (projectile_fade + 1.0f / 3.0f) ) ) ))),
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (projectile_fade + 2.0f / 3.0f) ) ) ))),
		0xff
	);


    //asteroids sprites 4-15
    for(uint16_t i = 0; i < ASTEROID_COUNT; i++) {
        ppu.sprites[i + 4].x = (uint8_t)std::round(center.x + asteroids[i].radius * std::cos( 2.0f * M_PI * ((totalTime + asteroids[i].time_offset)/asteroids[i].period)));
        ppu.sprites[i + 4].y = (uint8_t)std::round(center.y + asteroids[i].radius * std::sin( 2.0f * M_PI * ((totalTime + asteroids[i].time_offset)/asteroids[i].period)));
    }

	//player sprite:
	ppu.sprites[0].x = int8_t(player_at.x);
	ppu.sprites[0].y = int8_t(player_at.y);
	ppu.sprites[0].index = 0;
	ppu.sprites[0].attributes = 7;

    //black hole
    ppu.sprites[1].x = (uint8_t)std::round(center.x);
    ppu.sprites[1].y = (uint8_t)std::round(center.y);
    if((uint32_t)(totalTime * 2) % 2 == 0) {
        ppu.sprites[1].index = 2;
    } else {
        ppu.sprites[1].index = 3;
    }
    ppu.sprites[1].attributes = 0;

    //projectiles
    for(Projectile *proj : *activeProjectiles) {
        proj->draw();
    }

	//--- actually draw ---
	ppu.draw(drawable_size);
}
