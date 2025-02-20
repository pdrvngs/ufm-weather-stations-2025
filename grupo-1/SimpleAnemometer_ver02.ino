
int pulse=4;
int numRotations=0;
unsigned long prevMillis=0;
int radius=0.052; //radius of the anemometer
const int samplingtime = 6 * 1000;      // duration of sensor reading (ms)

 void setup() {

   
   pinMode(pulse, INPUT); 
   Serial.begin(115200);
   Serial.println("Serial start");     


    attachInterrupt(digitalPinToInterrupt(pulse), updateRotationCount, FALLING);
}

void updateRotationCount() {
  numRotations += 1;
}
void loop() {

unsigned long Millisnow=millis();

if(Millisnow>prevMillis+samplingtime){


sei();                                      // Enables interrupts 
    delay (3000);                               // Wait 3 seconds to average 
    cli();           
 
float rps = numRotations/samplingtime;       // rotations per second
     float rpm = 60*rps;                         // rotations per minute
      float omega = 2*PI*rps;                     // rad/seg
      float linear_velocity = omega*0.052;       // m/seg
      float linear_velocity_kmh = linear_velocity * 3.6; 
      
      Serial.print("  rot= ");
      Serial.print(numRotations);  
      Serial.print("  rpm= ");
      Serial.print(rpm);    
      Serial.print("  ,rps= ");
      Serial.print(rps);      
      Serial.print(" ,ω=  ");   
      Serial.print(omega);      
      Serial.print(" , vlin= ");    
      Serial.print(linear_velocity);      
      Serial.print(" ,vkmh=  ");            
      Serial.print(linear_velocity_kmh);
      Serial.println(" ");  
Millisnow=prevMillis;
numRotations = 0;
      delay(10);
}
else{Serial.println("  Sampling ");
      }}
 



 
