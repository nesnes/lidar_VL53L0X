import processing.serial.*;
Serial myPort;
String val;
int range,i=0;float pos;

void setup(){
 //Set UI at 30fps
 frameRate(30);  
 size(550,500);
 
 //Open serial port 
 String portName = "COM6";//x=your arduino port number
 myPort = new Serial(this, portName, 115200);
 
 //Set UI background
 background(255);
}

void draw(){
 // if data available on serial port
 if ( myPort.available() > 0) {
  //Read until the end of the line
  val = myPort.readStringUntil('\n');   
  if(val!=null) {
     println(val);
     //Split the angle/distance measures
     String[] measures=split(val,";");
     //For ever measure
     for(int i=0;i<measures.length;i++) {
       //split the angle and the distance
       String[] value=split(measures[i]," ");
       println(value.length);
       //if we correctly have an angle and a distance
       if(value.length==2){
         float angle=float(value[0])*PI/180.0;
         float distance=float(value[1])/10.0;
         //Clear the previous line at this angle
         stroke(255, 255, 255);
         strokeWeight(3);
         line(250,250,250-(5000*cos(angle)),250-(5000*sin(angle))); 
         //Draw the new line with color depending on distance
         stroke(0, 255, 0);
         strokeWeight(1);
         float inter = map(distance, 0, 100, 0, 1);
         color c = lerpColor(color(255, 0, 0), color(0, 255, 0), inter);
         stroke(c);
         line(250,250,250-4*(distance*cos(angle)),250-4*(distance*sin(angle))); 
         println("angle:"+angle+" distance:"+distance);
        }
     }
  }
}
}
