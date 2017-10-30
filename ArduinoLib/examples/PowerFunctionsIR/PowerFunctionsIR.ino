/*
 * PowerFunctionsIR.ino - Example sketch to document the usage of PowerFunctionsIR lib
 * Copyright© 2017 by Heiko Finzel
 * The Brixx Library and all other content of this project is distributed under the terms and conditions
 * of the GNU GENERAL PUBLIC LICENSE Version 3.
 * Note: Although this project aims to connect LEGO Power Functions hardware to Arduino without
 * damaging anything else then just a few extension cables, I'll take no responsibility if it happens
 * afterall. Your warranty for your LEGO Power Functions items may void using this project.
 */
#include <PowerFunctionsIR.h>

/*
 * This signature is used for all event handlers.
 * IRSample represent the raw signal sample and offers methods and attributes to access the data.
 * ChannelState offers access to state values for this channel which the PowerFunctionsIR lib keeps
 * track of.
 */
void myhandler(PowerFunctionsIR::IRSample &ir, PowerFunctionsIR::ChannelState &ch) {
  /*
   * Get the rc channel (1-4), this is the channel you choose on your remote control.
   * If you need channel - 1 e.g. for array index use ir.channel attribute instead, since
   * ir.get_channel() is actually an ir.channel + 1.
   */
  Serial.print("Channel: ");
  Serial.print(ir.get_channel());
  /*
   * Determine the effected "subchannel(s)" (red and/or blue).
   * Note that the standard remote control will always effect both "subchannels".
   */
  Serial.print("  Subchannel: ");
  if (ir.red_effected() && ir.blue_effected()) {
    Serial.print("both");
  } else if (ir.red_effected()) {
    Serial.print("red ");
  } else { // if ir.blue_effected()
    Serial.print("blue");
  }
  /*
   * Determine which kind of remote control was used, the LEGO Power Functions standard 
   * control (the one with the 2 joysticks) or the pwm control (the one with the speed
   * control wheels).
   * This could also be used to use the same channel for both types but drive different
   * functions with them.
   */
  Serial.print("  RCtype: ");
  if (ir.standard_rc()) {
    Serial.print("standard");
  // actually this could be a plain else since there only are those 2 remote controls at the moment
  } else if (ir.pwm_rc()) { 
    Serial.print("pwm     ");
  }
  /*
   * The attribute ir.data can get you the raw 4 data bits (actual command) of the
   * received signal. The stuff around is just to provide leading zeros for better
   * readability.
   */
  Serial.print("  Data: 0x");
  Serial.print(ir.data, HEX);
  /*
   * Note that the same command (data bits) has different meanings depending on if
   * it came from standard or pwm remote control.
   * Therefore and since the raw data bits aren't very human readable, I added some 
   * functionality for convenience, one of which is the ir.get_command() helper function
   * and the following literals to compare the command to.
   * (For a LEGO Power Functions motor FORWARD means clockwise, BACKWARD means counter
   * clockwise, INCREASE means "more clockwise" and DECREASE means "more counter clockwise".
   * Altering the direction switch on the remote control will just swap the commands sent,
   * so the commands themself still mean the same.)
   */
  Serial.print("  Command: ");
  switch (ir.get_command()) {
    case RED_STOP_BLUE_STOP:
      Serial.print("Red: stop ... Blue: stop");
      break;
    case RED_FORWARD_BLUE_STOP:
      Serial.print("Red: forward ... Blue: stop");
      break;
    case RED_BACKWARD_BLUE_STOP:
      Serial.print("Red: backward ... Blue: stop");
      break;
    case RED_STOP_BLUE_FORWARD:
      Serial.print("Red: stop ... Blue: forward");
      break;
    case RED_FORWARD_BLUE_FORWARD:
      Serial.print("Red: forward ... Blue: forward");
      break;
    case RED_BACKWARD_BLUE_FORWARD:
      Serial.print("Red: backward ... Blue: forward");
      break;
    case RED_STOP_BLUE_BACKWARD:
      Serial.print("Red: stop ... Blue: backward");
      break;
    case RED_FORWARD_BLUE_BACKWARD:
      Serial.print("Red: forward ... Blue: backward");
      break;
    case RED_BACKWARD_BLUE_BACKWARD:
      Serial.print("Red: backward ... Blue: backward");
      break;
    case INCREASE_VALUE:
      Serial.print("increase value");
      break;
    case DECREASE_VALUE:
      Serial.print("decrease value");
      break;
    case RESET_VALUE:
      Serial.print("reset value");
      break;
  }
  Serial.println();
  // ...
  Serial.print("Red: ");
  Serial.print( ch.red.value );
  Serial.print("  Blue: ");
  Serial.println( ch.blue.value );
  /*
   * If you set this to true, the event is not passed to any further event handlers.
   * If you want the event to be processed by further event handlers you don't have to 
   * care for this attribute, since it defaults to false.
   */
  ir.handled = true;
}

void setup() {
  Serial.begin(9600);
  /*
   * Register your event handlers.
   * An event is passed to event handlers in the same order as handlers are described here.
   * 
   * generic_handler is triggered for every IR event sampled, regardless of its type or source.
   */
  PowerFunctionsIR::generic_handler = myhandler;
  /*
   * Initialize PowerFunctionsIR.
   * This is start listening for IR signals and when successfully sampled a signal enqueue
   * it for further processing.
   */
  PowerFunctionsIR::init();
}

void loop() {
  /*
   * Add this to your loop() to process the event queue.
   * Note if you initialized PowerFunctionsIR with a call to init() in your setup(), but don't
   * call update() here signals are still sampled and enqueued resulting in an out-of-memory
   * issue after some time and of course your event handlers won't be triggered.
   */
  PowerFunctionsIR::update();
  /*
   * If your board keeps hanging on IR inputs after some time this is due to a too fast loop().
   * Add a short delay or build a non-blocking delay around the update() function to resolve this issue.
   */
  delay(1);
}
