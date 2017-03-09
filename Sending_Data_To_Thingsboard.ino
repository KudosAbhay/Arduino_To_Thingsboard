/*  
 *  Sending Data to Thingsboard
 *  Working Code!
 *  
 *  This is the CURL Command to send data:
 *  curl -v -X POST -d @telemetry-data-as-object.json http://demo.thingsboard.io:80/api/v1/<ACCESS_TOKEN>/telemetry --header "Content-Type:application/json"
 *  
 *  Connected to demo.thingsboard.io (104.196.24.70) port 80 (#0)
 *  > POST /api/v1/<ACCESS_TOKEN>/telemetry HTTP/1.1
 *  > Host: demo.thingsboard.io
 *   > Accept: **
 *   > Content-Type:application/json
 *   > Content-Length: 34
 *   {"temperature":30.1, "voltage":24}               //This was the content in my JSON file
 *  
 *  
 *  This is the CURL Command to publish attributes
 *  curl -v -X POST -d @new-attributes-values.json http://ec2-35-154-9-130.ap-south-1.compute.amazonaws.com:80/api/v1/A1_TEST_TOKEN/attributes --header "Content-Type:application/json"
 */
 

#include <ArduinoJson.h>                                      //For Creating a Json File
DynamicJsonBuffer jsonBuffer;                                 //Set Buffer size to Dynamic
JsonObject& root = jsonBuffer.createObject();                 //Create an object 'root' which is called later to print JSON Buffer        


char aux_str[100];
char pin[]="";
char apn[]="airtelgprs.com";      //For airtel gprs connection
char user_name[]="";
char password[]="";
int lengthOfJSON;

char thingsboard_url[]="demo.thingsboard.io";   

boolean sim900Status = false;
char port[]="80";                      // PORT Connected on 
String getStr="";

String AccessToken ="A1_TEST_TOKEN";    //For Device 1

void setup() 
{
  Serial.begin(9600);    //To print serial megssages on Serial Monitor
  Serial1.begin(9600);   //To connect SIM900A and send AT Commands
  power_on();            // POWER ON GSM Module for communication 
  pinMode(13,OUTPUT);
}



void loop() 
{
  makeJson(12.30, 32.3, 43.3, 33.3, 54.5, 65.3);                    //Making JSON text here
  /*You can send any values from here and make a json file to send to thingsboard*/
  updateThingsboard();
  Serial.println("\n----Sleeping----\n");
  delay(10000); 
}


void updateThingsboard()
{
   lengthOfJSON = 0;                  //Set size of JSON text as '0' initially
   if(sim900Status==true)
   {  
      // Selects Single-connection mode
      if (sendATcommand2("AT+CIPMUX=0", "OK","ERROR", 1000) == 1)      // CIMPUX=0 is already set in Single-connection mode
       {
        // Waits for status IP INITIAL
          while(sendATcommand("AT+CIPSTATUS","INITIAL", 1000)  == 0 );  // Check Current Connection Status
          delay(2000);                                                   //wait 5 sec
                
          snprintf(aux_str, sizeof(aux_str), "AT+CSTT=\"%s\",\"%s\",\"%s\"", apn, user_name, password);  //Put GPRS setings
        
        // Sets the APN, user name and password
          if (sendATcommand2(aux_str, "OK",  "ERROR", 30000) == 1)
          {            
            // Waits for status IP START
            while(sendATcommand("AT+CIPSTATUS", "START", 500)  == 0 );
            delay(2000);
          
            // Brings Up Wireless Connection
            if (sendATcommand2("AT+CIICR", "OK", "ERROR", 30000) == 1)
            {    
              delay(2000);
              Serial.println("\n Bringup Wireless Connection ...........");
             // Waits for status IP GPRSACT
             while(sendATcommand("AT+CIPSTATUS", "GPRSACT", 1000)  == 0 );
             delay(2000);
            
              // Gets Local IP Address
              if (sendATcommand2("AT+CIFSR", ".", "ERROR", 10000) == 1)
              {
                 // Waits for status IP STATUS
                 while(sendATcommand("AT+CIPSTATUS", "IP STATUS", 500)  == 0 );
                 delay(2000);
                 Serial.println("Opening TCP");
                 snprintf(aux_str, sizeof(aux_str), "AT+CIPSTART=\"TCP\",\"%s\",\"%s\"",thingsboard_url, port); //IP_address  
                 if (sendATcommand2(aux_str, "CONNECT OK", "CONNECT FAIL", 30000) == 1)
                 {      
                    Serial.println("Connected");
                    String json="";
                    root.printTo(json);                       //Store JSON in a String named 'json'
                    lengthOfJSON = json.length();             //This gives us total size of our json text
                    
                    //TCP packet to send POST Request on https (Thingsboard)
                      getStr="POST /api/v1/"+ AccessToken +"/telemetry HTTP/1.1\r\nHost: demo.thingsboard.io\r\nAccept: */*\r\nContent-Type: application/json\r\nContent-Length:"+lengthOfJSON+"\r\n\r\n"+json; 
                    //TCP packet to send POST Request on https (Thingsboard)
                 
                    String sendcmd = "AT+CIPSEND="+ String(getStr.length());
                          
                    if (sendATcommand2(sendcmd, ">", "ERROR", 10000) == 1)    
                    {
                      delay(100);
                      sendATcommand2(getStr, "SEND OK", "ERROR", 10000);      //Sending Data Here
                    }
                    Serial.println("Closing the Socket............");                            
                    sendATcommand2("AT+CIPCLOSE", "CLOSE OK", "ERROR", 10000);
                  }
                  else
                  {
                    Serial.println("Error opening the connection");
                  }  
              }
          }
        }
      }
   }
    Serial.println("Shutting down the connection.........");
    sendATcommand2("AT+CIPSHUT", "OK", "ERROR", 10000);
    delay(5000);
}//updateThingsboard




void makeJson( float val1, float val2, float val3, float val4, float val5, float val6 )
{
  Serial.println("\nMaking JSON text meanwhile\n");
  root["Time"] = val1;
  root["Milk Temperature"] = val2;
  root["AC Voltage"] = val3;
  root["Auxillary Temperature"] = val4;
  root["Battery Temperature"] = val5;
  root["Battery Voltage"] = val6;
}





void power_on()
{
   uint8_t answer=0;
   sim900Status=true;
   int ctT=0;
   digitalWrite(13,HIGH);
   delay(1000);
   digitalWrite(13,LOW);
   delay(2000);
   digitalWrite(13,HIGH);
   delay(5000);

    answer = sendATcommand("AT", "OK", 2000);
    if (answer == 0)
    {   Serial.println("SIM ON");
    
        while(answer == 0)
        { 
          // Send AT every two seconds and wait for the answer
            answer = sendATcommand("AT", "OK", 2000);    
            ctT++;
            if(ctT==10)
            {
              sim900Status=true;
              Serial.println("GSM OFF");
              power_on();
            }//nested if()  
        }
    }//if()
    
    Serial.println("GSM ON Sucessfully");
}//power_on()




int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout)
{
    uint8_t x=0,  answer=0;
    char response[3000];         //Response buffer
    unsigned long previous;

    memset(response, '\0', 3000);    // Initialize the string
    delay(100);
    while( Serial.available() > 0) Serial.read();    // Clean the input buffer

    Serial.println(ATcommand);    // Send the AT command 
    Serial1.println(ATcommand); //Response from Serial1 

    x = 0;
    previous = millis();

    // this loop waits for the answer
    do
    {
        if(Serial1.available() != 0)
        {    
            // if there is data in the UART input buffer, read it and checks for the asnwer
            response[x] = Serial1.read();     //Read Response from Serial1 port
            Serial.print(response[x]);        //Print response on Serial 0 port
            x++;
            
            // check if the desired answer  is in the response of the module
            if (strstr(response, expected_answer) != NULL)    
            {
                answer = 1;
            }
        }
    }//do
    // Waits for the asnwer with time out
    while((answer == 0) && ((millis() - previous) < timeout));    //Check till answer = 0 and timout period(ms)
    return answer;
}//sendATcommand()


int8_t sendATcommand2(String ATcommand, char* expected_answer1,char* expected_answer2, unsigned int timeout)
{
    uint8_t x=0,  answer=0;
    char response[3000];
    unsigned long previous;
    memset(response, '\0', 3000);    // Initialize the string
    delay(100);
    
    while( Serial1.available() > 0) Serial1.read();    // Clean the input buffer

    Serial1.println(ATcommand);    // Send the AT command 
    Serial.println(ATcommand);
    x = 0;
    previous = millis();

    // this loop waits for the answer
    do
    {
        // if there are data in the UART input buffer, reads it and checks for the asnwer
        if(Serial1.available() != 0)
        {    
            response[x] = Serial1.read();
            Serial.print(response[x]);
            x++;
            // check if the desired answer 1  is in the response of the module
            if (strstr(response, expected_answer1) != NULL)    
            {
                answer = 1;
            }
            // check if the desired answer 2 is in the response of the module
            else if (strstr(response, expected_answer2) != NULL)    
            {
                answer = 2;
            }
        }//if()
    
    }//do
    
    // Waits for the asnwer with time out
    while((answer == 0) && ((millis() - previous) < timeout));    
    return answer;
}//sendATcommand2()


int8_t sendATcommand3(String ATcommand, char* expected_answer1,char* expected_answer2,char*expected_answer3, unsigned int timeout)
{
    uint8_t x=0,  answer=0;
    char response[3000];
    unsigned long previous;

    memset(response, '\0', 3000);    // Initialize the string
    delay(100);

    while( Serial1.available() > 0) Serial1.read();    // Clean the input buffer

    Serial1.println(ATcommand);    // Send the AT command 
    Serial.println(ATcommand);
    x = 0;
    previous = millis();

    // this loop waits for the answer
    do
    {
        // if there are data in the UART input buffer, reads it and checks for the asnwer
        if(Serial1.available() != 0)
        {    
            response[x] = Serial1.read();
             Serial.print(response[x]);
            x++;
            // check if the desired answer 1  is in the response of the module
            if (strstr(response, expected_answer1) != NULL)    
            {
                answer = 1;
            }
            // check if the desired answer 2 is in the response of the module
            if (strstr(response, expected_answer2) != NULL)    
            {
                answer = 2;
            }
            else if (strstr(response, expected_answer3) != NULL)    
            {
                answer = 3;
            }
          
         
        }//if()
    }//do

    // Waits for the answer with time out
    while((answer == 0) && ((millis() - previous) < timeout));    
    return answer;
}//sendATcommand3
