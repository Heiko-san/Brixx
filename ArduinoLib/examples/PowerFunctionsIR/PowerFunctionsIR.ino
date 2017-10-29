#include <PowerFunctionsIR.h>

void myhandler(PowerFunctionsIR::IRSample &ir) {
  Serial.print("Toggle: "); 
  Serial.print( ir.toggle, BIN );
  Serial.print("  Channel: ");
  Serial.print( ir.get_channel() );
  Serial.print("  Mode: ");
  if ( ir.extended_mode() ) {
    Serial.print("extended_mode");
  } else if ( ir.combo_direct_mode() ) {
    Serial.print("combo_direct_mode");
  } else if ( ir.combo_pwm_mode() ) {
    Serial.print("combo_pwm_mode");
  } else if ( ir.single_output_mode_pwm() ) {
    Serial.print("single_output_mode_pwm");
  } else if ( ir.single_output_mode_toggle() ) {
    Serial.print("single_output_mode_toggle");
  } else {
    Serial.print("unkown");
  }
  if ( ir.single_output_mode() ) Serial.print( ir.get_single_output_port() );
  Serial.print("  Data: "); 
  Serial.println( ir.data, BIN );
  
  ir.handled = true;
}

void setup() {
  Serial.begin(9600);
  PowerFunctionsIR::generic_handler = myhandler;
  PowerFunctionsIR::init();
}

void loop() {
  PowerFunctionsIR::update();
  delay(1);
}
