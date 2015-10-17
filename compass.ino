#include <Wire.h> //I2C Arduino Library
 
#define address 0x1E //001 1110b(0x3C>>1), I2C 7bit address of HMC5883
#define MagnetcDeclination 4.17 //Hsinchu
#define CalThreshold 0
 
int offsetX,offsetY;

int led[12] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

void setup() {
  // put your setup code here, to run once:
  for(int i = 0; i < 12;i++){
    pinMode(led[i], OUTPUT);
    digitalWrite(led[i], HIGH);
  }
  
  Wire.begin();
 
  //Put the HMC5883 IC into the correct operating mode
  Wire.beginTransmission(address); //open communication with HMC5883
  Wire.write(0x00); //select configuration register A
  Wire.write(0x70); //0111 0000b configuration
  Wire.endTransmission();
 
  Wire.beginTransmission(address);
  Wire.write(0x02); //select mode register
  Wire.write(0x00); //set continuous measurement mode:0x00,single-measurement mode:0x01
  Wire.endTransmission();
 
  calibrateMag();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  int heading;
  int x,y,z; //triple axis data
  
  getRawData(&x,&y,&z);
  heading = calculateHeading(&x,&y,&z)/30;
  digitalWrite(led[heading], LOW);
  delay(50);
  digitalWrite(led[heading], HIGH);
  delay(1);
}

void getRawData(int* x ,int* y,int* z)
{
  //Tell the HMC5883L where to begin reading data
  Wire.beginTransmission(address);
  Wire.write(0x03); //select register 3, X MSB register
  Wire.endTransmission();
  //Read data from each axis, 2 registers per axis
  Wire.requestFrom(address, 6);
  if(6<=Wire.available()){
    *x = Wire.read()<<8; //X msb
    *x |= Wire.read(); //X lsb
    *z = Wire.read()<<8; //Z msb
    *z |= Wire.read(); //Z lsb
    *y = Wire.read()<<8; //Y msb
    *y |= Wire.read(); //Y lsb
  }
}
 
int calculateHeading(int* x ,int* y,int* z)
{
  float headingRadians = atan2((double)(-((*y)-offsetY)),(double)((*x)-offsetX));
  // Correct for when signs are reversed.
  if(headingRadians < 0)
    headingRadians += 2*PI;
 
  int headingDegrees = headingRadians * 180/M_PI;
  headingDegrees += MagnetcDeclination; //the magnetc-declination angle 
 
  // Check for wrap due to addition of declination.
  if(headingDegrees > 360)
    headingDegrees -= 360;
 
  return headingDegrees;
}

void calibrateMag()
{
  int x,y,z; //triple axis data
  int xMax, xMin, yMax, yMin;

  int num = 0;

  //initialize the variables
  getRawData(&x,&y,&z);  
  xMax=xMin=x;
  yMax=yMin=y;
  offsetX = offsetY = 0;
 
  for(int i=0;i<50;i++)
  {
    getRawData(&x,&y,&z);
    
    if (x > xMax)
      xMax = x;
    if (x < xMin )
      xMin = x;
    if(y > yMax )
      yMax = y;
    if(y < yMin )
      yMin = y;
    
    for(int j = 0;j < 12;j++)
      digitalWrite(led[j], HIGH);

    digitalWrite(led[num], LOW);
    num = (num + 1) % 12;
    
    delay(100);
  }
  //compute offsets
  if(abs(xMax - xMin) > CalThreshold )
    offsetX = (xMax + xMin)/2;
  if(abs(yMax - yMin) > CalThreshold )
    offsetY = (yMax + yMin)/2;
  
   for(int i = 0;i < 12;i++)
         digitalWrite(led[i], HIGH);
  delay(1000);  
}