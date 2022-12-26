import threading

from server import tcp, udp

PORT = 9999
PERIOD = 10


def main():
    # TCP, Download, Throughput
    tcp.start_throughput(
        port=PORT,
        period=PERIOD,
        upload=False
    )
    

if __name__ == '__main__':
    main()
