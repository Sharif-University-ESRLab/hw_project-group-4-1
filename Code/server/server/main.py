import argparse
import os
import signal

import udp
from glogger.logger import get_logger
from server import tcp
import uploadserver

PORT = 9999
PERIOD = 10
NUMBER_OF_PACKETS = 10
FILE_SIZE = 512
FILE_NAME = 'dummyFile'
EMPTY_FILE_NAME = 'emptyDummyFile'


def main():
    logger = get_logger('main')
    # Parse arguments
    parser = argparse.ArgumentParser(description="Run tests")
    parser.add_argument(
        "--protocol", "-p", type=str, help="Protocol\nValues: tcp, udp, http, quic", required=True,
    )
    parser.add_argument(
        "--test", "-t", type=str, help="Type of test\nValues: latency, download, upload", required=False,
    )

    args = parser.parse_args()

    protocol: str = args.protocol
    test: str = args.test

    logger.info("Protocol: %s, test: %s, port: %d", protocol, test, PORT)

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
    elif protocol == 'udp':
        if test == 'download':
            # UDP, Download, Throughput
            udp.start_throughput(
                port=PORT,
                period=PERIOD,
                upload=False
            )
        elif test == 'upload':
            # UDP, Upload, Throughput
            udp.start_throughput(
                port=PORT,
                period=PERIOD,
                upload=True
            )
        else:
            # UDP, Latency
            udp.start_latency(
                port=PORT,
                number_of_packets=NUMBER_OF_PACKETS,
            )
    elif protocol == 'http':
        import sys
        sys.argv = sys.argv[:1]
        sys.argv.append(str(PORT))
        # Creating a huge file to test download
        os.system(f'dd if=/dev/zero of={FILE_NAME} bs={FILE_SIZE}MB count=1')
        # Creating an empty file to test latency
        os.system(f'touch {EMPTY_FILE_NAME}')

        # Clean dummy file
        def sigint_handler(_, __):
            logger.info("Cleaning dummy file")
            os.system(f'rm -f {FILE_NAME} {EMPTY_FILE_NAME}')
            sys.exit(0)

        signal.signal(signal.SIGINT, sigint_handler)
        uploadserver.main()


if __name__ == '__main__':
    main()
