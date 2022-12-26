import datetime
import socket
from typing import List

from glogger.logger import get_logger

logger = get_logger('tcp')


def start(port: int):
    """
    Starts a tcp socket on given port

    Args:
        port: socket port
    """
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(('0.0.0.0', port))
        s.listen()
        conn, addr = s.accept()
        logger.info("New connecting %s", addr)
        return conn


def start_throughput(port: int, period: int, upload: bool) -> List[int]:
    """
    Start a tcp socket server to test throughput

    Args:
        port: socket port
        period: time of test in seconds
        upload: test upload or download
    """
    conn = start(port)
    logger.info("Start tcp throughput with period: %d upload: %s", period, upload)
    now = datetime.datetime.now()
    recv_bytes = [0] * period
    while True:
        total = (datetime.datetime.now() - now).total_seconds()
        if total >= period:
            break
        if upload:
            data = conn.recv(1024)
            recv_bytes[int(total)] += len(data)
        else:
            conn.sendall(b'*'*512)
    if upload:
        logger.info("Result: %s", recv_bytes)
    conn.close()
    return recv_bytes


def start_latency(port: int, number_of_packets: int) -> List[int]:
    """
    Starts latency test

    Args:
        port: socket port
        number_of_packets: number of packets to test latency
    """
    conn = start(port)
    logger.info("Start tcp latency with %d packets", number_of_packets)
    result = []
    for i in range(number_of_packets):
        now = datetime.datetime.now()
        conn.sendall(b'$')
        conn.recv(1)
        result.append(int((datetime.datetime.now() - now).total_seconds() * 1_000))
    conn.close()
    logger.info("Result: %s", result)
    return result
