import processing.serial.*;
Serial myPort;
byte[] val = new byte[32];
int range,i=0;float pos;

void setup(){
 //Set UI at 30fps
 frameRate(30);  
 size(550,500);
 
 //Open serial port 
 String portName = "/dev/cu.usbserial-001K39BS";//x=your arduino port number
 myPort = new Serial(this, portName, 115200);
 
 //Set UI background
 background(255);
}
int countLoop = 0;
int freq=250;
void draw(){
 if(countLoop++==freq){
   myPort.write("distances get\n");
   println("send");
   countLoop=0;
   freq=2;
 }
 // if data available on serial port
 if ( myPort.available() > 0) {
  //Read until the end of the line
  val = myPort.readBytes(); 
  //print("Available:");
  //println(val);
  if(val!=null) {
      //println(val);
     //For ever measure
     int step = 0;
     for(int i=0;i<val.length;i++) {
       if(i==0 && val[i]!='D') break;
       if(i==0) continue;
       if(i==1 && val[i]!='D'){step = val[i]& 0xFF; continue;}
       if(val[i]=='\n' || (i+1<val.length && val[i+1]=='\n')) break;
       //split the angle and the distance
       int angle = (i-2)*step;
       int value = val[i] & 0xFF;
       if(angle>=360)break;
       //String[] value=split(measures[i]," ");
       //println(value.length);
       //if we correctly have an angle and a distance
       //if(value.length==2){
         float angleRad=float(angle)*PI/180.0;
         float distance=float(value);
         //Clear the previous line at this angle
         stroke(255, 255, 255);
         strokeWeight(3);
         line(250,250,250-(5000*cos(angleRad)),250-(5000*sin(angleRad))); 
         //Draw the new line with color depending on distance
         stroke(0, 255, 0);
         strokeWeight(1);
         float inter = map(distance, 0, 255, 0, 1);
         color c = lerpColor(color(255, 0, 0), color(0, 255, 0), inter);
         stroke(c);
         line(250,250,250-1*(distance*cos(angleRad)),250-1*(distance*sin(angleRad))); 
         println("angle:"+angle+" distance:"+value);
        }
     }
  }
}
