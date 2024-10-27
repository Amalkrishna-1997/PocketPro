import serial
import time
import psutil
import itertools
import sys
import random
from colorama import Fore, Style, init

# Initialize colorama for Windows compatibility with colors
init(autoreset=True)

# Set up serial connection
def setup_serial(port='COM5', baudrate=9600):
    try:
        ser = serial.Serial(port, baudrate)
        time.sleep(2)
        print(f"{Fore.CYAN}Connected to Arduino on {port}")
        return ser
    except serial.SerialException:
        print(f"{Fore.RED}Failed to connect to Arduino. Check the connection.")
        return None

# Retrieve system information
def get_system_info():
    cpu = psutil.cpu_percent(interval=1)
    ram = psutil.virtual_memory().percent
    storage = psutil.disk_usage('/').percent
    return cpu, ram, storage

# Display system info with colors
def format_system_info(cpu, ram, storage):
    cpu_color = Fore.GREEN if cpu < 50 else Fore.YELLOW if cpu < 80 else Fore.RED
    ram_color = Fore.GREEN if ram < 50 else Fore.YELLOW if ram < 80 else Fore.RED
    storage_color = Fore.GREEN if storage < 50 else Fore.YELLOW if storage < 80 else Fore.RED
    
    return (f"{cpu_color}CPU: {cpu}%{Style.RESET_ALL} "
            f"{ram_color}RAM: {ram}%{Style.RESET_ALL} "
            f"{storage_color}Storage: {storage}%{Style.RESET_ALL}")

# Spinner animation for loading screen
def spinner():
    spinner_cycle = itertools.cycle(['|', '/', '-', '\\'])
    for _ in range(5):  # Spins for a few cycles
        sys.stdout.write(f"\r{Fore.YELLOW}Loading {next(spinner_cycle)}")
        sys.stdout.flush()
        time.sleep(0.2)
    print("\r", end="")  # Clear the line after the spinner

# Send system info to Arduino
def send_to_arduino(ser, cpu, ram, storage):
    data = f"CONNECTED CPU:{cpu}% RAM:{ram}% STORAGE:{storage}%"
    ser.write((data + '\n').encode())
    print(f"{Fore.CYAN}Sent to Arduino: {data}")

# Send the current time and date to Arduino in 12-hour format
def send_time_and_date(ser):
    now = time.localtime()  # Get local time
    formatted_time = time.strftime("%I:%M %p", now)  # Format time as 12-hour
    formatted_date = time.strftime("%Y-%m-%d", now)  # Format date
    ser.write(f"TIME:{formatted_time}\n".encode())
    ser.write(f"DATE:{formatted_date}\n".encode())
    print(f"{Fore.CYAN}Sent time to Arduino: {formatted_time} and date: {formatted_date}")

# Send a short motivational quote to Arduino
def send_quote(ser):
    quotes = [
        "Keep it up!",
        "Stay strong!",
        "You got this!",
        "Never give up!",
        "Stay focused!",
        "Keep pushing!",
        "Just keep swimming!"
    ]
    quote = random.choice(quotes)
    ser.write(f"QUOTE:{quote}\n".encode())
    print(f"{Fore.CYAN}Sent to Arduino: {quote}")

# Main function to manage serial connection and data transmission
def main():
    ser = setup_serial()
    if ser is None:
        return

    last_quote_time = time.time()  # Initialize last quote time
    try:
        while True:
            # Display loading spinner animation
            spinner()
            
            # Fetch system data
            cpu, ram, storage = get_system_info()
            formatted_info = format_system_info(cpu, ram, storage)
            print(formatted_info)

            # Send data to Arduino
            send_to_arduino(ser, cpu, ram, storage)

            # Send time and date every second
            send_time_and_date(ser)

            # Send a quote every 12 seconds
            current_time = time.time()
            if current_time - last_quote_time >= 12:
                send_quote(ser)
                last_quote_time = current_time  # Update last quote time

            time.sleep(1)  # Data refresh every second

    except serial.SerialException:
        print(f"{Fore.RED}Arduino disconnected. Attempting to reconnect...")

    except KeyboardInterrupt:
        print(f"{Fore.MAGENTA}\nProgram interrupted by user.")

    finally:
        if ser and ser.is_open:
            ser.close()
        print(f"{Fore.CYAN}Program ended and resources released.")

if __name__ == "__main__":
    main()
