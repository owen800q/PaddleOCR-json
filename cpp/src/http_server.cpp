// HTTP Server implementation for PaddleOCR-json

#include "include/http_server.h"
#include "include/nlohmann/json.hpp"
#include "include/args.h"
#include "include/base64.h"
#include <opencv2/imgcodecs.hpp>
#include <chrono>
#include <iostream>

#define PROJECT_VER "v1.4.1 dev.1"

namespace PaddleOCR
{
    HttpServer::HttpServer(int port) : port_(port)
    {
        std::cout << "Initializing OCR HTTP Server on port " << port_ << "..." << std::endl;

        // Initialize task and OCR engine
        task_.reset(new Task());
        std::cout << "Initializing OCR engine..." << std::endl;
        task_->init_engine();
        std::cout << "OCR engine initialized successfully" << std::endl;

        // Setup routes
        setup_routes();

        // Set idle interval for better macOS compatibility
        // This enables select() polling instead of blocking accept()
        server_.set_idle_interval(0, 100000); // 100ms idle interval
    }

    void HttpServer::setup_routes()
    {
        // CORS middleware
        server_.set_default_headers({
            {"Access-Control-Allow-Origin", "*"},
            {"Access-Control-Allow-Methods", "POST, GET, OPTIONS"},
            {"Access-Control-Allow-Headers", "Content-Type, Authorization"}
        });

        // Health check endpoint
        server_.Get("/api/health", [this](const httplib::Request &req, httplib::Response &res)
                    {
            auto start_time = std::chrono::high_resolution_clock::now();
            handle_health(req, res);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            log_request("GET", "/api/health", res.status, duration); });

        // Version endpoint
        server_.Get("/api/version", [this](const httplib::Request &req, httplib::Response &res)
                    {
            auto start_time = std::chrono::high_resolution_clock::now();
            handle_version(req, res);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            log_request("GET", "/api/version", res.status, duration); });

        // OCR endpoint - multipart form data
        server_.Post("/api/ocr", [this](const httplib::Request &req, httplib::Response &res)
                     {
            auto start_time = std::chrono::high_resolution_clock::now();
            handle_ocr_upload(req, res);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            log_request("POST", "/api/ocr", res.status, duration); });

        // OCR endpoint - base64 JSON
        server_.Post("/api/ocr/base64", [this](const httplib::Request &req, httplib::Response &res)
                     {
            auto start_time = std::chrono::high_resolution_clock::now();
            handle_ocr_base64(req, res);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            log_request("POST", "/api/ocr/base64", res.status, duration); });

        // Set max request body size (10MB)
        server_.set_payload_max_length(10 * 1024 * 1024);

        // Set timeouts
        server_.set_read_timeout(30, 0);  // 30 seconds
        server_.set_write_timeout(30, 0); // 30 seconds
    }

    void HttpServer::handle_health(const httplib::Request &req, httplib::Response &res)
    {
        nlohmann::json response = {
            {"status", "ok"},
            {"version", PROJECT_VER},
            {"timestamp", std::time(nullptr)}};

        res.set_content(response.dump(), "application/json");
    }

    void HttpServer::handle_version(const httplib::Request &req, httplib::Response &res)
    {
        nlohmann::json response = {
            {"name", "PaddleOCR-json"},
            {"version", PROJECT_VER},
            {"api_version", "v1"}};

        res.set_content(response.dump(), "application/json");
    }

    void HttpServer::handle_ocr_upload(const httplib::Request &req, httplib::Response &res)
    {
        try
        {
            // Check if image file is present
            if (!req.form.has_file("image"))
            {
                res.status = 400;
                res.set_content(create_error_response(400, "No image file provided. Use 'image' field in form data."),
                                "application/json");
                return;
            }

            // Get uploaded file
            auto file = req.form.get_file("image");

            std::cout << "Received file: " << file.filename
                      << " (" << file.content.size() << " bytes)" << std::endl;

            // Validate file size
            if (file.content.size() > 10 * 1024 * 1024)
            {
                res.status = 413;
                res.set_content(create_error_response(413, "File size exceeds 10MB limit"),
                                "application/json");
                return;
            }

            // Decode image from bytes
            cv::Mat img = decode_image_from_bytes(file.content);

            if (img.empty())
            {
                res.status = 400;
                res.set_content(create_error_response(400, "Invalid image format. Supported: JPEG, PNG, BMP, TIFF"),
                                "application/json");
                return;
            }

            std::cout << "Image decoded: " << img.cols << "x" << img.rows << std::endl;

            // Run OCR
            std::string result = task_->run_ocr_mat(img);

            // Parse result to add processing time
            try
            {
                nlohmann::json result_json = nlohmann::json::parse(result);
                res.set_content(result_json.dump(), "application/json");
            }
            catch (...)
            {
                res.set_content(result, "application/json");
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            res.status = 500;
            res.set_content(create_error_response(500, std::string("Internal server error: ") + e.what()),
                            "application/json");
        }
    }

    void HttpServer::handle_ocr_base64(const httplib::Request &req, httplib::Response &res)
    {
        try
        {
            // Parse JSON body
            nlohmann::json body = nlohmann::json::parse(req.body);

            if (!body.contains("image"))
            {
                res.status = 400;
                res.set_content(create_error_response(400, "Missing 'image' field in JSON body"),
                                "application/json");
                return;
            }

            std::string base64_str = body["image"];

            // Decode base64 to bytes
            std::string decoded_str;
            try
            {
                // Simple base64 decode (you may need to include base64.h)
                decoded_str = base64_decode(base64_str);
            }
            catch (...)
            {
                res.status = 400;
                res.set_content(create_error_response(400, "Invalid base64 encoding"),
                                "application/json");
                return;
            }

            // Decode image from bytes
            cv::Mat img = decode_image_from_bytes(decoded_str);

            if (img.empty())
            {
                res.status = 400;
                res.set_content(create_error_response(400, "Invalid image format"),
                                "application/json");
                return;
            }

            // Run OCR
            std::string result = task_->run_ocr_mat(img);

            // Return result
            res.set_content(result, "application/json");
        }
        catch (const nlohmann::json::parse_error &e)
        {
            res.status = 400;
            res.set_content(create_error_response(400, std::string("Invalid JSON: ") + e.what()),
                            "application/json");
        }
        catch (const std::exception &e)
        {
            res.status = 500;
            res.set_content(create_error_response(500, std::string("Internal server error: ") + e.what()),
                            "application/json");
        }
    }

    cv::Mat HttpServer::decode_image_from_bytes(const std::string &data)
    {
        std::vector<uchar> buffer(data.begin(), data.end());
        return cv::imdecode(buffer, cv::IMREAD_COLOR);
    }

    std::string HttpServer::create_error_response(int code, const std::string &message)
    {
        nlohmann::json error_response = {
            {"code", code},
            {"error", message}};

        return error_response.dump();
    }

    void HttpServer::log_request(const std::string &method, const std::string &path, int status, long duration_ms)
    {
        std::cout << "[" << method << "] " << path
                  << " - Status: " << status
                  << " - Duration: " << duration_ms << "ms"
                  << std::endl;
    }

    void HttpServer::start()
    {
        std::cout << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "PaddleOCR-json HTTP Server" << std::endl;
        std::cout << "Version: " << PROJECT_VER << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << std::endl;
        std::cout << "Server binding to 127.0.0.1:" << port_ << std::endl;
        std::cout << std::endl;
        std::cout << "API Endpoints:" << std::endl;
        std::cout << "  POST http://localhost:" << port_ << "/api/ocr         - Upload image for OCR" << std::endl;
        std::cout << "  POST http://localhost:" << port_ << "/api/ocr/base64  - Submit base64 encoded image" << std::endl;
        std::cout << "  GET  http://localhost:" << port_ << "/api/health      - Health check" << std::endl;
        std::cout << "  GET  http://localhost:" << port_ << "/api/version     - Version info" << std::endl;
        std::cout << std::endl;
        std::cout << "Example:" << std::endl;
        std::cout << "  curl -X POST http://localhost:" << port_ << "/api/ocr -F \"image=@test.jpg\"" << std::endl;
        std::cout << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << std::endl;
        std::cout.flush();

        // Bind to 127.0.0.1 for better macOS compatibility
        if (!server_.listen("127.0.0.1", port_)) {
            std::cerr << "Failed to start server on 127.0.0.1:" << port_ << std::endl;
            std::cerr << "Trying 0.0.0.0..." << std::endl;
            server_.listen("0.0.0.0", port_);
        }
    }

    void HttpServer::stop()
    {
        server_.stop();
    }
}
