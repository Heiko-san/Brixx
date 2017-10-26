/*
  BrixxSettings.h - Setting definitions for the Brixx library.
  Created by Heiko Finzel, 2017.
  If you add or remove pin-pair definitions, you may want to also edit keywords.txt.
*/
#pragma once
#include "Arduino.h"

/*
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
