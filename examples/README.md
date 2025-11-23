# Web Service Integration Examples

This directory contains examples and documentation for integrating PaddleOCR-json as a web service.

## Contents

- **`http_server_example.cpp`** - Proof-of-concept HTTP server implementation
- **`api_usage_examples.sh`** - API usage examples (curl, Python, JavaScript, Node.js)
- **`../docs/web-service-proposal.md`** - Detailed proposal document

## Quick Start

### 1. Review the Proposal

Read the comprehensive proposal:
```bash
cat ../docs/web-service-proposal.md
```

### 2. Understand the Architecture

The HTTP server integration is designed to:
- Accept binary file uploads via multipart/form-data
- Support base64 encoded images
- Return JSON responses matching current output format
- Work on both Linux and Windows
- Minimal code changes to existing codebase

### 3. See API Examples

View usage examples:
```bash
./api_usage_examples.sh
```

## Recommended Solution: cpp-httplib

**Why cpp-httplib?**
- ✅ Header-only (single file, easy integration)
- ✅ Cross-platform (Windows, Linux, macOS)
- ✅ MIT License (compatible)
- ✅ Built-in multipart/form-data support
- ✅ Production-ready features
- ✅ Active development

**Integration Time:** 3-5 days for basic implementation

## API Endpoints

### POST /api/ocr
Upload image and get OCR result.

**Request:**
```bash
curl -X POST http://localhost:8080/api/ocr \
     -F "image=@test.jpg"
```

**Response:**
```json
{
  "code": 100,
  "data": [
    {
      "text": "粤B82685",
      "score": 0.99,
      "box": [[973,900], [1278,846], [1295,939], [989,992]]
    }
  ],
  "processing_time_ms": 234
}
```

### POST /api/ocr/base64
Submit base64 encoded image.

**Request:**
```bash
IMAGE_BASE64=$(base64 -w 0 test.jpg)
curl -X POST http://localhost:8080/api/ocr/base64 \
     -H "Content-Type: application/json" \
     -d "{\"image\":\"$IMAGE_BASE64\"}"
```

### GET /api/health
Health check endpoint.

**Response:**
```json
{
  "status": "ok",
  "version": "v1.4.1",
  "timestamp": 1234567890
}
```

### GET /api/version
Version information.

**Response:**
```json
{
  "name": "PaddleOCR-json",
  "version": "v1.4.1",
  "api_version": "v1"
}
```

## Client Examples

### Python
```python
import requests

with open('test.jpg', 'rb') as f:
    files = {'image': f}
    response = requests.post('http://localhost:8080/api/ocr', files=files)
    result = response.json()
    print(result)
```

### JavaScript
```javascript
const formData = new FormData();
formData.append('image', fileInput.files[0]);

fetch('http://localhost:8080/api/ocr', {
    method: 'POST',
    body: formData
})
.then(response => response.json())
.then(data => console.log(data));
```

### Node.js
```javascript
const fs = require('fs');
const FormData = require('form-data');
const axios = require('axios');

const form = new FormData();
form.append('image', fs.createReadStream('test.jpg'));

axios.post('http://localhost:8080/api/ocr', form, {
    headers: form.getHeaders()
})
.then(response => console.log(response.data));
```

### curl
```bash
curl -X POST http://localhost:8080/api/ocr \
     -F "image=@test.jpg"
```

## Implementation Roadmap

### Phase 1: Basic HTTP Server (Week 1)
- [x] Research and select framework (cpp-httplib)
- [ ] Add cpp-httplib header to project
- [ ] Create http_server.cpp and http_server.h
- [ ] Implement /api/ocr endpoint
- [ ] Implement /api/health endpoint
- [ ] Update CMakeLists.txt
- [ ] Add --server flag to main.cpp
- [ ] Basic testing

### Phase 2: Production Features (Week 2)
- [ ] Configuration management
- [ ] Error handling improvements
- [ ] Request validation
- [ ] Rate limiting
- [ ] CORS support
- [ ] SSL/HTTPS support
- [ ] Performance optimization
- [ ] Memory management

### Phase 3: Deployment (Week 3)
- [ ] Docker image
- [ ] Docker Compose setup
- [ ] Systemd service (Linux)
- [ ] Windows Service setup
- [ ] API documentation
- [ ] Load testing
- [ ] Production deployment guide

## Performance Expectations

### Single Request
- Request handling: ~1-2ms
- OCR processing: 100-500ms (varies by image)
- Total: ~100-500ms per request

### Concurrent Requests (4 threads)
- Throughput: 8-40 requests/second
- Latency: ~100-500ms average

### Scalability
- Can scale horizontally with load balancer
- Each instance handles 4 concurrent requests
- Memory: ~500MB-1GB per instance

## Security Considerations

1. **File Size Limits**: Max 10MB per upload
2. **Rate Limiting**: 100 requests/min per IP (optional)
3. **Authentication**: API key support (optional)
4. **Input Validation**: File type and size checks
5. **HTTPS**: SSL/TLS support available

## Deployment Options

### Docker
```bash
docker run -d -p 8080:8080 \
  -v /path/to/models:/models \
  paddleocr-server \
  --server --port=8080
```

### Systemd (Linux)
```bash
sudo systemctl enable paddleocr-server
sudo systemctl start paddleocr-server
```

### Windows Service
```powershell
nssm install PaddleOCR "C:\PaddleOCR\PaddleOCR-json.exe" "--server --port=8080"
```

### Kubernetes
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: paddleocr-server
spec:
  replicas: 3
  template:
    spec:
      containers:
      - name: paddleocr
        image: paddleocr-server:latest
        ports:
        - containerPort: 8080
```

## Testing

### Unit Tests
```bash
./run_tests.sh --server
```

### Integration Tests
```bash
pytest tests/test_http_server.py
```

### Load Testing
```bash
# Apache Bench
ab -n 1000 -c 10 -p test.jpg http://localhost:8080/api/ocr

# wrk
wrk -t4 -c100 -d30s http://localhost:8080/api/health
```

## Monitoring

### Metrics to Track
- Request rate (req/s)
- Response time (ms)
- Error rate (%)
- Memory usage (MB)
- CPU usage (%)

### Logging
All requests are logged with:
- Timestamp
- Method and path
- Status code
- Processing time
- File size (if applicable)

## Next Steps

1. **Review Proposal**: Read `../docs/web-service-proposal.md`
2. **Approve Architecture**: Confirm cpp-httplib as framework
3. **Create Branch**: `feature/http-server-integration`
4. **Implement Phase 1**: Basic HTTP server
5. **Test & Deploy**: Production deployment

## Questions?

- Technical questions: See `../docs/web-service-proposal.md`
- API usage: Run `./api_usage_examples.sh`
- Integration help: Review `http_server_example.cpp`

## License

Same as PaddleOCR-json (Apache 2.0)

## References

- cpp-httplib: https://github.com/yhirose/cpp-httplib
- PaddleOCR: https://github.com/PaddlePaddle/PaddleOCR
- REST API Best Practices: https://restfulapi.net/
