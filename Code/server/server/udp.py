import datetime
import socket
from typing import Tuple

from glogger.logger import get_logger

logger = get_logger("udp")


def start(port: int) -> Tuple[socket.socket, Tuple[str, int]]:
    """
    Starts a udp socket on given port

    Args:
        port: socket port
    """
    logger.info("Waiting for udp socket")
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind(("0.0.0.0", port))  # Bind to 0.0.0.0:port
    # Waits for new client to connect and send 1 byte of data
    while True:
        data, addr = s.recvfrom(1)
        if data is not None:
            break
    logger.info("Found udp socket")
    return s, (addr[0], 10210)


def start_throughput(port: int, period: int, upload: bool, packet_size: int) -> None:
    """
    Start an udp socket server to test throughput

    Args:
        port: socket port
        period: time of test in seconds
        upload: test upload or download
        packet_size: size of packets
    """

    conn, addr = start(port)
    logger.info("Start udp throughput with period: %d, addr: %s", period, addr)
    now = datetime.datetime.now()  # test start time
    bytes_cnt = [0] * period  # Result of number of bytes sent or received per seconds
    while True:
        total = (datetime.datetime.now() - now).total_seconds()
        if total >= period:
            break
        if upload:
            # Receive `packet_size` bytes of data
            data, _ = conn.recvfrom(packet_size)
            bytes_cnt[int(total)] += len(data)
        else:
            # Sends `packet_size` bytes of data
            conn.sendto(b"*" * packet_size, addr)
            bytes_cnt[int(total)] += packet_size
    logger.info("Result: %s", bytes_cnt)
    conn.close()  # Close connection


def start_latency(port: int, number_of_packets: int) -> List[int]:
    """
    Starts latency test

    Args:
        port: socket port
        number_of_packets: number of packets to test latency
    """
    conn, addr = start(port)
    logger.info("Start udp latency with %d packets", number_of_packets)
    result = []  # Result of latency tests
    for i in range(number_of_packets):
        # Sends 1 byte of data and wait for 1 byte of data to receive
        now = datetime.datetime.now()
        conn.sendto(b"$", addr)
        conn.recv(1)
        result.append(int((datetime.datetime.now() - now).total_seconds() * 1_000))
    conn.close()  # Close connection
    logger.info("Result: %s", result)
