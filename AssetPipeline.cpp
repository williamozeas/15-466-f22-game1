//
// Created by William Ozeas on 9/4/22.
//
#include "AssetPipeline.hpp"
#include "PPU466.hpp"
#include "read_write_chunk.hpp"
#include "load_save_png.hpp"
#include <glm/glm.hpp>
#include <fstream>

//load png
//convert data to tile format
//use write_chunk to save to file

//r, g, b, a channels used to correspond to 11, 10, 01, and 00 channels respectively
//correctly formed assets should only consist of colors #000000 #ff0000 #00ff00 and #0000ff
void png_to_tile(std::vector< glm::u8vec4 > *data, PPU466::Tile *tile) {
    assert(data);
    assert(data->size() == 64);
    assert(tile);
    //IF IMAGE IS UPSIDE DOWN REVERSE DATA

    for(uint32_t i = 0; i < data->size(); i++) {
        if(data->at(i).z == 0x00) {
            //setting correct bits to 00
            tile->bit1[i % 8] = tile->bit1[i % 8] & ~(1 << i/8);
            tile->bit0[i % 8] = tile->bit0[i % 8] & ~(1 << i/8);
        }
        else if(data->at(i).w == 0xff) {
            //setting bits to 11
            tile->bit1[i % 8] = tile->bit1[i % 8] | (1 << i/8);
            tile->bit0[i % 8] = tile->bit0[i % 8] | (1 << i/8);
        }
        else if(data->at(i).x == 0xff) {
            //setting bits to 11
            tile->bit1[i % 8] = tile->bit1[i % 8] | (1 << i/8);
            tile->bit0[i % 8] = tile->bit0[i % 8] & ~(1 << i/8);
        }
        else if(data->at(i).y == 0xff) {
            //setting bits to 11
            tile->bit1[i % 8] = tile->bit1[i % 8] | (1 << i/8);
            tile->bit0[i % 8] = tile->bit0[i % 8] | (1 << i/8);
        }
        else {
            throw std::runtime_error("Asset improperly formed - no color assigned to pixel " + std::to_string(i));
        }
    }
}

void add_tile(std::string filepath, std::vector< PPU466::Tile > tiles) {
    tiles.emplace_back();
    std::vector< glm::u8vec4 > spriteData;
    glm::uvec2 size = glm::uvec2(8,8);
    load_png(filepath, &size, &spriteData, UpperLeftOrigin);
    png_to_tile(&spriteData, &tiles.back());
}

int main(int argc, char **argv) {

    std::ofstream ofs("dist/assets/main.tiles", std::ofstream::out);
    std::vector< PPU466::Tile > tiles;

    //add all tiles
    add_tile("assets/player.png", tiles);

    write_chunk("str1", tiles, &ofs);
}


