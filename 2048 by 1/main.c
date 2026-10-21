#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <gb/gb.h>
#include <gb/drawing.h>
#include "Splash.h"
#include "Splash.c"

uint16_t rand_seed = 1337; // Simple seed for PRNG

bool bGameActive = false;

// 4 x 4 grid of cells, 0 = empty, >0 = value displayed in cell
uint8_t contents[4][4] = {
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};

uint8_t joypadCurrent = 0;
uint8_t joypadPrevious = 0;

uint8_t is_joypad_pressed(uint8_t button) {
    return (joypadCurrent & button) && !(joypadPrevious & button);
}

uint8_t is_joypad_released(uint8_t button) {
    return !(joypadCurrent & button) && (joypadPrevious & button);
}

void update_input(void) {
    joypadPrevious = joypadCurrent;
    joypadCurrent = joypad();
}

void draw_splash(void) {
    set_bkg_data(0, Splash_TILE_COUNT, Splash_tiles);
    set_bkg_tiles(0, 0, Splash_WIDTH / 8, Splash_HEIGHT / 8, Splash_map);
    SHOW_BKG;
}

void draw_grid(void) {
    // Draw grid border
    line(16, 8, 144, 8); // Top horizontal line
    line(16, 136, 144, 136); // Bottom horizontal line
    line(16, 8, 16, 136); // Left vertical line
    line(144, 8, 144, 136); // Right vertical line
    // Draw inner grid lines for a 4x4 grid
    line(48, 8, 48, 136); // First vertical line
    line(80, 8, 80, 136); // Second vertical line
    line(112, 8, 112, 136); // Third vertical line
    line(16, 40, 144, 40); // First horizontal line
    line(16, 72, 144, 72); // Second horizontal line
    line(16, 104, 144, 104); // Third horizontal line
}

void draw_cells(void) {
    for(uint8_t row = 0; row < 4; row++) {
        for(uint8_t col = 0; col < 4; col++) {
            if(contents[row][col] > 0) {
                // Calculate the position to draw the value
                uint8_t x = 3 + col * 4; // Adjusted for cell width
                uint8_t y = 3 + row * 4; // Adjusted for cell height
                gotogxy(x, y);
                char buffer[2];
                if(contents[row][col] > 9) {
                    sprintf(buffer, "%d", contents[row][col]);
                    gprint(buffer);
                }
                else {
                    sprintf(buffer, " %d", contents[row][col]);
                    gprint(buffer);
                }
            }
            else {
                // Clear the cell if it's empty
                uint8_t x = 3 + col * 4; // Adjusted for cell width
                uint8_t y = 3 + row * 4; // Adjusted for cell height
                gotogxy(x, y);
                gprint("  "); // Clear the cell by printing spaces
            }
        }
    }
}

uint16_t get_pseudo_rand(void) {
    rand_seed = (rand_seed * 1664525) + 1013904223;
    return rand_seed;
}

int get_empty_count(void) {
    int iCount = 0;
    for (int iRow = 0; iRow < 4; iRow++) {
        for (int iCol = 0; iCol < 4; iCol++) {
            if (contents[iRow][iCol] == 0) {
                iCount++;
            }
        }
    }
    return iCount;
}

bool get_next(void) {
    int empty_count = get_empty_count();
    if (empty_count > 0) {
        uint16_t rand_val = get_pseudo_rand() % empty_count;
        int iCount = 0;
        for (int iRow = 0; iRow < 4; iRow++) {
            for (int iCol = 0; iCol < 4; iCol++) {
                if (contents[iRow][iCol] == 0) {
                    if (iCount == rand_val) {
                        contents[iRow][iCol] = 1;
                    }
                    iCount++;
                }
            }
        }
        return true;
    }
    else {
        bGameActive = false;
        return false;
    }
}

bool shift_up(void) {
    bool bUpdated = false;
    
    // shift up
    for (int iCol = 0; iCol < 4; iCol++) {
        bool bShifted = true;
        while (bShifted) {
            bShifted = false;
            for (int iRow = 2; iRow >= 0; iRow--) {
                if ((contents[iRow][iCol] == 0) && (contents[iRow+1][iCol] != 0)) {
                    contents[iRow][iCol] = contents[iRow+1][iCol];
                    contents[iRow+1][iCol] = 0;
                    bShifted = true;
                    bUpdated = true;
                }
            }
        }
    }
    
    // combine
    bool bModified = false;
    for (int iCol = 0; iCol < 4; iCol++) {
        bool bCombined = true;
        while (bCombined) {
            bCombined = false;
            for (int iRow = 0; iRow < 3; iRow++) {
                if ((contents[iRow][iCol] == contents[iRow+1][iCol]) && (contents[iRow][iCol] != 0)) {
                    contents[iRow][iCol] += 1;
                    contents[iRow+1][iCol] = 0;
                    bCombined = true;
                    bModified = true;
                    bUpdated = true;
                }
            }
        }
    }
    
    // do it again if modified
    if (bModified) {
        shift_up();
    }
    return bUpdated;
}

bool shift_down(void) {
    bool bUpdated = false;
    
    // shift down
    for (int iCol = 0; iCol < 4; iCol++) {
        bool bShifted = true;
        while (bShifted) {
            bShifted = false;
            for (int iRow = 0; iRow < 3; iRow++) {
                if ((contents[iRow+1][iCol] == 0) && (contents[iRow][iCol] != 0)) {
                    contents[iRow+1][iCol] = contents[iRow][iCol];
                    contents[iRow][iCol] = 0;
                    bShifted = true;
                    bUpdated = true;
                }
            }
        }
    }
    
    // combine
    bool bModified = false;
    for (int iCol = 0; iCol < 4; iCol++) {
        bool bCombined = true;
        while (bCombined) {
            bCombined = false;
            for (int iRow = 2; iRow >= 0; iRow--) {
                if ((contents[iRow][iCol] == contents[iRow+1][iCol]) && (contents[iRow][iCol] != 0)) {
                    contents[iRow+1][iCol] += 1;
                    contents[iRow][iCol] = 0;
                    bCombined = true;
                    bModified = true;
                    bUpdated = true;
                }
            }
        }
    }
    
    // do it again if modified
    if (bModified) {
        shift_down();
    }
    return bUpdated;
}
        
bool shift_left(void) {
    bool bUpdated = false;
    
    // shift left
    for (int iRow = 0; iRow < 4; iRow++) {
        bool bShifted = true;
        while (bShifted) {
            bShifted = false;
            for (int iCol = 0; iCol < 3; iCol++) {
                if ((contents[iRow][iCol] == 0) && (contents[iRow][iCol+1] != 0)) {
                    contents[iRow][iCol] = contents[iRow][iCol+1];
                    contents[iRow][iCol+1] = 0;
                    bShifted = true;
                    bUpdated = true;
                }
            }
        }
    }
    
    // combine
    bool bModified = false;
    for (int iRow = 0; iRow < 4; iRow++) {
        bool bCombined = true;
        while (bCombined) {
            bCombined = false;
            for (int iCol = 0; iCol < 3; iCol++) {
                if ((contents[iRow][iCol] == contents[iRow][iCol+1]) && (contents[iRow][iCol] != 0)) {
                    contents[iRow][iCol] += 1;
                    contents[iRow][iCol+1] = 0;
                    bCombined = true;
                    bModified = true;
                    bUpdated = true;
                }
            }
        }
    }
    
    // do it again if modified
    if (bModified) {
        shift_left();
    }
    return bUpdated;
}

bool shift_right(void) {
    bool bUpdated = false;
    
    // shift right
    for (int iRow = 0; iRow < 4; iRow++) {
        bool bShifted = true;
        while (bShifted) {
            bShifted = false;
            for (int iCol = 2; iCol >= 0; iCol--) {
                if ((contents[iRow][iCol+1] == 0) && (contents[iRow][iCol] != 0)) {
                    contents[iRow][iCol+1] = contents[iRow][iCol];
                    contents[iRow][iCol] = 0;
                    bShifted = true;
                    bUpdated = true;
                }
            }
        }
    }
    
    // combine
    bool bModified = false;
    for (int iRow = 0; iRow < 4; iRow++) {
        bool bCombined = true;
        while (bCombined) {
            bCombined = false;
            for (int iCol = 2; iCol >= 0; iCol--) {
                if ((contents[iRow][iCol] == contents[iRow][iCol+1]) && (contents[iRow][iCol] != 0)) {
                    contents[iRow][iCol+1] += 1;
                    contents[iRow][iCol] = 0;
                    bCombined = true;
                    bModified = true;
                    bUpdated = true;
                }
            }
        }
    }
    
    // do it again if modified
    if (bModified) {
        shift_right();
    }
    return bUpdated;
}

void main(void) { 
    // Draw the splash screen
    draw_splash();
    //game loop 
    while(1) {
        // Wait for vertical blanking interval
        vsync();
        // Update input state
        update_input();
        if(bGameActive) {
            // Check for button presses and releases
            if(is_joypad_pressed(J_UP)) {
                // Handle UP button press
                if(shift_up()) {
                    if(get_next()) draw_cells();
                }
                else {
                    bGameActive = get_empty_count() != 0;
                }
            }
            if(is_joypad_pressed(J_DOWN)) {
                // Handle DOWN button press
                if(shift_down()) {
                    if(get_next()) draw_cells();
                }
                else {
                    bGameActive = get_empty_count() != 0;
                }
            }
            if(is_joypad_pressed(J_LEFT)) {
                // Handle LEFT button press
                if(shift_left()) {
                    if(get_next()) draw_cells();
                }
                else {
                    bGameActive = get_empty_count() != 0;
                }
            }
            if(is_joypad_pressed(J_RIGHT)) {
                // Handle RIGHT button press
                if(shift_right()) {
                    if(get_next()) draw_cells();
                }
                else {
                    bGameActive = get_empty_count() != 0;
                }
            }
        }
        else {
            if(is_joypad_pressed(J_START)) {
                // Handle START button press
                bGameActive = true;
                for(int y = 0; y < 4; y++) {
                    for(int x = 0; x < 4; x++) {
                        contents[y][x] = 0;
                    }
                }
                // Draw the grid
                draw_grid();
                // Initialize the grid with a random cell
                get_next();
                // Draw the initial state of the cells
                draw_cells();
            }
        }
    }
}
 