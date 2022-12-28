# TCP
python server/main.py -p tcp -t download
python server/main.py -p tcp -t upload
python server/main.py -p tcp -t latency

# UDP
python server/main.py -p udp -t download
python server/main.py -p udp -t upload
python server/main.py -p udp -t latency

# HTTP
python server/main.py -p http

# Quic
python server/quic/http3_server.py --certificate cert.pem --private-key cert.key

# Download
python -m http.server

# Upload
python -m uploadserver
curl -X POST http://127.0.0.1:8000/upload -F 'files=@/home/smss/Downloads/108585262.jpeg'

python server/quic/http3_client.py --ca-certs cert.pem https://localhost:4433/10000000