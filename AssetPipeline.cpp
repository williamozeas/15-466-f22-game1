//
// Created by William Ozeas on 9/4/22.
//
#ifndef ASSETPIPELINE_H
#define ASSETPIPELINE_H

#include "PPU466.hpp"
#include "read_write_chunk.hpp"
#include "load_save_png.hpp"
#include "data_path.hpp"
#include "PlayMode.hpp"
#include "strnatcmp.hpp"
#include <glm/glm.hpp>
#include <fstream>
#include <filesystem>

//load png
//convert data to tile format
//use write_chunk to save to file

//r, g, b, a channels used to correspond to 11, 10, 01, and 00 channels respectively
//correctly formed assets should only consist of colors #000000 #ff0000 #00ff00 and #0000ff
void png_to_tile(std::vector< glm::u8vec4 > *data, PPU466::Tile *tile, bool debug = false) {
    assert(data);
    assert(data->size() == 64);
    assert(tile);

    for(uint32_t i = 0; i < data->size(); i++) {
        if(debug) {
            std::cout << "pixel " << i << ": " << std::to_string(data->at(i).r) << " " << std::to_string(data->at(i).g)
                      << " " << std::to_string(data->at(i).b) << " " << std::to_string(data->at(i).a) << "\n";
        }
        if(data->at(i).a == 0x00) {
            //setting correct bits to 00
            tile->bit1[i / 8] = tile->bit1[i / 8] & ~(1 << i%8);
            tile->bit0[i / 8] = tile->bit0[i / 8] & ~(1 << i%8);
        }
        else if(data->at(i).b == 0xff) {
            //setting bits to 11
            tile->bit1[i / 8] = tile->bit1[i / 8] | (1 << i%8);
            tile->bit0[i / 8] = tile->bit0[i / 8] | (1 << i%8);
        }
        else if(data->at(i).g == 0xff) {
            //setting bits to 10
            tile->bit1[i / 8] = tile->bit1[i / 8] | (1 << i%8);
            tile->bit0[i / 8] = tile->bit0[i / 8] & ~(1 << i%8);
        }
        else if(data->at(i).r == 0xff) {
            //setting bits to 01
            tile->bit1[i / 8] = tile->bit1[i / 8] & ~(1 << i%8);
            tile->bit0[i / 8] = tile->bit0[i / 8] | (1 << i%8);
        }
        else {
            throw std::runtime_error("Asset improperly formed - no color assigned to pixel " + std::to_string(i));
        }
    }
}

void png_to_projectile_map(std::vector< glm::u8vec4 > *data, std::vector< PlayMode::ProjectileSet > *projectile_map, bool debug = false) {
    assert(data);
    assert(projectile_map);
    assert(data->size() % 8 == 0);

    u_int8_t bits = 0;
    for(int32_t i = data->size() - 8; i >= 0; i -= 8) {
        for(uint32_t j = 0; j < 8; j++) {
            u_int8_t bit = data->at(i + j).a == 0xff;
            bits = bit | (bits << 1);
            if(debug) {
                std::cout << bit;
            }
        }
        projectile_map->emplace_back();
        projectile_map->back().directions = bits;

        if(debug) {
            std::cout << '\n';
        }
    }
}

void add_tiles(std::vector<std::string> *filepaths, std::vector< PPU466::Tile > *tiles, bool debug = false) {
    std::vector< glm::u8vec4 > spriteData;
    glm::uvec2 size = glm::uvec2(8,8);
    for(const auto &filepath : *filepaths) {
        load_png(filepath, &size, &spriteData, LowerLeftOrigin);
        tiles->emplace_back();
        png_to_tile(&spriteData, &tiles->back(), debug);
    }
}

void add_projectile_map(std::string filepath, std::vector< PlayMode::ProjectileSet > *projectile_map, bool debug = false) {
    std::vector< glm::u8vec4 > spriteData;
    glm::uvec2 size = glm::uvec2(8,480);
    load_png(filepath, &size, &spriteData, LowerLeftOrigin);
    png_to_projectile_map(&spriteData, projectile_map, debug);
}

//from https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
void getFilenames(std::string directory, std::vector<std::string> *out) {
    for (const auto & entry : std::__fs::filesystem::directory_iterator(directory)) {
        if(entry.path() != directory + "/.DS_Store") {
            out->push_back(entry.path());
        }
    }
    //natural sort implementation by Martin Pool, details in strnatcmp.hpp
    std::sort(out->begin(), out->end(), compareNat);
}


int main(int argc, char **argv) {
    std::vector< PPU466::Tile > tiles;
    std::vector< PlayMode::ProjectileSet > projectile_map;

    std::ofstream tile_ofs;
    tile_ofs.open(data_path("dist/main.tiles"), std::fstream::trunc);
    std::ofstream projectile_map_ofs;
    projectile_map_ofs.open(data_path("dist/projectile.map"), std::fstream::trunc);

    if(!tile_ofs.is_open()) {
        std::cerr<<"Failed to open tile file : "<<errno<<std::endl;
        return -1;
    }
    if(!projectile_map_ofs.is_open()) {
        std::cerr<<"Failed to open map file : "<<errno<<std::endl;
        return -1;
    }

    std::vector< std::string > tile_filepaths;
    getFilenames(data_path("assets/tiles"), &tile_filepaths);

    add_tiles(&tile_filepaths, &tiles);
    add_projectile_map(data_path("assets/map.png"), &projectile_map);

    //add all tiles
//    add_tile("assets/tiles/player.png", &tiles);
//    add_tile("assets/tiles/projectile.png", &tiles);
//    add_tile("assets/tiles/blackhole1.png", &tiles);
//    add_tile("assets/tiles/blackhole2.png", &tiles);
//    add_tile("assets/tiles/blank.png", &tiles);
//    add_tile("assets/tiles/test.png", &tiles, true);
    write_chunk("til0", tiles, &tile_ofs);
    write_chunk("map0", projectile_map, &projectile_map_ofs);
    std::cout << "completed\n";
}

#endif
