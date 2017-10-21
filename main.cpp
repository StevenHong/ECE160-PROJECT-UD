/*====================INC FILE==================*/
#include <Arduino.h>
#include <Servo.h>


/*=================INC FILE END=================*/

/*================GLOBAL DEFINE=================*/
const float SERVO_CALI_L = 93.0; //SERVO MID POINT CALI NUMBER
const float SERVO_CALI_R = 93.0; //SERVO MID POINT CALI NUMBER

const float SERVO_MAGIC_L = 0.995;
const float SERVO_MAGIC_R = 0.98;

const uint8_t WHEEL_THRESHOLD   = 10; //SERVO FLEX REGION 

//#define USE_CURVED_THROTT
//#define USE_CURVED_TURN

#define USE_BLUETOOTH_JOYSTICK
//#define USE_LORA_JOYSTICK
//#define USE_IR_REMOTE
//#define USE_NRF24_JOYSTICK

//#define ENABLE_AUTO_MODE

#define ENABLE_FRC_JOYSTICK_MODE

#define EN_DEBUG

#ifdef USE_IR_REMOTE
    #include <IRLremote.h>
#endif

#ifdef USE_BLUETOOTH_JOYSTICK
    #include <PS2X_lib.h> 
#endif

#ifdef EN_DEBUG
    #define DEBUG_PRINT(str) \
    Serial.print(millis()); \
    Serial.print(": "); \
    Serial.print(__FUNCTION__); \
    Serial.print("() in "); \
    Serial.print(__FILE__); \
    Serial.print(':'); \
    Serial.print(__LINE__); \
    Serial.print(' '); \
    Serial.println(str);
#else
    #define DEBUG_PRINT(str)
#endif

#ifdef USE_CURVED_THROTT
    #define THROTTCALU(x) (3.0*pow(10.0, -5)*((x) * (x) * (x)) + double(0.1916)*(x))
#endif

#ifdef USE_CURVED_TURN
    #define CURVECALU(x) (3.0*pow(10.0, -5)*((x) * (x) * (x)) + double(0.1916)*(x))
#endif

#ifndef ENABLE_AUTO_MODE
    #warning "############AUTOMATION MODE DISABLED##################"
#endif
/*==============GLOBAL DEFINE END===============*/

/*=================PIN DEFINE===================*/
const uint8_t SERVO_LEFT_PIN    = 13;
const uint8_t SERVO_RIGHT_PIN   = 12;

const uint8_t SERVO_GRAP_PIN    = 11;

#ifdef USE_IR_REMOTE
    const uint8_t IR_PIN        = 2;
    const uint32_t UP_ID        = 70;
    const uint32_t LEFT_ID      = 68;
    const uint32_t DOWN_ID      = 21;
    const uint32_t RIGHT_ID     = 67;
    const uint32_t GRAB_ID      = 69;
    const uint32_t DEGRAB_ID    = 71;
#endif

#ifdef USE_BLUETOOTH_JOYSTICK
    const uint8_t PS2X_CLK      = 5;
    const uint8_t PS2X_CMD      = 7;
    const uint8_t PS2X_ATT      = 4;
    const uint8_t PS2X_DAT      = 6;
#endif
/*===============PIN DEFINE END==================*/


/*==============INSTANLIZATION==================*/
Servo servoL;
Servo servoR;
Servo servoG;
#ifdef USE_BLUETOOTH_JOYSTICK
    PS2X ps2x;
    int8_t CALI_ZERO_LX = 0;
    int8_t CALI_ZERO_LY = 0;
    int8_t CALI_ZERO_RX = 0;
    int8_t CALI_ZERO_RY = 0;
#endif

#ifdef USE_IR_REMOTE
    CNec IRLremote;
#endif

/*============INSTANLIZATION END===================*/

/*==============INIT FUNCTION======================*/
void vInitServo(){
    servoL.attach(SERVO_LEFT_PIN);
    servoR.attach(SERVO_RIGHT_PIN);
    servoG.attach(SERVO_GRAP_PIN);
}
/*==============INIT FUNCTION END==================*/

/*==============SERVO CLASS========================*/
void vServoForward(float speed){
    servoL.write(SERVO_CALI_L + speed * WHEEL_THRESHOLD);
    servoR.write(SERVO_CALI_R - speed * WHEEL_THRESHOLD );
}

void vServoBackward(float speed){
    servoL.write(SERVO_CALI_L - speed * WHEEL_THRESHOLD);
    servoR.write(SERVO_CALI_R + speed * WHEEL_THRESHOLD );
}

void vServoSpin(float speed){
    servoL.write(SERVO_CALI_L + speed * WHEEL_THRESHOLD);
    servoR.write(SERVO_CALI_R + speed * WHEEL_THRESHOLD);
}

void vServoTurn(float speed, float bias){
    if(speed + bias > 1){
        speed *= 1.0/(speed+bias);
        speed *= 1.0/(speed+bias);
    }
    servoL.write(SERVO_CALI_L + speed * WHEEL_THRESHOLD * SERVO_MAGIC_L +  WHEEL_THRESHOLD * bias);
    servoR.write(SERVO_CALI_R - speed * WHEEL_THRESHOLD * SERVO_MAGIC_R +  WHEEL_THRESHOLD * bias * SERVO_MAGIC_R);
}

void vServoDual(float fServoLeft, float fServoRight){
    servoL.write(SERVO_CALI_L + fServoLeft * WHEEL_THRESHOLD * SERVO_MAGIC_L);
    servoR.write(SERVO_CALI_R - fServoRight * WHEEL_THRESHOLD * SERVO_MAGIC_R);
}

void vServoGrab(){
    servoG.write(180);
}

void vServoDegrab(){
    servoG.write(0);
}
/*=================SERVO CALSS END=================*/

#ifdef USE_BLUETOOTH_JOYSTICK
    void vBLEJoystickCalib(){
        ps2x.read_gamepad(); 
        CALI_ZERO_LX = 128 - ps2x.Analog(PSS_LX);
        CALI_ZERO_LY = 128 - ps2x.Analog(PSS_LY);
        CALI_ZERO_RX = 128 - ps2x.Analog(PSS_RX);
        CALI_ZERO_RY = 128 - ps2x.Analog(PSS_RY);
        DEBUG_PRINT(String("OFFSET: LX")+String(CALI_ZERO_LX) +
                        String(" LY")+String(CALI_ZERO_LY) +
                            String(" RX")+String(CALI_ZERO_RX) +
                                String(" RY")+String(CALI_ZERO_RY));
    }
#endif
void setup()
{
    Serial.begin(115200);
    DEBUG_PRINT("PROGRAM START, BANDRATE INIT TO 115200");
    
    vInitServo();

    #ifdef USE_BLUETOOTH_JOYSTICK
        uint8_t error = ps2x.config_gamepad(PS2X_CLK, PS2X_CMD, PS2X_ATT, PS2X_DAT, false, false);
        if(0 == error){
            DEBUG_PRINT("PS2X JOYSTICK INIT SUCCESS");
        }else{
            DEBUG_PRINT(String("PS2X JOYSTICK INIT FAIL, ERROR CODE: ") + error);
            while(1);
        }
        vBLEJoystickCalib();
    #endif

    #ifdef USE_IR_REMOTE
        if(!IRLremote.begin(IR_PIN)){
            DEBUG_PRINT("INVAILD PIN SELETED FOR IR_PIN");
        }
    #endif
    
}

uint64_t u64SysTick = 0;
bool bIsGrab = false;
float fForwardSpeed = 0.0;
float fTurnSpeed = 0.0;
uint32_t u32IrCmd = 0;
uint32_t cnt = 0;
#ifdef ENABLE_FRC_JOYSTICK_MODE
    float fLeftSpeed = 0.0;
    float fRightSpeed = 0.0;
#endif

void loop()
{
    #ifdef USE_BLUETOOTH_JOYSTICK
        ps2x.read_gamepad(); 

        #ifndef ENABLE_FRC_JOYSTICK_MODE
            #ifdef USE_CURVED_THROTT
                fForwardSpeed = THROTTCALU(double(128 - (CALI_ZERO_LY + ps2x.Analog(PSS_LY)))) / 512.0;
            #else
                fForwardSpeed = float(128 - (CALI_ZERO_LY + ps2x.Analog(PSS_LY))) / 128.0;
            #endif

            #ifdef USE_CURVED_TURN
                fTurnSpeed = CURVECALU(double(CALI_ZERO_RX + ps2x.Analog(PSS_RX) - 128)) / 384.0;
            #else
                fTurnSpeed = float((CALI_ZERO_RX + ps2x.Analog(PSS_RX) - 128)) / 256.0;
            #endif
        #else
            #ifdef USE_CURVED_THROTT
                fLeftSpeed = THROTTCALU(double(128 - (CALI_ZERO_LY + ps2x.Analog(PSS_LY)))) / 512.0;
            #else
                fLeftSpeed = float(128 - (CALI_ZERO_LY + ps2x.Analog(PSS_LY))) / 128.0;
            #endif

            #ifdef USE_CURVED_TURN
                fRightSpeed = CURVECALU(double(CALI_ZERO_RX + ps2x.Analog(PSS_RY) - 128)) / 384.0;
            #else
                fRightSpeed = float((CALI_ZERO_RX + ps2x.Analog(PSS_RY) - 128)) / 128.0;
            #endif
        #endif

        if(true == ps2x.Button(PSB_L1)){
            bIsGrab = false;
        }
        if(true == ps2x.Button(PSB_R1)){
            bIsGrab = true;
        }

    #endif
    
    #ifdef USE_IR_REMOTE
        if(IRLremote.available())
        {
            auto data = IRLremote.read();
            u32IrCmd = data.command;
            process_ir_non_zero:
            if(0 != u32IrCmd){
                DEBUG_PRINT(String("IR REMOTE COMMAND: ") + String(u32IrCmd));
                switch(u32IrCmd){
                    case UP_ID:
                        fForwardSpeed = 1.0;
                        fTurnSpeed = 0.0;
                        break;
                    case LEFT_ID:
                        fTurnSpeed = -0.3;
                        fForwardSpeed = 0.0;
                        break;
                    case DOWN_ID:
                        fForwardSpeed = -1.0;
                        fTurnSpeed = 0.0;
                        break;
                    case RIGHT_ID:
                        fTurnSpeed = 0.3;
                        fForwardSpeed = 0.0;
                        break;
                    case GRAB_ID:
                         bIsGrab = true;
                         break;
                    case DEGRAB_ID:
                         bIsGrab = false;
                         break;
                    default:
                        fTurnSpeed = 0.0;
                        fForwardSpeed = 0.0;
                        break;
                }
            }else{
                DEBUG_PRINT("IR REMOTE CONTINUE");
            }
        }

    #endif
    if(0.05 >= abs(fForwardSpeed) && (0.05 >=abs(fTurnSpeed))){
        vServoDual(fLeftSpeed, fRightSpeed);
        DEBUG_PRINT(String("Left Speed: ") + String(fLeftSpeed) + 
        String("   Right Speed: ") + String(fRightSpeed) + 
            String(" Grab Stuatus: ") + String(bIsGrab));
    }else{
        vServoTurn(fForwardSpeed, fTurnSpeed);
        DEBUG_PRINT(String("Forward Speed: ") + String(fForwardSpeed) + 
        String(" TurnSpeed: ") + String(fTurnSpeed) + 
            String(" Grab Stuatus: ") + String(bIsGrab));
    }
    
    if(true == bIsGrab){
        vServoGrab();
    }else{
        vServoDegrab();
    }

}