/*
 * PowerFunctionsIR.ino - Example sketch to document the usage of PowerFunctionsIR lib
 * Copyright© 2017 by Heiko Finzel
 * The Brixx Library and all other content of this project is distributed under the terms and conditions
 * of the GNU GENERAL PUBLIC LICENSE Version 3.
 * Note: Although this project aims to connect LEGO Power Functions hardware to Arduino without
 * damaging anything else then just a few extension cables, I'll take no responsibility if it happens
 * afterall. Your warranty for your LEGO Power Functions items may void using this project.
 *
 * You could also use
 * #include <Brixx.h>
 * instead of PowerFunctionsIR.h to include all the Brixx functionality.
 */
#include <PowerFunctionsIR.h>

/*
 * This signature is used for all event handlers.
 * IRSample represent the raw signal sample and offers methods and attributes to access the data.
 * ChannelState offers access to state values for this channel which the PowerFunctionsIR lib keeps
 * track of.
 */
void my_generic_handler(PowerFunctionsIR::IRSample &ir, PowerFunctionsIR::ChannelState &ch) {
  /*
   * Get the rc channel, this is the channel you choose on your LEGO Power Functions 
   * remote control. The channels are zero based (0-3), so to get the channel as printed
   * on your remote control we have to add 1 to get 1-4.
   * 0-3 here = 1-4 on remote control
   */
  Serial.print("Channel: ");
  Serial.print(ir.get_channel() + 1);
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
   * functions with them. However you should use value() funtions of ChannelState carefully 
   * then since this may have unintended side effects, like input on standard rc red will
   * also clear any value for blue (since standard rc always effects both subchannels).
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
   * received signal.
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
  /*
   * As another convenience function a ChannelState is passed to event handlers.
   * This holds values especially for the effected channel that the PowerFunctionsIR
   * lib keeps track of. Here we retrieve the values for both subchannels (red & blue).
   * These values range between -255 (full backward) and +255 (full forward) and can
   * be used as input for the PowerFunctionsOutput class and its subclasses.
   * These values are most powerful if used in red/blue_changed_handler event handlers,
   * for details see below.
   */
  Serial.print(" ... Red value is: ");
  Serial.print(ch.red.value());
  Serial.print(" ... Blue value is: ");
  Serial.println(ch.blue.value());
  /*
   * If you set this to true, the event is not passed to any further event handlers.
   * If you want the event to be processed by further event handlers you don't have to 
   * care for this attribute, since it defaults to false.
   */
  //ir.handled = true;
}

/*
 * This event handler is assigned to PowerFunctionsIR::red_effected_handler[0] in setup(), which is
 * especially triggered for events on channel 1 subchannel red, there are identical handlers
 * for the other channels and blue subchannel, too.
 */
void my_ch1red_handler(PowerFunctionsIR::IRSample &ir, PowerFunctionsIR::ChannelState &ch) {
  /*
   * ir.get_red_command() and ir.get_blue_command() will return command codes that can be interpreted
   * for each subchannel without considering the other. They are especially useful in chXred/blue 
   * handlers but could be used in generic handler aswell.
   */
  Serial.print("Channel 1 red effected  ... Command: ");
  switch (ir.get_red_command()) {
    /*
     * These are sent from standard remote control
     */
    case STOP:
      Serial.print("stop");
      break;
    case FORWARD:
      Serial.print("forward");
      break;
    case BACKWARD:
      Serial.print("backward");
      break;
    /*
     * The call could return NO_COMMAND if the IR signal was meant for the other (blue) channel only,
     * of course this can't really happen here, since the whole event handler is especially for red.
     */
    case NO_COMMAND:
      Serial.print("command was for blue channel only ...");
      break;
    /*
     * These are sent from pwm remote control
     */
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
}

/*
 * Same as above, but for channel 1 blue
 */
void my_ch1blue_handler(PowerFunctionsIR::IRSample &ir, PowerFunctionsIR::ChannelState &ch) {
  Serial.print("Channel 1 blue effected ... Command: ");
  switch (ir.get_blue_command()) {
    case STOP:
      Serial.print("stop");
      break;
    case FORWARD:
      Serial.print("forward");
      break;
    case BACKWARD:
      Serial.print("backward");
      break;
    case NO_COMMAND:
      Serial.print("command was for red channel only ...");
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
}

/*
 * This event handler is assigned to PowerFunctionsIR::red_changed_handler[0] in setup(), which is
 * especially triggered if the value associated with channel 1 subchannel red changes, there are 
 * identical handlers for the other channels and blue subchannel, too.
 */
void my_ch1red_changed_handler(PowerFunctionsIR::IRSample &ir, PowerFunctionsIR::ChannelState &ch) {
  /*
   * Get the value for red subchannel.
   */
  Serial.print("Red value changed, new value is: ");
  Serial.println(ch.red.value());
}

/*
 * Same as above, but for channel 1 blue
 */
void my_ch1blue_changed_handler(PowerFunctionsIR::IRSample &ir, PowerFunctionsIR::ChannelState &ch) {
  Serial.print("Blue value changed, new value is: ");
  Serial.println(ch.blue.value());
}

void setup() {
  Serial.begin(9600);
  /*
   * Register your event handlers.
   * An event is passed to event handlers in the same order as handlers are described here.
   * 
   * generic_handler is triggered for every IR event sampled, regardless of its type or source.
   * red_effected_handler[0-3] is triggered for every IR event that effects the red subchannel of the channel in question (use index 0-3 for channels 1-4).
   * blue_effected_handler[0-3] is triggered for every IR event that effects the blue subchannel of the channel in question (use index 0-3 for channels 1-4).
   * red_changed_handler[0-3] is triggered if the IR event changes the value associated with red subchannel of the channel in question (use index 0-3 for channels 1-4).
   * blue_changed_handler[0-3] is triggered if the IR event changes the value associated with blue subchannel of the channel in question (use index 0-3 for channels 1-4).
   */
  PowerFunctionsIR::generic_handler = my_generic_handler;
  /*
   * We use index 0 to add handlers for channel 0, which is channel 1 on our remote control (remember that channels are zero based here in code, so
   * we have to add 1 to get tha channel number as it is printed on LEGO Power Functions remote control).
   * 0-3 here = 1-4 on remote control
   */
  PowerFunctionsIR::red_effected_handler[0] = my_ch1red_handler;
  PowerFunctionsIR::blue_effected_handler[0] = my_ch1blue_handler;
  PowerFunctionsIR::red_changed_handler[0] = my_ch1red_changed_handler;
  PowerFunctionsIR::blue_changed_handler[0] = my_ch1blue_changed_handler;
  
  /*
   * Initialize PowerFunctionsIR.
   * This is start listening for IR signals and when successfully sampled a signal enqueue
   * it for further processing.
   */
  PowerFunctionsIR::init();
  /*
   * Call this *after* init() to adjust steps for the pwm remote control.
   * 1st argument is the channel (1-4), 2nd is the amount of steps from 0 to full (1-127) in
   * both directions for the red subchannel, 3rd is the same for blue subchannel.
   * Default for all channels and subchannels is 7 (as it is for the original LEGO Power Functions
   * IR receiver).
   * If any argument is out of range false is returned and nothing is adjusted.
   * Here we set channel 1 blue to have 20 steps from 0 to full.
   * These steps effect the internal tracking values you can get with ch.red.value() and ch.blue.value()
   * within your event handlers.
   */
  bool success = PowerFunctionsIR::set_steps(1, 3, 20);
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

/* Sample output
 * 
 * Standard rc - red up
 * 
Channel: 1  Subchannel: both  RCtype: standard  Data: 0x1  Command: Red: forward ... Blue: stop ... Red value is: 255 ... Blue value is: 0
Channel 1 red effected  ... Command: forward
Channel 1 blue effected ... Command: stop
Red value changed, new value is: 255
 *
 * Standard rc - blue down (while red is still up)
 * 
Channel: 1  Subchannel: both  RCtype: standard  Data: 0x9  Command: Red: forward ... Blue: backward ... Red value is: 255 ... Blue value is: -255
Channel 1 red effected  ... Command: forward
Channel 1 blue effected ... Command: backward
Blue value changed, new value is: -255
 * 
 * Standard rc - let lose both
 * 
Channel: 1  Subchannel: both  RCtype: standard  Data: 0x0  Command: Red: stop ... Blue: stop ... Red value is: 0 ... Blue value is: 0
Channel 1 red effected  ... Command: stop
Channel 1 blue effected ... Command: stop
Red value changed, new value is: 0
Blue value changed, new value is: 0
 * 
 * Pwm rc - red 1 turn clockwise
 * 
Channel: 1  Subchannel: red   RCtype: pwm       Data: 0x4  Command: increase value ... Red value is: 85 ... Blue value is: 0
Channel 1 red effected  ... Command: increase value
Red value changed, new value is: 85
 * 
 * Pwm rc - red 1 turn clockwise
 * 
Channel: 1  Subchannel: red   RCtype: pwm       Data: 0x4  Command: increase value ... Red value is: 170 ... Blue value is: 0
Channel 1 red effected  ... Command: increase value
Red value changed, new value is: 170
 * 
 * Pwm rc - red 1 turn clockwise
 * 
Channel: 1  Subchannel: red   RCtype: pwm       Data: 0x4  Command: increase value ... Red value is: 255 ... Blue value is: 0
Channel 1 red effected  ... Command: increase value
Red value changed, new value is: 255
 * 
 * Pwm rc - red 1 turn clockwise - Note: no changed value since we already reached 255
 * 
Channel: 1  Subchannel: red   RCtype: pwm       Data: 0x4  Command: increase value ... Red value is: 255 ... Blue value is: 0
Channel 1 red effected  ... Command: increase value
 * 
 * Pwm rc - red 1 turn counter clockwise
 * 
Channel: 1  Subchannel: red   RCtype: pwm       Data: 0x5  Command: decrease value ... Red value is: 170 ... Blue value is: 0
Channel 1 red effected  ... Command: decrease value
Red value changed, new value is: 170
 * 
 * Pwm rc - red 1 turn counter clockwise
 * 
Channel: 1  Subchannel: red   RCtype: pwm       Data: 0x5  Command: decrease value ... Red value is: 85 ... Blue value is: 0
Channel 1 red effected  ... Command: decrease value
Red value changed, new value is: 85
 * 
 * Pwm rc - red 1 turn counter clockwise - back to 0
 * 
Channel: 1  Subchannel: red   RCtype: pwm       Data: 0x5  Command: decrease value ... Red value is: 0 ... Blue value is: 0
Channel 1 red effected  ... Command: decrease value
Red value changed, new value is: 0
 * 
 * Pwm rc - red 1 reset button - Note: no changed value since we already are back to 0
 * 
Channel: 1  Subchannel: red   RCtype: pwm       Data: 0x8  Command: reset value ... Red value is: 0 ... Blue value is: 0
Channel 1 red effected  ... Command: reset value
 * 
 * Pwm rc - red 1 turn clockwise
 *
Channel: 1  Subchannel: red   RCtype: pwm       Data: 0x4  Command: increase value ... Red value is: 85 ... Blue value is: 0
Channel 1 red effected  ... Command: increase value
Red value changed, new value is: 85
 * 
 * Pwm rc - blue 1 turn clockwise - Note: value is only 12 since we set steps to 20, while red has 3 steps
 *
Channel: 1  Subchannel: blue  RCtype: pwm       Data: 0x4  Command: increase value ... Red value is: 85 ... Blue value is: 12
Channel 1 blue effected ... Command: increase value
Blue value changed, new value is: 12
 * 
 * Pwm rc - red 1 reset button
 * 
Channel: 1  Subchannel: red   RCtype: pwm       Data: 0x8  Command: reset value ... Red value is: 0 ... Blue value is: 12
Channel 1 red effected  ... Command: reset value
Red value changed, new value is: 0
 * 
 * Standard rc - red up - Note: blue goes back to 0 since both standard and pwm remote controls are set to the same channel
 *
Channel: 1  Subchannel: both  RCtype: standard  Data: 0x1  Command: Red: forward ... Blue: stop ... Red value is: 255 ... Blue value is: 0
Channel 1 red effected  ... Command: forward
Channel 1 blue effected ... Command: stop
Red value changed, new value is: 255
Blue value changed, new value is: 0
 * 
 * Standard rc - let lose red
 * 
Channel: 1  Subchannel: both  RCtype: standard  Data: 0x0  Command: Red: stop ... Blue: stop ... Red value is: 0 ... Blue value is: 0
Channel 1 red effected  ... Command: stop
Channel 1 blue effected ... Command: stop
Red value changed, new value is: 0
 * 
 * Standard rc on channel 2 - red up - Note: We only get 1 output line from generic handler since we didn't set any other handlers for channel 2
 * (same applies for channels 3 and 4)
 *
Channel: 2  Subchannel: both  RCtype: standard  Data: 0x1  Command: Red: forward ... Blue: stop ... Red value is: 255 ... Blue value is: 0
 */
