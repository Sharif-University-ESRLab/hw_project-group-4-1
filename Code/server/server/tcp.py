import datetime
import socket

from glogger.logger import get_logger

logger = get_logger('tcp')


def start(port: int):
    """
    Starts a tcp socket on given port

    Args:
        port: socket port
    """
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(('0.0.0.0', port))  # Bind to 0.0.0.0:port
        s.listen()  # Waits for new client to connect
        conn, addr = s.accept()
        logger.info("New connecting %s", addr)
        return conn


def start_throughput(port: int, period: int, upload: bool, packet_size: int) -> None:
    """
    Start a tcp socket server to test throughput

    Args:
        port: socket port
        period: time of test in seconds
        upload: test upload or download
        packet_size: size of packets to send
    """
    conn = start(port)
    logger.info("Start tcp throughput with period: %d upload: %s", period, upload)
    now = datetime.datetime.now()  # test start time
    bytes_cnt = [0] * period  # Result of number of bytes sent or received per seconds
    while True:
        total = (datetime.datetime.now() - now).total_seconds()
        if total >= period:
            break
        if upload:
            # Receive `packet_size` bytes of data
            data = conn.recv(packet_size)
            bytes_cnt[int(total)] += len(data)
        else:
            # Sends `packet_size` bytes of data
            conn.sendall(b'*' * packet_size)
            bytes_cnt[int(total)] += packet_size
    logger.info("Result: %s", bytes_cnt)
    conn.close()  # Close connection


def start_latency(port: int, number_of_packets: int) -> None:
    """
    Starts latency test

    Args:
        port: socket port
        number_of_packets: number of packets to test latency
    """
    conn = start(port)
    logger.info("Start tcp latency with %d packets", number_of_packets)
    result = []  # Result of latency tests
    for i in range(number_of_packets):
        # Sends 1 byte of data and wait for 1 byte of data to receive
        now = datetime.datetime.now()
        conn.sendall(b'$')
        conn.recv(1)
        result.append(int((datetime.datetime.now() - now).total_seconds() * 1_000))
    conn.close()  # Close connection
    logger.info("Result: %s", result)
