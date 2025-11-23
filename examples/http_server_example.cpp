// Example HTTP Server Integration for PaddleOCR-json
// This is a proof-of-concept showing how to integrate cpp-httplib
//
// Compile: g++ -std=c++11 -I../cpp/include http_server_example.cpp -o server
// Usage: ./server 8080

#include <iostream>
#include <memory>

// NOTE: In actual implementation, download cpp-httplib from:
// https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
// #include "httplib.h"

// Simulated includes (replace with actual in implementation)
// #include "include/paddleocr.h"
// #include "include/task.h"
// #include "include/nlohmann/json.hpp"
// #include <opencv2/imgcodecs.hpp>

/*
 * Complete implementation would look like this:
 */

/*
class OCRHttpServer {
public:
    OCRHttpServer(int port, const std::string& models_path, const std::string& config_path)
        : port_(port), models_path_(models_path), config_path_(config_path) {

        std::cout << "Initializing OCR HTTP Server..." << std::endl;
        std::cout << "Models path: " << models_path_ << std::endl;
        std::cout << "Config path: " << config_path_ << std::endl;

        // Initialize OCR engine
        task_ = std::make_unique<PaddleOCR::Task>();
        task_->init_engine();

        std::cout << "OCR engine initialized successfully" << std::endl;

        setup_routes();
    }

    void start() {
        std::cout << "Starting HTTP server on 0.0.0.0:" << port_ << std::endl;
        std::cout << "API Endpoints:" << std::endl;
        std::cout << "  POST /api/ocr         - Upload image for OCR" << std::endl;
        std::cout << "  POST /api/ocr/base64  - Submit base64 encoded image" << std::endl;
        std::cout << "  GET  /api/health      - Health check" << std::endl;
        std::cout << "  GET  /api/version     - Version info" << std::endl;
        std::cout << std::endl;

        server_.listen("0.0.0.0", port_);
    }

    void stop() {
        server_.stop();
    }

private:
    int port_;
    std::string models_path_;
    std::string config_path_;
    httplib::Server server_;
    std::unique_ptr<PaddleOCR::Task> task_;

    void setup_routes() {
        // CORS middleware
        server_.set_default_headers({
            {"Access-Control-Allow-Origin", "*"},
            {"Access-Control-Allow-Methods", "POST, GET, OPTIONS"},
            {"Access-Control-Allow-Headers", "Content-Type, Authorization"}
        });

        // Health check endpoint
        server_.Get("/api/health", [this](const httplib::Request& req, httplib::Response& res) {
            handle_health(req, res);
        });

        // Version endpoint
        server_.Get("/api/version", [this](const httplib::Request& req, httplib::Response& res) {
            handle_version(req, res);
        });

        // OCR endpoint - multipart form data
        server_.Post("/api/ocr", [this](const httplib::Request& req, httplib::Response& res) {
            handle_ocr_upload(req, res);
        });

        // OCR endpoint - base64 JSON
        server_.Post("/api/ocr/base64", [this](const httplib::Request& req, httplib::Response& res) {
            handle_ocr_base64(req, res);
        });

        // Set max request body size (10MB)
        server_.set_payload_max_length(10 * 1024 * 1024);

        // Set read timeout
        server_.set_read_timeout(30, 0); // 30 seconds
        server_.set_write_timeout(30, 0);
    }

    void handle_health(const httplib::Request& req, httplib::Response& res) {
        nlohmann::json response = {
            {"status", "ok"},
            {"version", PROJECT_VER},
            {"timestamp", std::time(nullptr)}
        };

        res.set_content(response.dump(), "application/json");
    }

    void handle_version(const httplib::Request& req, httplib::Response& res) {
        nlohmann::json response = {
            {"name", "PaddleOCR-json"},
            {"version", PROJECT_VER},
            {"api_version", "v1"},
            {"models_path", models_path_},
            {"config_path", config_path_}
        };

        res.set_content(response.dump(), "application/json");
    }

    void handle_ocr_upload(const httplib::Request& req, httplib::Response& res) {
        auto start_time = std::chrono::high_resolution_clock::now();

        try {
            // Check if image file is present
            if (!req.has_file("image")) {
                send_error(res, 400, "No image file provided. Use 'image' field in form data.");
                return;
            }

            // Get uploaded file
            auto file = req.get_file_value("image");

            std::cout << "Received file: " << file.filename
                     << " (" << file.content.size() << " bytes)" << std::endl;

            // Validate file size
            if (file.content.size() > 10 * 1024 * 1024) {
                send_error(res, 413, "File size exceeds 10MB limit");
                return;
            }

            // Decode image from bytes
            cv::Mat img = decode_image_from_bytes(file.content);

            if (img.empty()) {
                send_error(res, 400, "Invalid image format. Supported: JPEG, PNG, BMP, TIFF");
                return;
            }

            std::cout << "Image decoded: " << img.cols << "x" << img.rows << std::endl;

            // Run OCR
            std::string result = task_->run_ocr_image(img);

            // Calculate processing time
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                end_time - start_time).count();

            std::cout << "OCR completed in " << duration << "ms" << std::endl;

            // Add processing time to response
            nlohmann::json result_json = nlohmann::json::parse(result);
            result_json["processing_time_ms"] = duration;

            // Return result
            res.set_content(result_json.dump(), "application/json");

        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            send_error(res, 500, std::string("Internal server error: ") + e.what());
        }
    }

    void handle_ocr_base64(const httplib::Request& req, httplib::Response& res) {
        try {
            // Parse JSON body
            nlohmann::json body = nlohmann::json::parse(req.body);

            if (!body.contains("image")) {
                send_error(res, 400, "Missing 'image' field in JSON body");
                return;
            }

            std::string base64_str = body["image"];

            // Use existing base64 handling from Task class
            std::string result = task_->run_ocr(base64_str);

            // Return result
            res.set_content(result, "application/json");

        } catch (const nlohmann::json::parse_error& e) {
            send_error(res, 400, std::string("Invalid JSON: ") + e.what());
        } catch (const std::exception& e) {
            send_error(res, 500, std::string("Internal server error: ") + e.what());
        }
    }

    cv::Mat decode_image_from_bytes(const std::string& data) {
        std::vector<uchar> buffer(data.begin(), data.end());
        return cv::imdecode(buffer, cv::IMREAD_COLOR);
    }

    void send_error(httplib::Response& res, int status_code, const std::string& message) {
        nlohmann::json error_response = {
            {"code", status_code},
            {"error", message}
        };

        res.status = status_code;
        res.set_content(error_response.dump(), "application/json");
    }
};

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <port> <models_path> <config_path>" << std::endl;
        std::cerr << "Example: " << argv[0] << " 8080 ./models ./models/config_chinese.txt" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);
    std::string models_path = argv[2];
    std::string config_path = argv[3];

    try {
        OCRHttpServer server(port, models_path, config_path);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
*/

// Minimal example showing the concept
int main() {
    std::cout << "=== PaddleOCR-json HTTP Server Example ===" << std::endl;
    std::cout << std::endl;
    std::cout << "This is a proof-of-concept example." << std::endl;
    std::cout << "See docs/web-service-proposal.md for full implementation details." << std::endl;
    std::cout << std::endl;
    std::cout << "Key Features:" << std::endl;
    std::cout << "  1. Cross-platform (Linux + Windows)" << std::endl;
    std::cout << "  2. RESTful API with JSON responses" << std::endl;
    std::cout << "  3. Multipart form-data file uploads" << std::endl;
    std::cout << "  4. Base64 image support" << std::endl;
    std::cout << "  5. CORS support" << std::endl;
    std::cout << "  6. Health check endpoint" << std::endl;
    std::cout << std::endl;
    std::cout << "Integration Steps:" << std::endl;
    std::cout << "  1. Download cpp-httplib (header-only, single file)" << std::endl;
    std::cout << "  2. Add http_server.cpp and http_server.h to project" << std::endl;
    std::cout << "  3. Update main.cpp with --server flag" << std::endl;
    std::cout << "  4. Build and deploy" << std::endl;
    std::cout << std::endl;
    std::cout << "Estimated effort: 3-5 days for basic implementation" << std::endl;
    std::cout << std::endl;

    return 0;
}
