import asyncio
import serial_asyncio


class Output(asyncio.Protocol):
    def connection_made(self, transport):
        self.transport = transport
        print('port opened', transport)
        transport.serial.rts = False  # You can manipulate Serial object via transport
        transport.write(b'Hello, World!\n')  # Write serial data via transport

    def data_received(self, data):
        print('data received', repr(data))

        # if b'\n' in data:
        #     self.transport.close()

    def connection_lost(self, exc):
        print('port closed')
        self.transport.loop.stop()

if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    coro = serial_asyncio.create_serial_connection(loop, Output, '/dev/tty.SLAB_USBtoUART', baudrate=115200)
    print(dir(coro))
    # loop.write(b'h')
    loop.run_until_complete(coro)
    loop.run_forever()
    loop.close()
