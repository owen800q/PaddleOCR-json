// HTTP Server for PaddleOCR-json
// Provides RESTful API for OCR service

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "include/httplib.h"
#include "include/task.h"
#include "opencv2/core.hpp"
#include <memory>
#include <string>

namespace PaddleOCR
{
    class HttpServer
    {
    public:
        HttpServer(int port = 8080);
        ~HttpServer() = default;

        void start();
        void stop();

    private:
        int port_;
        httplib::Server server_;
        std::unique_ptr<Task> task_;

        // Route handlers
        void handle_ocr_upload(const httplib::Request &req, httplib::Response &res);
        void handle_ocr_base64(const httplib::Request &req, httplib::Response &res);
        void handle_health(const httplib::Request &req, httplib::Response &res);
        void handle_version(const httplib::Request &req, httplib::Response &res);

        // Helper methods
        cv::Mat decode_image_from_bytes(const std::string &data);
        std::string create_error_response(int code, const std::string &message);
        void setup_routes();
        void log_request(const std::string &method, const std::string &path, int status, long duration_ms);
    };
}

#endif // HTTP_SERVER_H
