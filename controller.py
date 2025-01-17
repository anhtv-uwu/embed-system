import argparse
import keyboard
import serial
import time

def main():
    parser = argparse.ArgumentParser(description='Send arrow keys via UART')
    parser.add_argument('--port', required=True, 
                        help='Serial port name, e.g. COM3 or /dev/ttyUSB0')
    parser.add_argument('--baud', type=int, default=115200, 
                        help='Baudrate (mặc định: 115200)')
    args = parser.parse_args()

    # Mở cổng UART
    ser = serial.Serial(port=args.port, 
                        baudrate=args.baud, 
                        timeout=0)
    print(f"Đã mở {args.port} ở baudrate={args.baud}.")
    print("Nhấn ESC để thoát...")

    try:
        while True:
            # Thoát khi nhấn ESC
            if keyboard.is_pressed('esc'):
                print("Thoát chương trình.")
                break

            # Kiểm tra phím mũi tên
            if keyboard.is_pressed('left'):
                ser.write(b"BUTTON: 4\r") 
                print("Sent: BUTTON: 4")
                # time.sleep(0.2)

            if keyboard.is_pressed('right'):
                ser.write(b"BUTTON: 2\r") 
                print("Sent: BUTTON: 2")
                # time.sleep(0.2)

            if keyboard.is_pressed('up'):
                ser.write(b"BUTTON: 1\r")  
                print("Sent: BUTTON: 1")
                time.sleep(0.2)

            if keyboard.is_pressed('down'):
                ser.write(b"BUTTON: 3\r")  
                print("Sent: BUTTON: 3")
                # time.sleep(0.2)

            # Nghỉ một chút
            time.sleep(0.01)

    except KeyboardInterrupt:
        # Ctrl + C
        pass
    finally:
        ser.close()
        print("Đã đóng cổng serial.")

if __name__ == "__main__":
    main()
