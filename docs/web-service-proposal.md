# Web Service Integration Proposal for PaddleOCR-json

## Executive Summary

This document proposes integrating a C++ HTTP server into PaddleOCR-json to enable web service deployment. The solution will accept binary image uploads via form data and return JSON responses, compatible with both Linux and Windows.

## Current Architecture Analysis

### Existing Components
- **Core OCR Engine**: `PPOCR` class with `ocr()` method
- **Task Handler**: `Task` class with `run_ocr(string)` method
- **JSON Support**: Already using `nlohmann/json` library
- **Input Methods**:
  - File path
  - Base64 encoded images
  - Clipboard (Windows only)
- **Output Format**: JSON with structure:
  ```json
  {
    "code": 100,
    "data": [
      {
        "text": "recognized text",
        "score": 0.99,
        "box": [[x1,y1], [x2,y2], [x3,y3], [x4,y4]]
      }
    ]
  }
  ```

## Recommended Solutions

### Option 1: cpp-httplib (Recommended)

**Pros:**
- ✅ Header-only library (single file, ~10K LOC)
- ✅ Cross-platform (Windows, Linux, macOS)
- ✅ MIT License (compatible with Apache 2.0)
- ✅ Built-in multipart/form-data support
- ✅ Very easy to integrate (minimal code changes)
- ✅ Supports HTTPS, compression, chunked transfer
- ✅ Thread-pool for concurrent requests
- ✅ Active development (1.7K+ GitHub stars)

**Integration Complexity:** Low (1-2 days)

**Example Code:**
```cpp
#include "httplib.h"

httplib::Server svr;

svr.Post("/api/ocr", [](const httplib::Request &req, httplib::Response &res) {
    auto file = req.get_file_value("image");
    // Decode image from file.content
    cv::Mat img = cv::imdecode(
        cv::Mat(1, file.content.size(), CV_8UC1, (void*)file.content.data()),
        cv::IMREAD_COLOR
    );
    // Run OCR
    Task task;
    auto result = task.run_ocr_image(img);
    // Return JSON
    res.set_content(result, "application/json");
});

svr.listen("0.0.0.0", 8080);
```

**Repository:** https://github.com/yhirose/cpp-httplib

---

### Option 2: Crow

**Pros:**
- ✅ Header-only framework
- ✅ Flask-like syntax (Python developers friendly)
- ✅ Cross-platform
- ✅ BSD License (compatible)
- ✅ JSON support built-in
- ✅ WebSocket support
- ✅ Middleware support

**Cons:**
- ⚠️ Multipart form parsing requires additional library
- ⚠️ Less actively maintained than cpp-httplib

**Integration Complexity:** Medium (2-3 days)

**Repository:** https://github.com/CrowCpp/Crow

---

### Option 3: oatpp

**Pros:**
- ✅ Modern C++ framework (C++11+)
- ✅ High performance (async I/O)
- ✅ Built-in Swagger/OpenAPI support
- ✅ Cross-platform
- ✅ Apache 2.0 License
- ✅ Very well documented

**Cons:**
- ⚠️ Not header-only (requires compilation)
- ⚠️ More complex setup
- ⚠️ Steeper learning curve

**Integration Complexity:** High (4-5 days)

**Repository:** https://github.com/oatpp/oatpp

---

## Recommended Implementation Plan

### Phase 1: Basic HTTP Server (Week 1)

**Choice:** cpp-httplib (for simplicity and quick deployment)

**Tasks:**
1. Add cpp-httplib as header-only dependency
2. Create new `src/http_server.cpp` with HTTP endpoints
3. Implement `/api/ocr` endpoint accepting multipart form data
4. Update CMakeLists.txt to build server mode
5. Add command-line flag `--server` to enable server mode

**API Endpoints:**

#### POST /api/ocr
Upload image and get OCR result.

**Request:**
```http
POST /api/ocr HTTP/1.1
Content-Type: multipart/form-data; boundary=----WebKitFormBoundary

------WebKitFormBoundary
Content-Disposition: form-data; name="image"; filename="test.jpg"
Content-Type: image/jpeg

[binary image data]
------WebKitFormBoundary--
```

**Response:**
```json
{
  "code": 100,
  "data": [
    {
      "text": "粤B82685",
      "score": 0.9947937726974487,
      "box": [[973,900], [1278,846], [1295,939], [989,992]]
    }
  ]
}
```

#### GET /api/health
Health check endpoint.

**Response:**
```json
{
  "status": "ok",
  "version": "v1.4.1"
}
```

#### POST /api/ocr/base64
Accept base64 encoded image (alternative method).

**Request:**
```json
{
  "image": "base64_encoded_string_here"
}
```

### Phase 2: Production Features (Week 2)

1. **Configuration:**
   - Port configuration
   - Thread pool size
   - Max upload size
   - CORS support

2. **Error Handling:**
   - Invalid image format
   - File size limits
   - Rate limiting

3. **Performance:**
   - Request queue management
   - Memory pooling
   - Response compression

4. **Security:**
   - Request size limits (default 10MB)
   - Authentication (API key support)
   - HTTPS support (optional)

5. **Monitoring:**
   - Request logging
   - Performance metrics
   - Health monitoring

### Phase 3: Documentation & Deployment (Week 3)

1. **Documentation:**
   - API documentation (OpenAPI/Swagger)
   - Deployment guide
   - Docker support

2. **Deployment Scripts:**
   - Systemd service file (Linux)
   - Windows Service wrapper
   - Docker image
   - Docker Compose setup

---

## Proposed File Structure

```
PaddleOCR-json/
├── cpp/
│   ├── include/
│   │   ├── httplib.h              # cpp-httplib header
│   │   └── http_server.h          # Server class declaration
│   ├── src/
│   │   ├── main.cpp               # Entry point (add --server flag)
│   │   ├── http_server.cpp        # HTTP server implementation
│   │   └── task.cpp               # Modified to support cv::Mat input
│   ├── CMakeLists.txt             # Updated build config
│   └── configs/
│       └── server_config.json     # Server configuration
├── docker/
│   ├── Dockerfile                 # Docker image
│   └── docker-compose.yml         # Docker compose
├── deployment/
│   ├── systemd/
│   │   └── paddleocr-server.service
│   └── windows/
│       └── install-service.ps1
└── docs/
    └── api-documentation.md
```

---

## Code Example: Integration

### http_server.h
```cpp
#pragma once

#include "httplib.h"
#include "include/task.h"

namespace PaddleOCR {

class HttpServer {
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
    void handle_ocr(const httplib::Request& req, httplib::Response& res);
    void handle_ocr_base64(const httplib::Request& req, httplib::Response& res);
    void handle_health(const httplib::Request& req, httplib::Response& res);

    // Helper methods
    cv::Mat decode_image_from_bytes(const std::string& data);
    std::string create_error_response(int code, const std::string& message);
};

}
```

### http_server.cpp (Simplified)
```cpp
#include "include/http_server.h"
#include "include/nlohmann/json.hpp"
#include <opencv2/imgcodecs.hpp>

namespace PaddleOCR {

HttpServer::HttpServer(int port) : port_(port) {
    task_ = std::make_unique<Task>();

    // Initialize OCR engine
    task_->init_engine();

    // Setup routes
    server_.Post("/api/ocr", [this](auto& req, auto& res) {
        handle_ocr(req, res);
    });

    server_.Post("/api/ocr/base64", [this](auto& req, auto& res) {
        handle_ocr_base64(req, res);
    });

    server_.Get("/api/health", [this](auto& req, auto& res) {
        handle_health(req, res);
    });

    // Set CORS headers
    server_.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "POST, GET, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type"}
    });
}

void HttpServer::handle_ocr(const httplib::Request& req, httplib::Response& res) {
    try {
        // Get uploaded file
        if (!req.has_file("image")) {
            res.status = 400;
            res.set_content(create_error_response(400, "No image file provided"),
                          "application/json");
            return;
        }

        auto file = req.get_file_value("image");

        // Decode image
        cv::Mat img = decode_image_from_bytes(file.content);
        if (img.empty()) {
            res.status = 400;
            res.set_content(create_error_response(400, "Invalid image format"),
                          "application/json");
            return;
        }

        // Run OCR
        std::string result = task_->run_ocr_image(img);

        // Return result
        res.set_content(result, "application/json");

    } catch (const std::exception& e) {
        res.status = 500;
        res.set_content(create_error_response(500, e.what()),
                      "application/json");
    }
}

cv::Mat HttpServer::decode_image_from_bytes(const std::string& data) {
    std::vector<uchar> buffer(data.begin(), data.end());
    return cv::imdecode(buffer, cv::IMREAD_COLOR);
}

void HttpServer::start() {
    std::cout << "Starting HTTP server on port " << port_ << std::endl;
    server_.listen("0.0.0.0", port_);
}

}
```

### Updated main.cpp
```cpp
int main(int argc, char **argv) {
    // ... existing code ...

    if (FLAGS_server) {
        // Server mode
        PaddleOCR::HttpServer server(FLAGS_port);
        server.start();
        return 0;
    }

    // CLI mode (existing code)
    Task task = Task();
    if (FLAGS_type == "ocr") {
        return task.ocr();
    }
}
```

---

## Performance Estimates

### Single-threaded Performance
- **Request handling**: ~1-2ms overhead
- **OCR processing**: 100-500ms (depends on image size)
- **Total**: ~100-500ms per request

### Multi-threaded (Thread Pool: 4)
- **Concurrent requests**: 4 simultaneous
- **Throughput**: ~8-40 requests/second

### Optimization Strategies
1. **Keep OCR engine loaded** (avoid re-initialization)
2. **Use thread pool** for concurrent requests
3. **Image preprocessing** caching
4. **Response compression** (gzip)

---

## Security Considerations

1. **File Size Limits**: Max 10MB per upload
2. **Rate Limiting**: Optional (100 req/min per IP)
3. **Authentication**: API key support (optional)
4. **Input Validation**:
   - Check file type (JPEG, PNG, BMP, TIFF)
   - Validate image dimensions
   - Sanitize file names
5. **HTTPS**: SSL/TLS support via cpp-httplib

---

## Deployment Examples

### Docker Deployment
```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    gcc g++ cmake make libgomp1

COPY . /app
WORKDIR /app

RUN ./build_linux.sh

EXPOSE 8080

CMD ["./cpp/build/bin/PaddleOCR-json", \
     "--server", \
     "--port=8080", \
     "--models_path=/app/models", \
     "--config_path=/app/models/config_chinese.txt"]
```

### Systemd Service (Linux)
```ini
[Unit]
Description=PaddleOCR HTTP Service
After=network.target

[Service]
Type=simple
User=www-data
WorkingDirectory=/opt/paddleocr
ExecStart=/opt/paddleocr/bin/PaddleOCR-json \
    --server \
    --port=8080 \
    --models_path=/opt/paddleocr/models \
    --config_path=/opt/paddleocr/models/config_chinese.txt
Restart=always

[Install]
WantedBy=multi-user.target
```

### Windows Service
```powershell
# Using NSSM (Non-Sucking Service Manager)
nssm install PaddleOCR "C:\PaddleOCR\PaddleOCR-json.exe" `
    "--server --port=8080 --models_path=C:\PaddleOCR\models"
nssm start PaddleOCR
```

---

## Testing Strategy

### Unit Tests
- Image decoding
- JSON parsing
- Error handling

### Integration Tests
- Full OCR pipeline
- Concurrent requests
- Memory leak detection

### Load Testing
```bash
# Using Apache Bench
ab -n 1000 -c 10 -p test.jpg -T 'multipart/form-data' \
   http://localhost:8080/api/ocr

# Using wrk
wrk -t4 -c100 -d30s --latency http://localhost:8080/api/health
```

---

## Estimated Timeline

| Phase | Tasks | Duration |
|-------|-------|----------|
| **Phase 1** | Basic HTTP server integration | 3-5 days |
| **Phase 2** | Production features | 5-7 days |
| **Phase 3** | Documentation & deployment | 2-3 days |
| **Testing** | Full testing suite | 2-3 days |
| **Total** | | **12-18 days** |

---

## Conclusion

**Recommended Approach:** Integrate **cpp-httplib** for the following reasons:

1. ✅ Minimal integration effort (header-only)
2. ✅ Cross-platform compatibility (Windows + Linux)
3. ✅ Built-in form data support
4. ✅ Active maintenance and community
5. ✅ Production-ready features (SSL, compression, etc.)
6. ✅ Compatible licensing

**Next Steps:**
1. Approve this proposal
2. Create feature branch for web service integration
3. Implement Phase 1 (basic HTTP server)
4. Test and iterate
5. Deploy to production

**Alternative:** If advanced features like async I/O or WebSocket support are needed in the future, we can migrate to **oatpp** or **Drogon**.

---

## References

- cpp-httplib: https://github.com/yhirose/cpp-httplib
- Crow: https://github.com/CrowCpp/Crow
- oatpp: https://github.com/oatpp/oatpp
- nlohmann/json: https://github.com/nlohmann/json
- PaddleOCR: https://github.com/PaddlePaddle/PaddleOCR
