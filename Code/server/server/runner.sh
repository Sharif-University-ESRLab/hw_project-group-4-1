# Download
python -m http.server

# Upload
python -m uploadserver
curl -X POST http://127.0.0.1:8000/upload -F 'files=@/home/smss/Downloads/108585262.jpeg'

