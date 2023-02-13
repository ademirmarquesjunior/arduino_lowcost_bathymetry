#include <SD.h>
//#include <Maxbotix.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>

#define LOG_INTERVAL 500 // Interval between readings
#define SYNC_INTERVAL 500 // Interval between SD card writings

//Ultrasonic sensor definitions using the Maxbotix library
//Maxbotix rangeSensorPW(6, Maxbotix::PW, Maxbotix::XL, Maxbotix::BEST);
//Maxbotix rangeSensorTX(0 && 1, Maxbotix::TX, Maxbotix::XL, Maxbotix::MEDIAN);
//Maxbotix rangeSensorAD(A0, Maxbotix::AN, Maxbotix::XL, Maxbotix::BEST, 9);

//Thermal sensor
//#define ONE_WIRE_BUS 5
//OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
//DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.

uint32_t syncTime = 0; // time of last sync()

//SD card definitions
const int chipSelect = 10;
File logfile;

TinyGPS gps;
SoftwareSerial ss(4, 3);

int led = 7;

//Sensor pins
const int pwPin1 = 6;
const int anPin1 = A0;

//Sensor variables
long sensor1, cm, sensor2, cm2;


void error(char *str) {
	Serial.print("error: ");
	Serial.println(str);
	while(1);
}

void read_sensor(){
  sensor1 = pulseIn(pwPin1, HIGH);
  sensor2 = analogRead(anPin1);
  cm = sensor1/58; 
  cm2 = sensor2;
}

void setup(void) {
	
	Serial.begin(9600);
	ss.begin(9600);

	// Sensor delay for analog readings
	// rangeSensorAD.setADSampleDelay(10);
	pinMode(led, OUTPUT);
	
    // novo sensor
    pinMode(pwPin1, INPUT);
        

    // Initialize thermal sensor
    // sensors.begin();

	Serial.println();
	
	
	// Initialize SD card
	Serial.print("Initializing SD card...");
	// make sure that the default chip select pin is set to
	// output, even if you don't use it:
	pinMode(10, OUTPUT); 
	
	// VERIFICA SE O CARTÃO ESTÁ PRESENTE E PODE SER INICIALIZADO:
	if (!SD.begin(chipSelect)) {
		error("SD card not initialized");
	}
	Serial.println("SD card not initialized");
	// Create a file LOGGER00.txt. 
	char filename[] = "LOGGER00.TXT";
	for (uint8_t i = 0; i < 1000; i++) {
		filename[6] = i/10 + '0';
		filename[7] = i%10 + '0';
		if (!(SD.exists(filename))) {
			// only open a new file if it doesn't exist
			logfile = SD.open(filename, FILE_WRITE); 
			break; // leave the loop!
		}
	}
	if (! logfile) {
		error("File not created");
	}
	Serial.print("Recording ");
	Serial.println(filename);
	
	// Collumns for the log file
	logfile.println("date_time, satelites, precision, latitude, longitude, pulse_signal, analog_signal");  
	Serial.println("date_time, satelites, precision, latitude, longitude, pulse_signal, analog_signal");
	
}


void loop(void) {
	
	
	// delay for the amount of time we want between readings
	delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
	// log milliseconds since starting
	uint32_t m = millis();
	
        read_sensor();
        
        if (!(cm < 20)) { // Record only readings greater than 20
			bool newData = false;
			unsigned long chars;
			unsigned short sentences, failed;
			float flat, flon;
			unsigned long age;
			int year;
			byte month, day, hour, minute, second, hundredths; 
			
			digitalWrite(led, HIGH);
        
			// For one second we parse GPS data and report some key values
			for (unsigned long start = millis(); millis() - start < 1000;){
				while (ss.available()){
					char c = ss.read();
					//Serial.write(c); // uncomment this line if you want to see the GPS data flowing
					if (gps.encode(c)) // Did a new valid sentence come in?
					newData = true;
				}
			}
	
			// Get GPS data
			if (newData){
				gps.f_get_position(&flat, &flon, &age);
				gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
			}	
			gps.stats(&chars, &sentences, &failed);
			
			/*
				Serial.print(" CHARS=");
				Serial.print(chars);
				Serial.print(" SENTENCES=");
				Serial.print(sentences);
				Serial.print(" CSUM ERR=");
				Serial.println(failed);
			*/
			
			if (sentences == 0){ // The led used as GPS indicator is off when no GPS data is read;
				digitalWrite(led, LOW);
			}
			//logfile.print('"');
			
			//novo sensor

        
			logfile.print(day);
			logfile.print('/');
			logfile.print(month);
			logfile.print('/');
			logfile.print(year);
			logfile.print(' ');       
			logfile.print(hour);
			logfile.print(':');
			logfile.print(minute);
			logfile.print(':');
			logfile.print(second);
			logfile.print(", ");
			logfile.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
			logfile.print(", ");
			logfile.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
			logfile.print(", ");
			logfile.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
			logfile.print(", ");
			logfile.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);		
			//logfile.print(", ");
			//logfile.print(sensors.getTempCByIndex(0)); 
			logfile.print(", ");
			logfile.print(cm);
			//logfile.print(", ");
			//logfile.print(rangeSensorTX.getRange());
			logfile.print(", ");
			logfile.println(cm2);

			/*
			//Serial.print('"');
			Serial.print(day);
			Serial.print('/');
			Serial.print(month);
			Serial.print('/');
			Serial.print(year);
			Serial.print(' ');       
			Serial.print(hour);
			Serial.print(':');
			Serial.print(minute);
			Serial.print(':');
			Serial.print(second);
			Serial.print(", ");
			Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
			Serial.print(", ");
			Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
			Serial.print(", ");
			Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
			Serial.print(", ");	
			Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
			//Serial.print(", ");
			//Serial.print(sensors.getTempCByIndex(0));	
			Serial.print(", ");
			Serial.print(cm);
			//Serial.print(", ");
			//Serial.print(rangeSensorTX.getRange());
			Serial.print(", ");
			Serial.println(cm);
			*/
			
			// Check if the time elapsed is greater than the sync time to properly flush data to SD card
			if ((millis() - syncTime) < SYNC_INTERVAL) return;
			
			syncTime = millis();
			logfile.flush();
			}
}
