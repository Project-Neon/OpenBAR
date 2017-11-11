 /* MPU6050 Basic Example Code
  by: Kris Winer
  date: May 1, 2014
  license: Beerware - Use this code however you'd like. If you
  find it useful you can buy me a beer some time.

  Demonstrate  MPU-6050 basic functionality including initialization, accelerometer trimming, sleep mode functionality as well as
  parameterizing the register addresses. Added display functions to allow display to on breadboard monitor.
  No DMP use. We just want to get out the accelerations, temperature, and gyro readings.

  SDA and SCL should have external pull-up resistors (to 3.3V).
  10k resistors worked for me. They should be on the breakout
  board.

  Hardware setup:
  MPU6050 Breakout --------- Arduino
  3.3V --------------------- 3.3V
  SDA ----------------------- A4
  SCL ----------------------- A5
  GND ---------------------- GND

   Note: The MPU6050 is an I2C sensor and uses the Arduino Wire library.
  Because the sensor is not 5V tolerant, we are using a 3.3 V 8 MHz Pro Mini or a 3.3 V Teensy 3.1.
  We have disabled the internal pull-ups used by the Wire library in the Wire.h/twi.c utility file.
  We are also using the 400 kHz fast I2C mode by setting the TWI_FREQ  to 400000L /twi.h utility file.
  */

 #include "mbed.h"
 #include "MPU6050.h"
// #include "Motor.h"
 //#include "QEI.h"
 #include "_config.h"
 #include "Ultrasonic.h"
 //#include "N5110.h"

 // Using NOKIA 5110 monochrome 84 x 48 pixel display
 // pin 9 - Serial clock out (SCLK)
 // pin 8 - Serial data out (DIN)
 // pin 7 - Data/Command select (D/C)
 // pin 5 - LCD chip select (CS)
 // pin 6 - LCD reset (RST)
 //Adafruit_PCD8544 display = Adafruit_PCD8544(9, 8, 7, 5, 6);

 float sum = 0;
 uint32_t sumCount = 0;

    MPU6050 mpu6050;

    Timer t;

    Serial pc(USBTX, USBRX); // tx, rx

    //        VCC,   SCE,  RST,  D/C,  MOSI,S CLK, LED
    //N5110 lcd(PA_8, PB_10, PA_9, PA_6, PA_7, PA_5, PC_7);
    // Motors
    // Motor m1(PIN_M1_EN, PIN_M1_IN1, PIN_M1_IN2); //trás esquerda
    // Motor m2(PIN_M2_EN, PIN_M2_IN1, PIN_M2_IN2); //frente esquerda
    // Motor m3(PIN_M3_EN, PIN_M3_IN1, PIN_M3_IN2); //trás direita
    // Motor m4(PIN_M4_EN, PIN_M4_IN1, PIN_M4_IN2); //frente direita
    //
    // //QEI(PinName channelA, PinName channelB, PinName index, int pulsesPerRev, Encoding encoding = X2_ENCODING);
    // QEI encA (PIN_ENCA_1, PIN_ENCA_2, NC, COUNTS_PER_REV);
    // QEI encB (PIN_ENCB_1, PIN_ENCB_2, NC, COUNTS_PER_REV);
    // QEI encC (PIN_ENCC_1, PIN_ENCC_2, NC, COUNTS_PER_REV);
    // QEI encD (PIN_ENCD_1, PIN_ENCD_2, NC, COUNTS_PER_REV);

    // ultrasonic(PinName trigPin, PinName echoPin, float updateSpeed, float timeout);
    ultrasonic us1 (PIN_US_S1_TRIG, PIN_US_S1_ECHO, .1,3);
    ultrasonic us2 (PIN_US_S2_TRIG, PIN_US_S2_ECHO, .1,3);


    int pulses_m1 = 0;
    int pulses_m2 = 0;
    int pulses_m3 = 0;
    int pulses_m4 = 0;
    int degrees_m1 = 0;
    int distance = 5000;

    int us1_value = 0;
    int us2_value = 0;

    bool waitDegrees(QEI &enc, Motor &motor,float speed = 0.5, int setPoint = 360, bool isBrake = true){
      int _pulses = 0;
      int _degrees = 0;
      int _tolerance = 0.8;
      float _expectedRPS = (MOTOR_RPM/60.0)*abs(speed);
      float setPointInRotarion = setPoint/360.0;
      float timeOut = (setPointInRotarion/_expectedRPS) * (1.0 + _tolerance);
      Timer _localTime;

      _localTime.start();

      enc.reset();
      motor.speed(speed);

      while(_degrees<setPoint){
        _pulses  = enc.getPulses();
        _degrees = (_pulses /(.5*COUNTS_PER_REV))*360;
        _degrees = abs(_degrees);

        pc.printf("\nDegrees: %d", _degrees);
        pc.printf("\nTimeOUT: %.3f",timeOut );
        //pc.printf("\n Pulses : %f", ((setPoint*_tolerance)));

         if(_localTime >timeOut){
           break;
         }
        }

      isBrake? motor.brake():motor.coast();

      _localTime.stop();
      _localTime.reset();

      return(true);

    }


void forward(float _speed,int setPoint, bool isBrake=true){

  // Motor m1 --> trás esquerda
  // Motor m2 --> frente esquerda
  // Motor m3 --> trás direita
  // Motor m4 --> frente direita
  // waitDegrees(QEI &enc, Motor &motor,float speed = 0.5, int setPoint = 360, bool isBrake = true)
    int _pulses_m1 = 0;
    int _pulses_m2 = 0;
    int _pulses_m3 = 0;
    int _pulses_m4 = 0;

    int _degrees_m1 = 0;
    int _degrees_m2 = 0;
    int _degrees_m3 = 0;
    int _degrees_m4 = 0;

    int _tolerance = 0.8;
    float _expectedRPS = (MOTOR_RPM/60.0)*abs(_speed);
    float setPointInRotarion = setPoint/360.0;
    float timeOut = (setPointInRotarion/_expectedRPS) * (1.0 + _tolerance);
    Timer _localTime;

    _localTime.start();

    encA.reset();
    encB.reset();
    encC.reset();
    encD.reset();

    m1.speed(_speed);
    m2.speed(_speed);
    m3.speed(_speed);
    m4.speed(_speed);

    bool m1_finish = false;
    bool m2_finish = false;
    bool m3_finish = false;
    bool m4_finish = false;


    while(!(m1_finish||m3_finish) && !(m2_finish||m4_finish)){ //_degrees<setPoint
      // _pulses  = enc.getPulses();
      _pulses_m1 = encA.getPulses();
      _pulses_m2 = encB.getPulses();
      _pulses_m3 = encC.getPulses();
      _pulses_m4 = encD.getPulses();

      // _degrees = (_pulses /(.5*COUNTS_PER_REV))*360;
      _degrees_m1 = (_pulses_m1/(.5*COUNTS_PER_REV))*360;
      _degrees_m2 = (_pulses_m2/(.5*COUNTS_PER_REV))*360;
      _degrees_m3 = (_pulses_m3/(.5*COUNTS_PER_REV))*360;
      _degrees_m4 = (_pulses_m4/(.5*COUNTS_PER_REV))*360;

      // _degrees = abs(_degrees);
      _degrees_m1 = abs(_degrees_m1);
      _degrees_m2 = abs(_degrees_m2);
      _degrees_m3 = abs(_degrees_m3);
      _degrees_m4 = abs(_degrees_m4);

      // (_degrees<setPoint)? : ;
      (_degrees_m1>=setPoint)?m1_finish = !m1_finish : m1_finish = m1_finish;
      (_degrees_m2>=setPoint)?m2_finish = !m2_finish : m2_finish = m2_finish;
      (_degrees_m3>=setPoint)?m3_finish = !m3_finish : m3_finish = m3_finish;
      (_degrees_m4>=setPoint)?m4_finish = !m4_finish : m4_finish = m4_finish;

      // pc.printf("\nDegrees: %d", _degrees);
      // pc.printf("\nTimeOUT: %.3f",timeOut );
      //pc.printf("\n Pulses : %f", ((setPoint*_tolerance)));

       if(_localTime >timeOut){
         break;
       }
      }

    if(isBrake){
      m1.brake();
      m2.brake();
      m3.brake();
      m4.brake();
    }else{
      m1.coast();
      m2.coast();
      m3.coast();
      m4.coast();
    }

    _localTime.stop();
    _localTime.reset();

}

void turnLeft(float _speed, float setPoint, bool isBrake=true){
  // Motor m1 --> trás esquerda
  // Motor m2 --> frente esquerda
  // Motor m3 --> trás direita
  // Motor m4 --> frente direita
  // waitDegrees(QEI &enc, Motor &motor,float speed = 0.5, int setPoint = 360, bool isBrake = true)
    int _pulses_m1 = 0;
    int _pulses_m2 = 0;
    int _pulses_m3 = 0;
    int _pulses_m4 = 0;
    int _degrees_m1 = 0;
    int _degrees_m2 = 0;
    int _degrees_m3 = 0;
    int _degrees_m4 = 0;

    int _tolerance = 0.8;
    float _expectedRPS = (MOTOR_RPM/60.0)*abs(_speed);
    float setPointInRotarion = setPoint/360.0;
    float timeOut = (setPointInRotarion/_expectedRPS) * (1.0 + _tolerance);
    Timer _localTime;

    _localTime.start();

    encA.reset();
    encB.reset();
    encC.reset();
    encD.reset();

    m1.speed(-_speed);
    m2.speed(-_speed);
    m3.speed(_speed);
    m4.speed(_speed);

    bool m1_finish = false;
    bool m2_finish = false;
    bool m3_finish = false;
    bool m4_finish = false;


    while(!(m1_finish||m3_finish) && !(m2_finish||m4_finish)){ //_degrees<setPoint
      // _pulses  = enc.getPulses();
      _pulses_m1 = encA.getPulses();
      _pulses_m2 = encB.getPulses();
      _pulses_m3 = encC.getPulses();
      _pulses_m4 = encD.getPulses();

      // _degrees = (_pulses /(.5*COUNTS_PER_REV))*360;
      _degrees_m1 = (_pulses_m1/(.5*COUNTS_PER_REV))*360;
      _degrees_m2 = (_pulses_m2/(.5*COUNTS_PER_REV))*360;
      _degrees_m3 = (_pulses_m3/(.5*COUNTS_PER_REV))*360;
      _degrees_m4 = (_pulses_m4/(.5*COUNTS_PER_REV))*360;

      // _degrees = abs(_degrees);
      _degrees_m1 = abs(_degrees_m1);
      _degrees_m2 = abs(_degrees_m2);
      _degrees_m3 = abs(_degrees_m3);
      _degrees_m4 = abs(_degrees_m4);

      pc.printf("\nDegrees: %d", _degrees_m1);

      // (_degrees<setPoint)? : ;
      (_degrees_m1>=setPoint)?m1_finish = !m1_finish : m1_finish = m1_finish;
      (_degrees_m2>=setPoint)?m2_finish = !m2_finish : m2_finish = m2_finish;
      (_degrees_m3>=setPoint)?m3_finish = !m3_finish : m3_finish = m3_finish;
      (_degrees_m4>=setPoint)?m4_finish = !m4_finish : m4_finish = m4_finish;

      // pc.printf("\nDegrees: %d", _degrees);
      // pc.printf("\nTimeOUT: %.3f",timeOut );
      //pc.printf("\n Pulses : %f", ((setPoint*_tolerance)));

       if(_localTime >timeOut){
         break;
       }
      }

    if(isBrake){
      m1.brake();
      m2.brake();
      m3.brake();
      m4.brake();
    }else{
      m1.coast();
      m2.coast();
      m3.coast();
      m4.coast();
    }

    _localTime.stop();
    _localTime.reset();

}

void turnRight(float _speed, float setPoint, bool isBrake=true){
    turnLeft(-_speed, setPoint, isBrake);
}
typedef enum sentido {
    horario,
    antihorario
} sentido;

void turn(sentido _sentido, float _speed, float setPoint, bool isBrake=true){
(_sentido == horario)? _speed = -_speed:_speed = _speed;
turnLeft(_speed, setPoint, isBrake);
}

void backward(float _speed, int setPoint, bool isBrake=true){
  forward(- _speed, setPoint, isBrake);
}
 int main()
 {
   pc.baud(115200);
   pc.printf("\nBAR Connected");

   //Set up I2C
   i2c.frequency(400000);  // use fast (400 kHz) I2C

   t.start();



  wait(10);
  int distanceSetPoint_back = (LARGURA_CAMPO*0.5)/WHEEL_RADIO;   //cm
  int distanceSetPoint_forward = (ALTURA_CAMPO*0.5)/WHEEL_RADIO; //cm

  int degreesSetPoint_forward = (distanceSetPoint_forward*180)/MATH_PI;
  int degreesSetPoint_back = (distanceSetPoint_back*180)/MATH_PI;
  int turnNinetyDegrees = 1.43*360;

  // pc.printf("\nDist back: %d", degreesSetPoint_back);
  // pc.printf("\nDist forward: %d", degreesSetPoint_forward);

  #ifdef TERRINES_LEFT
  backward(0.5, degreesSetPoint_back, false);
  forward(0.3, 150);
  turnLeft(0.7, turnNinetyDegrees, true);
  forward(0.3, degreesSetPoint_forward, false);
  forward(0.5, 200,false);
  //go back with terrines
  backward(0.1, 180, false);
  //go to cows
  turnRight(0.7, turnNinetyDegrees);
  backward(0.5, 500);
  forward(0.3,degreesSetPoint_forward);

  #endif

  #ifdef TERRINES_RIGHT
  backward(0.5, degreesSetPoint_back, false);
  forward(0.3, 100);
  turnRight(0.7, turnNinetyDegrees, true);
  forward(0.3, degreesSetPoint_forward, false);
  forward(0.5, 200,false);
  //go back with terrines
  backward(0.3, degreesSetPoint_forward, true);
  turnRight(0.2, 180);
  //go to cows
  turnLeft(0.7, turnNinetyDegrees);
  backward(0.5, 500);
  forward(0.3,degreesSetPoint_back);
  #endif

  // turnRight(0.7, turnNinetyDegrees, true);

   // m1.speed(-1);
   //m2.speed(0.5);
   //m4.speed(0.3);
   //m3.speed(0.5);
   // pc.printf("\n --- Motor M1" );
   // bool m1_finish = waitDegrees(encA,m1);
   // pc.printf("\n --- Motor M2" );
   // bool m2_finish = waitDegrees(encB,m2);
   // pc.printf("\n --- Motor M3" );
   // bool m3_finish = waitDegrees(encC,m3);
   // pc.printf("\n --- Motor M4");
  //bool m4_finish = waitDegrees(encD,m4);

  //   m1.brake();
   //pc.printf("\n Final M1 degrees: %d", degrees_m1);



//
//us1.startUpdates();
//us2.startUpdates();
//
//  while (1) {
//    us1.checkDistance();
//    us2.checkDistance();
//    /* code */
//
//    us1_value = us1.getCurrentDistance();
//    us2_value = us2.getCurrentDistance();
//
//    wait(2);
//
//    pc.printf("\n us1_value: %f",us1_value );
//    pc.printf("\n us2_value: %f",us2_value );
//  }
}