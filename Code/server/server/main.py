import argparse

from glogger.logger import get_logger
from server import tcp

PORT = 9999
PERIOD = 10
NUMBER_OF_PACKETS = 10


def main():
    logger = get_logger('main')
    # Parse arguments
    parser = argparse.ArgumentParser(description="Run tests")
    parser.add_argument(
        "--protocol", "-p", type=str, help="Protocol\nValues: tcp, udp, http, quic", required=True,
    )
    parser.add_argument(
        "--test", "-t", type=str, help="Type of test\nValues: latency, download, upload", required=True,
    )
    args = parser.parse_args()

    protocol: str = args.protocol
    test: str = args.test

    logger.info("Protocol: %s, test: %s", protocol, test)

    if protocol == 'tcp':
        if test == 'download':
            # TCP, Download, Throughput
            tcp.start_throughput(
                port=PORT,
                period=PERIOD,
                upload=False
            )
        elif test == 'upload':
            # TCP, Upload, Throughput
            tcp.start_throughput(
                port=PORT,
                period=PERIOD,
                upload=True
            )
        else:
            tcp.start_latency(
                port=PORT,
                number_of_packets=NUMBER_OF_PACKETS,
            )


if __name__ == '__main__':
    main()
