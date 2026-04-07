#define ntc_pin A0         // Pin,to which the voltage divider is connected
#define vd_power_pin 2     // 5V for the voltage divider
#define heatpad_pin 3
#define nominal_resistance 10000       //Nominal resistance at 25⁰C
#define nominal_temeprature 25         // temperature for nominal resistance (almost always 25⁰ C)
#define samplingrate 5                 // Number of samples
#define beta 3950                      // The beta coefficient
#define Rref 10000                     //Value of resistor used for the voltage divider


int samples = 0;   // ✅ CHANGED: kept as int, but we treat it safely as sum of ADC readings


// ➕ ADDED: Simple thermostat control parameters
const float setpointC = 35.0;          // target temperature (change this)
const float hysteresisC = 1.0;         // +/- band around setpoint to prevent chatter
const int heaterPWM = 200;             // PWM power level when ON (0..255)


// ➕ ADDED: keeps heater state so it doesn't rapidly toggle near the setpoint
bool heaterOn = false;


// ➕ ADDED: moved thermistor reading + conversion into a helper function
float readThermistorC() {
  samples = 0;


  digitalWrite(vd_power_pin, HIGH);
  delay(5);                            // ➕ ADDED: let divider settle before sampling


  for (uint8_t i = 0; i < samplingrate; i++) {
    samples += analogRead(ntc_pin);
    delay(10);
  }


  digitalWrite(vd_power_pin, LOW);


  // ✅ CHANGED: force float math so we don't lose precision in division
  float adc = samples / (float)samplingrate;


  // ➕ ADDED: protect against divide-by-zero / edge cases
  if (adc <= 0.5) adc = 0.5;
  if (adc >= 1022.5) adc = 1022.5;


  // ✅ CHANGED: renamed meaningfully and computed thermistor resistance more explicitly
  // (same formula you used, but with safer float handling)
  float rTherm = Rref / (1023.0 / adc - 1.0);


  float temperature;


  // ✅ CHANGED: use rTherm instead of reusing "average" for multiple meanings
  temperature = rTherm / nominal_resistance;     // (R/Ro)
  temperature = log(temperature);                // ln(R/Ro)
  temperature /= beta;                           // 1/B * ln(R/Ro)
  temperature += 1.0 / (nominal_temeprature + 273.15); // + (1/To)
  temperature = 1.0 / temperature;               // invert
  temperature -= 273.15;                         // K -> C


  return temperature;                            // ➕ ADDED: return temp in C
}


void setup(void) {
  pinMode(vd_power_pin, OUTPUT);
  Serial.begin(9600);
  pinMode(heatpad_pin, OUTPUT);


  // ➕ ADDED: ensure known safe startup states
  digitalWrite(vd_power_pin, LOW);
  analogWrite(heatpad_pin, 0);
}


void loop(void) {
  // 🗑️ REMOVED: the old inline thermistor read/convert code block
  // ✅ CHANGED: replace it with a function call:
  float temperature = readThermistorC();


  // ➕ ADDED: thermostat control with hysteresis
  if (temperature <= (setpointC - hysteresisC)) {
    heaterOn = true;
  } else if (temperature >= (setpointC + hysteresisC)) {
    heaterOn = false;
  }


  // ✅ CHANGED: THIS is the key behavior change — heater output now depends on temperature
  // (Your original code always forced analogWrite(..., 0) every loop.)
  analogWrite(heatpad_pin, heaterOn ? heaterPWM : 0);


  // ✅ CHANGED: simplified debug prints to show temp + heater state (instead of raw resistance too)
  Serial.print("Temperature ");
  Serial.print(temperature, 2);
  Serial.print(" *C | Heater: ");
  Serial.println(heaterOn ? "ON" : "OFF");


  // ✅ CHANGED: faster update loop improves control responsiveness
  delay(500);  // was 2000
}
