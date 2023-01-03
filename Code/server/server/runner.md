# TCP
```
python server/main.py -p tcp -t download
python server/main.py -p tcp -t upload
python server/main.py -p tcp -t latency
```

# UDP
```
python server/main.py -p udp -t download
python server/main.py -p udp -t upload
python server/main.py -p udp -t latency
```

# HTTP
```
python server/main.py -p http
```

# Quic
```
python server/quic/http3_server.py --certificate cert.pem --private-key cert.key
```

