/*
 * BrixxSettings.h - Setting definitions for the Brixx library.
 * Copyright© 2017 by Heiko Finzel
 * The Brixx Library and all other content of this project is distributed under the terms and conditions
 * of the GNU GENERAL PUBLIC LICENSE Version 3.
 * Note: Although this project aims to connect LEGO Power Functions hardware to Arduino without
 * damaging anything else then just a few extension cables, I'll take no responsibility if it happens
 * afterall. Your warranty for your LEGO Power Functions items may void using this project.
 *
 * If you add or remove pin-pair definitions, you may also want to edit keywords.txt.
 */
#pragma once

/*
    PowerFunctionsIR
*/
#define IR_SAMPLE_INTERRUPT_PIN 18
#define DEFAULT_STEPS            7

/*
    PowerFunctionsOutput
    Pin ports for PowerFunctionsOutput classes.
    Pairs of analog (PWM) or digital pins for output to LEGO connectors (c1, c2).
*/

// PWM output pin pairs
#define PF_OUT_A1  2,  3
#define PF_OUT_A2  4,  5
#define PF_OUT_A3  6,  7
#define PF_OUT_A4  8,  9
#define PF_OUT_A5 10, 11
#define PF_OUT_A6 12, 13
#define PF_OUT_A7 44, 45

// digital output pin pairs
#define PF_OUT_D1 42, 43
#define PF_OUT_D2 40, 41
#define PF_OUT_D3 38, 39
