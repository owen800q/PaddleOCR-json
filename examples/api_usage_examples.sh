#!/bin/bash

# API Usage Examples for PaddleOCR-json HTTP Server
# These examples show how to interact with the OCR web service

SERVER_URL="http://localhost:8080"

echo "=== PaddleOCR-json API Usage Examples ==="
echo ""

# Example 1: Health Check
echo "1. Health Check"
echo "   curl ${SERVER_URL}/api/health"
echo ""
echo "   Expected Response:"
echo '   {"status":"ok","version":"v1.4.1","timestamp":1234567890}'
echo ""
echo "---"
echo ""

# Example 2: Version Info
echo "2. Version Info"
echo "   curl ${SERVER_URL}/api/version"
echo ""
echo "   Expected Response:"
echo '   {"name":"PaddleOCR-json","version":"v1.4.1","api_version":"v1"}'
echo ""
echo "---"
echo ""

# Example 3: OCR via File Upload (multipart/form-data)
echo "3. OCR - Upload Image File"
echo "   curl -X POST ${SERVER_URL}/api/ocr \\"
echo "        -F \"image=@/path/to/image.jpg\""
echo ""
echo "   Expected Response:"
echo '   {
     "code": 100,
     "data": [
       {
         "text": "粤B82685",
         "score": 0.9947937726974487,
         "box": [[973,900],[1278,846],[1295,939],[989,992]]
       }
     ],
     "processing_time_ms": 234
   }'
echo ""
echo "---"
echo ""

# Example 4: OCR via Base64
echo "4. OCR - Base64 Encoded Image"
echo "   # First, encode image to base64"
echo "   IMAGE_BASE64=\$(base64 -w 0 /path/to/image.jpg)"
echo ""
echo "   # Then send to API"
echo "   curl -X POST ${SERVER_URL}/api/ocr/base64 \\"
echo "        -H \"Content-Type: application/json\" \\"
echo "        -d '{\"image\":\"'\$IMAGE_BASE64'\"}'"
echo ""
echo "   Expected Response:"
echo '   {
     "code": 100,
     "data": [...]
   }'
echo ""
echo "---"
echo ""

# Example 5: Using Python requests
echo "5. Python Example"
cat << 'PYTHON_EOF'
   import requests

   # Upload file
   with open('/path/to/image.jpg', 'rb') as f:
       files = {'image': f}
       response = requests.post('http://localhost:8080/api/ocr', files=files)
       result = response.json()
       print(result)

   # Or use base64
   import base64
   with open('/path/to/image.jpg', 'rb') as f:
       image_data = base64.b64encode(f.read()).decode('utf-8')
       response = requests.post(
           'http://localhost:8080/api/ocr/base64',
           json={'image': image_data}
       )
       result = response.json()
       print(result)
PYTHON_EOF
echo ""
echo "---"
echo ""

# Example 6: Using JavaScript fetch
echo "6. JavaScript (Browser) Example"
cat << 'JS_EOF'
   // Upload file from file input
   const fileInput = document.getElementById('fileInput');
   const formData = new FormData();
   formData.append('image', fileInput.files[0]);

   fetch('http://localhost:8080/api/ocr', {
       method: 'POST',
       body: formData
   })
   .then(response => response.json())
   .then(data => console.log(data))
   .catch(error => console.error('Error:', error));

   // Or use base64
   const reader = new FileReader();
   reader.onload = function(e) {
       const base64 = e.target.result.split(',')[1];
       fetch('http://localhost:8080/api/ocr/base64', {
           method: 'POST',
           headers: {'Content-Type': 'application/json'},
           body: JSON.stringify({image: base64})
       })
       .then(response => response.json())
       .then(data => console.log(data));
   };
   reader.readAsDataURL(fileInput.files[0]);
JS_EOF
echo ""
echo "---"
echo ""

# Example 7: Using Node.js
echo "7. Node.js Example"
cat << 'NODE_EOF'
   const fs = require('fs');
   const FormData = require('form-data');
   const axios = require('axios');

   // Upload file
   const form = new FormData();
   form.append('image', fs.createReadStream('/path/to/image.jpg'));

   axios.post('http://localhost:8080/api/ocr', form, {
       headers: form.getHeaders()
   })
   .then(response => console.log(response.data))
   .catch(error => console.error(error));

   // Or use base64
   const imageBuffer = fs.readFileSync('/path/to/image.jpg');
   const base64Image = imageBuffer.toString('base64');

   axios.post('http://localhost:8080/api/ocr/base64', {
       image: base64Image
   })
   .then(response => console.log(response.data))
   .catch(error => console.error(error));
NODE_EOF
echo ""
echo "---"
echo ""

# Example 8: Using Postman
echo "8. Postman Configuration"
echo ""
echo "   Method: POST"
echo "   URL: http://localhost:8080/api/ocr"
echo "   Body Type: form-data"
echo "   Key: image"
echo "   Value: [Select File]"
echo ""
echo "---"
echo ""

# Example 9: Error Handling
echo "9. Error Responses"
echo ""
echo "   400 Bad Request:"
echo '   {
     "code": 400,
     "error": "No image file provided"
   }'
echo ""
echo "   413 Payload Too Large:"
echo '   {
     "code": 413,
     "error": "File size exceeds 10MB limit"
   }'
echo ""
echo "   500 Internal Server Error:"
echo '   {
     "code": 500,
     "error": "Internal server error: ..."
   }'
echo ""
echo "---"
echo ""

# Example 10: Batch Processing
echo "10. Batch Processing Multiple Images"
echo ""
cat << 'BATCH_EOF'
   #!/bin/bash
   # Process all images in a directory

   for img in /path/to/images/*.jpg; do
       echo "Processing: $img"
       curl -X POST http://localhost:8080/api/ocr \
            -F "image=@$img" \
            -o "result_$(basename $img .jpg).json"
       sleep 0.1  # Rate limiting
   done
BATCH_EOF
echo ""
echo "---"
echo ""

echo "=== Load Testing Example ==="
echo ""
echo "Using Apache Bench:"
echo "  ab -n 100 -c 10 -p test.jpg -T 'multipart/form-data' \\"
echo "     http://localhost:8080/api/ocr"
echo ""
echo "Using wrk:"
echo "  wrk -t4 -c100 -d30s http://localhost:8080/api/health"
echo ""

echo "=== Docker Deployment ==="
echo ""
echo "Build image:"
echo "  docker build -t paddleocr-server ."
echo ""
echo "Run container:"
echo "  docker run -d -p 8080:8080 \\"
echo "    -v /path/to/models:/models \\"
echo "    paddleocr-server"
echo ""
echo "Test:"
echo "  curl http://localhost:8080/api/health"
echo ""
