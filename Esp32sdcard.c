#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <SD.h>  

const char* ssid = "IMPARATOR2.0";
const char* password = "182207MROA";

WebServer server(80);

void setup() {
    Serial.begin(115200);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // SD kart modulu veya kartin kendisinin calistigini kontrol eden komut.
    if (!SD.begin()) {
        Serial.println("Card Mount Failed");
        return;
    }
    Serial.println("SD card mounted");

    // Route for root
    server.on("/", HTTP_GET, [](){
        String html = "<html><body>";
        html += "<h1>ESP32 File Management</h1>";
        
        html += "<h2>Upload a File:</h2>";
        html += "<form method='POST' action='/upload' enctype='multipart/form-data'>";
        html += "<input type='file' name='file'>";
        html += "<input type='submit' value='Upload'>";
        html += "</form>";

        html += "<h2>Download Files:</h2>";
        File root = SD.open("/");
        if (root) {
            html += "<ul>";
            while (File file = root.openNextFile()) {
                html += "<li><a href='/download?file=";
                html += file.name();
                html += "'>";
                html += file.name();
                html += "</a></li>";
                file.close();
            }
            html += "</ul>";
            root.close();
        } else {
            html += "<p>Failed to open directory</p>";
        }

        html += "</body></html>";

        server.send(200, "text/html", html);
    });

    // Yuklemeler icin dosyayi yonlendiren kod
    server.on("/upload", HTTP_POST, [](){
        HTTPUpload& upload = server.upload();
        static File fsUploadFile;

        if (upload.status == UPLOAD_FILE_START) {
            String filename = "/" + upload.filename;
            fsUploadFile = SD.open(filename, FILE_WRITE);
            if (!fsUploadFile) {
                Serial.println("Failed to open file for writing");
                server.send(500, "text/plain", "Failed to open file for writing");
                return;
            }
            Serial.print("Uploading: ");
            Serial.println(filename);
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (fsUploadFile) {
                fsUploadFile.write(upload.buf, upload.currentSize);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (fsUploadFile) {
                fsUploadFile.close();
                Serial.println("Upload finished");
            }
        }
        server.send(200, "text/plain", "File uploaded successfully");
    });

    // Indirmeler icin dosyayi yonlendiren kod
    server.on("/download", HTTP_GET, [](){
        String filename = server.arg("file");

        // SD kart icerigini okuma
        File file = SD.open("/" + filename);
        if (!file) {
            server.send(404, "text/plain", "File not found");
            return;
        }

        // Dosya iceriklerini uzantilarina gore ayiklayan komut
        String contentType = getContentType(filename);
        server.streamFile(file, contentType);  // Dosyayi uygun uzantiyla sayfaya yansitan komut
        file.close();
    });

    // Start server
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}

// Dosya tipini belirleyen fonksiyon
String getContentType(String filename) {
    if (filename.endsWith(".htm") || filename.endsWith(".html")) {
        return "text/html";
    } else if (filename.endsWith(".css")) {
        return "text/css";
    } else if (filename.endsWith(".js")) {
        return "application/javascript";
    } else if (filename.endsWith(".png")) {
        return "image/png";
    } else if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) {
        return "image/jpeg";
    } else if (filename.endsWith(".gif")) {
        return "image/gif";
    } else if (filename.endsWith(".ico")) {
        return "image/x-icon";
    } else if (filename.endsWith(".xml")) {
        return "text/xml";
    } else if (filename.endsWith(".pdf")) {
        return "application/pdf";
    } else if (filename.endsWith(".zip")) {
        return "application/zip";
    } else if (filename.endsWith(".json")) {
        return "application/json";
    } else if (filename.endsWith(".iso")) {
        return "diskimage/iso";
    } else {
        return "application/octet-stream";
    }
}