/* 
 *  Sending Data to Thingsboard
 *  Working Code!
 *  
 *  This is what is sent:
 *  curl -v -X POST -d @telemetry-data-as-object.json http://demo.thingsboard.io:80/api/v1/<Your Access Token>/telemetry --header "Content-Type:application/json"
 *  
 *  Connected to demo.thingsboard.io (104.196.24.70) port 80 (#0)
 *  > POST /api/v1/<Your AccessToken>/telemetry HTTP/1.1
 *  > Host: demo.thingsboard.io
 *  > Accept: **
 *  > Content-Type:application/json
 *  > Content-Length: 34
 *   {"temperature":30.1, "voltage":24}
 *  
 */


char aux_str[100];
char pin[]="";
char apn[]="airtelgprs.com";    
char user_name[]="";
char password[]="";
char thingsboard_url[]="demo.thingsboard.io";   //URL

boolean sim900Status = false;
char port[]="80";                      // PORT Connected on 
String getStr="";

String AccessToken ="Your Access Token";    //write API key


int field1 = 70; 
int field2 = -10;
int field3 = 60;


void setup() 
{
  Serial.begin(9600);     //Serial to print serial megssages
  Serial1.begin(9600);   // To connect SIM900A and send AT Commands
  power_on();            // POWER ON GSM Module for communication 
  pinMode(13,OUTPUT);
}

void loop() 
{
  updateThingsboard();
  delay(1000);    
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



void updateThingsboard()
{
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
                         Serial.println("\n Send some data to TCP Socket............");
                         getStr="POST /api/v1/"+ AccessToken +"/telemetry HTTP/1.1\r\nHost: demo.thingsboard.io\r\nAccept: */*\r\nContent-Type: application/json\r\nContent-Length:34\r\n\r\n{\"temperature\":30.1, \"voltage\":24}";   //TCP packet to send GET Request on https (Think Speak)
                          
                         String sendcmd = "AT+CIPSEND="+ String(getStr.length());
                          
                          if (sendATcommand2(sendcmd, ">", "ERROR", 10000) == 1)    
                            {
                              delay(100);
                              sendATcommand2(getStr, "SEND OK", "ERROR", 10000);      //Field1 Data sending from here
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
}
   
