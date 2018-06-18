/*
 * Brixx.h - Library for driving LEGO Power Functions hardware with Arduino Mega 
 * Copyright© 2017 by Heiko Finzel                                                                                      
 * The Brixx Library and all other content of this project is distributed under the terms and conditions of the         
 * GNU GENERAL PUBLIC LICENSE Version 3.                                                                                
 * Note: Although this project aims to connect LEGO Power Functions hardware to Arduino without damaging anything else  
 * then just a few extension cables, I'll take no responsibility for any damage done to any of your hardware.           
 * Warranty for your LEGO Power Functions items may void using this project.
 *
 * OOP classes are defined in separate header and code files and included here.
 * You only need to include Brixx.h into your project.
 */
#pragma once
// software version
#define BRIXX_VERSION 0.01

#include "BrixxSettings.h"
#include "PowerFunctionsIR.h"
#include "PowerFunctionsOutput.h"
//#include "PowerFunctionsMotor.h"
