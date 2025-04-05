/*
 * Copyright (C) 2023 Giovanni di Dio Bruno
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "pio_encoder.h"

// Initialize static members
uint PioEncoder::offset = 0;
bool PioEncoder::not_first_instance = false;

PioEncoder::PioEncoder(const uint8_t pin, const bool flip, const int zero_offset, 
                       const uint8_t count_mode, PIO pio, const uint sm, const int max_step_rate)
  : pin(pin),
    pio(pio),
    sm(sm),
    max_step_rate(max_step_rate),
    zero_offset(zero_offset),
    count_mode(count_mode),
    flip_it(flip ? -1 : 1) // Set flip_it based on the flip parameter
{
    // No local shadowing of offset
}

void PioEncoder::begin() {
    // Configure encoder pins as inputs
    pinMode(pin, INPUT);
    pinMode(pin + 1, INPUT);

    // Load the program only once
    if (!not_first_instance) {
        offset = pio_add_program(pio, &quadrature_encoder_program);
        not_first_instance = true;
    }

    // Claim an unused state machine if needed
    if (sm == static_cast<uint>(-1)) {
        sm = pio_claim_unused_sm(pio, true);
    }

    // Initialize the state machine with the encoder program
    quadrature_encoder_program_init(pio, sm, offset, pin, max_step_rate);
}

void PioEncoder::reset(const int reset_value) {
    quadrature_encoder_reset(pio, sm);
    zero_offset = reset_value;
}

void PioEncoder::flip(const bool x) {
    // Use a ternary operator for conciseness
    flip_it = x ? -1 : 1;
}

void PioEncoder::setMode(const uint8_t mode) {
    count_mode = mode;
}

int PioEncoder::getCount() {
    // Shift the raw count based on the mode, apply flip factor and add zero offset
    return flip_it * (quadrature_encoder_get_count(pio, sm) >> count_mode) + zero_offset;
}
