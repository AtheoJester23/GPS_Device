#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <TinyGsmClient.h>
#include <Wire.h>

// Set up your SIM800L serial communication
#define GSM_RX 7
#define GSM_TX 8
SoftwareSerial SerialAT(GSM_RX, GSM_TX);
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqttClient(client);

// AWS IoT Core parameters
const char *aws_endpoint = "axrwpuwd874ur-ats.iot.ap-southeast-1.amazonaws.com";  // AWS IoT endpoint, e.g., "your-aws-endpoint.amazonaws.com"
const char *topic = "test/topic";  // Test topic to publish to
const char *clientID = "YOUR_CLIENT_ID"; // Client ID, can use your Thing name

void setup()
{
    Serial.begin(9600);
    SerialAT.begin(4800);

    // Initialize the GSM module
    Serial.println("Initializing GSM module...");
    modem.restart();
    
    // Connect to your GSM network (replace with your APN)
    if (!modem.gprsConnect("YOUR_APN", "", "")) {
        Serial.println("Failed to connect to GSM network");
        while (true);
    }

    // Set MQTT server details
    mqttClient.setServer(aws_endpoint, 1883); // Port 1883 for non-SSL connections

    // Connect to MQTT
    if (connectToMQTT()) {
        Serial.println("Connected to AWS IoT Core!");
    } else {
        Serial.println("Failed to connect to AWS IoT Core.");
    }
}

bool connectToMQTT()
{
    // Attempt to connect to MQTT broker
    if (mqttClient.connect(clientID)) {
        Serial.println("MQTT connected!");
        return true;
    } else {
        Serial.print("MQTT connection failed, rc=");
        Serial.print(mqttClient.state());
        return false;
    }
}

void loop()
{
    // Ensure the MQTT connection stays alive
    if (!mqttClient.connected()) {
        connectToMQTT();
    }
    mqttClient.loop();

    // Publish a test message to AWS IoT Core
    String message = "Hello from Arduino Nano!";
    if (mqttClient.publish(topic, message.c_str())) {
        Serial.println("Message published to AWS IoT Core.");
    } else {
        Serial.println("Failed to publish message.");
    }

    delay(5000);  // Wait before sending the next message
}
