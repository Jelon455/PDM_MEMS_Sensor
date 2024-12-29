import serial
import csv
import time

try:
    #init serial
    ser = serial.Serial('COM3', baudrate=115200, timeout=1)  #adjust COM
    print("Connected with device.")

    with open('C:/Users/Julia/Desktop/Studia/Magisterka/PDM_MEMS_Sensor/Python/dane.csv', mode='w', newline='', encoding='utf-8') as file:
        writer = csv.writer(file, delimiter=';', quotechar='"', quoting=csv.QUOTE_MINIMAL)
        writer.writerow(['Sample Number', 'Time (ms)', 'X (g)', 'Y (g)', 'Z (g)'])

        while True:
            try:
                #data from ESP
                line = ser.readline().decode('utf-8').strip()
                if line:
                    values = line.split(',')
                    if len(values) == 5:
                        try:
                            sample_number = int(values[0])
                            current_time = float(values[1])
                            x = float(values[2])
                            y = float(values[3])
                            z = float(values[4])
                            writer.writerow([sample_number, current_time, x, y, z])
                            print(f"{sample_number}; {current_time}; {x}; {y}; {z}")
                        except ValueError:
                            print(f"Wrong data: {line}")
                    else:
                        print(f"Wrong data lenght: {line}")

            except KeyboardInterrupt:
                print("End of the data collection.")
                break

except serial.SerialException as e:
    print(f"Serial communication error: {e}")
except Exception as e:
    print(f"Error: {e}")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
    print("End of program.")
