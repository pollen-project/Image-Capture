#include <WiFi.h>
#include "esp_camera.h"
#include "time_config.h"

String serverName = "pollen-backend.azurewebsites.net";
int port = 80;
String serverPath = "/image/upload";
WiFiClient client;
void post_picture() {
  if (WiFi.status() == WL_CONNECTED) {
    if (client.connect(serverName.c_str(), port)) {
      Serial.println("Connection successful!");

      camera_fb_t *fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Failed to capture image");
        return;
      }

      const size_t maxTimeBufferSize = 30; // Maximum size for the time string buffer
      char timeBuffer[maxTimeBufferSize]; // Buffer to hold the formatted time string
      get_time(timeBuffer, maxTimeBufferSize);
      // Construct the filename with the time string
      String filename = String(timeBuffer) + ".jpg";

      String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"file\"; filename=\"" + filename + "\"\r\nContent-Type: image/jpeg\r\n\r\n";
      String tail = "\r\n--RandomNerdTutorials--\r\n";
      size_t imageLen = fb->len;
      size_t extraLen = head.length() + tail.length();
      size_t totalLen = imageLen + extraLen;

      // Send HTTP POST request headers
      client.print("POST " + serverPath + " HTTP/1.1\r\n");
      client.print("Host: " + serverName + ":" + String(port) + "\r\n");
      client.print("Content-Type: multipart/form-data; boundary=RandomNerdTutorials\r\n");
      client.print("Content-Length: " + String(totalLen) + "\r\n");
      client.print("\r\n");

      // Send image data
      client.print(head);
      client.write(fb->buf, fb->len);
      client.print(tail);

      // Wait for response
      delay(1000); // Allow time for server to process

      Serial.println("Response from server:");
      // Read and print server response
      while (client.available()) {
        char c = client.read();
        Serial.print(c);
      }
      Serial.println(); // Print a newline for clarity
      
      // Clean up
      client.stop();
      esp_camera_fb_return(fb);
      Serial.println(filename + ": Sent");
    } else {
      Serial.println("Connection failed!");
    }
  }
}
