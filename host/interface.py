import serial
import tkinter as tk 
import time

# ask value: 00 {channel} 07 01 00 01 00
# send metateds: 00 00 01 02 00 02 01 00
# send channel teds: 00 {channel} 01 02 00 02 03 00


# Initialize the serial connection
s = serial.Serial('COM3', timeout=2)

power = 3.3
label_text = "Channel: "

                                                                        ########################
                                                                        ###Helper functions###
                                                                        ########################


# Function to convert binary string to voltage
def bin_to_voltage(b, p):
    sum = 0
    for i in range(len(b)):
        sum += int(b[i]) * p / (2 ** (i + 1))
    return sum

def value_from_output(input_string):
    # Extract numbers and dots
    filtered_string = ''.join(char for char in input_string if char.isdigit() or char == '.')
    try:
        # Convert to float
        return float(filtered_string)
    except ValueError:
        # Handle the case where conversion fails (e.g., empty string)
        return -1


                                                                        ########################
                                                                        ###IEEE 1451 functions###
                                                                        ########################

                                                                        
# globals
lat = ""
lon = ""


def read_response(type): 
    received = s.read(3)
    
    if (len(received) < 3): 
        update_text("Try again", output)
        return
    
    if received[0] == 1: #if a success message was sent
        data = s.read(received[-1]) #read the rest of the bytes, thats the actual answer
        

        if (type==0): #text
            print(data)
            update_text(' '.join(f'{byte:02X}' for byte in data), output)    

        if (type == 1): #voltage
            binary = f'{data[0]:08b}'
            v = bin_to_voltage(binary, power)
            update_text(f"Voltage: {round(v, 4)} V", output)

        if (type==2): #location
            
            decoded_msg = data.decode('ascii').split(',')
            global lat
            lat = decoded_msg[1]
            global lon
            lon = decoded_msg[3]

            update_text(lat+' '+lon, output) 
            
        if (type==3):
             if  (led_state==1): update_text("Alarm on", output)
             elif (led_state==0): update_text("Alarm off", output)


    else:
        update_text("Invalid response received", output)


    #Write STIM confirmation in the port data entry
    hex_string = ' '.join(f'{byte:02X}' for byte in (received)) 
    port_data.insert(0,  hex_string + " | ")
    return


def update_text(content, box):
    if (type(box) == tk.Text):
        box.delete('1.0', tk.END)  # Clear previous response
        box.insert(tk.END, content)  # Add new content
    if (type(box) == tk.Entry):
        box.delete(0, tk.END)  # Clear previous response
        box.insert(tk.END, content)  # Add new content
    return



def draw_dot(v):
    # Parse latitude and longitude values
    try:
        x = float(lat.split("=")[1])  # Longitude
        y = float(lon.split("=")[1])  # Latitude
    except (IndexError, ValueError):
        print("Invalid latitude/longitude format.")
        return

    # Map -180 to 180 (longitude) and -90 to 90 (latitude) to canvas dimensions
    canvas_x = (x + 90) * canvas_width / 180
    canvas_y = canvas_height - ((y + 180) * canvas_height / 360)  # Invert Y-axis for canvas


    if (v > 3): # strong vibration, turn the alarm on
        value_request(3) #acender led
        time.sleep(2)
        value_request(3) #apagar led

    def rgb_to_hex(rgb):
        return "#{:02x}{:02x}{:02x}".format(*rgb)
    color = rgb_to_hex((int(v*255/power),int((power-v)*255/power),0))

    # Draw the dot
    mapa.create_oval(
        canvas_x - 5, canvas_y - 5,
        canvas_x + 5, canvas_y + 5,
        fill=color, outline=color
    )



                                                                        #######################
                                                                        ###Button callbacks###
                                                                        #######################


# Function to handle button click
led_state = 0
def value_request(c=0):
    if (c==0):
        text_content = channel_select.get()
        c=int(text_content[len(label_text):])

    message = bytes([0, c, 7, 1, 0, 1, 0])
    
    if (c==3):
        global led_state
        led_state = 0 if (led_state==1) else 1
        message = bytes([0,c,7,2,0,2,0,led_state])
        

    hex_string = ' '.join(f'{byte:02X}' for byte in message)
    update_text(hex_string, port_data)

    s.write(message)
    read_response(c)
    return


def ted_request():#00 {channel} 01 02 00 02 03 00
    text_content = channel_select.get()
    channel=int(text_content[len(label_text):])

    message = bytes([0,channel,1,2,0,2,3,0])

    hex_string = ' '.join(f'{byte:02X}' for byte in message)
    update_text(hex_string, port_data)

    s.write(message)
    read_response(0)
    return


def metated_request(): 
    message = bytes([0,0,1,2,0,2,1,0])

    hex_string = ' '.join(f'{byte:02X}' for byte in message)
    update_text(hex_string, port_data)

    s.write(message)
    read_response(0)
    return


def map_build(): #the map is only visible after the cicle completes
    n=50
    port_data.delete(0, tk.END)  # Clear previous response

    for i in range(n):
        value_request(1) #request vibration sensor
        v = value_from_output(output.get("1.0", tk.END).strip())
        print("Vibration Sensor: ",v)

        value_request(2) #request gps
        draw_dot(v)
        

        root.update()
    
    
    return

                                                ########################
                                                #######Interface########
                                                ########################

# Create the GUI
root = tk.Tk()
root.title("NCAP")

# Window styling
window_width = 900  # Width of the window
window_height = 600  # Height of the window
screen_width = root.winfo_screenwidth()
screen_height = root.winfo_screenheight()


# Calculate the x and y coordinates to center the window
x = (screen_width // 2) - (window_width // 2)
y = (screen_height // 2) - (window_height // 2)


# Set the geometry of the window with the calculated position
root.geometry(f"{window_width}x{window_height}+{x}+{y}")
root.configure(bg="#2B2D42")


# Grid Setup
canvas_weight = 4
root.columnconfigure(0, weight=1)
root.columnconfigure(1, weight=1)
root.columnconfigure(2, weight=1)
root.columnconfigure(3, weight=canvas_weight)

root.rowconfigure(0, weight=1)
root.rowconfigure(1, weight=1)
root.rowconfigure(2, weight=6)
root.rowconfigure(3, weight=20)
root.rowconfigure(4, weight=6)
total_weight = sum(root.grid_columnconfigure(i)['weight'] for i in range(root.grid_size()[0]))





def update_canvas_size(event):
    # Get the actual size of the canvas after the window is rendered
    global canvas_width, canvas_height
    canvas_width = event.width  
    canvas_height = event.height

    # Draw grid lines and labels for the -180 to 180 (longitude) and -90 to 90 (latitude) range
    mapa.delete("all")  # Clear previous drawings
    for i in range(-90, 91, 30):  # Longitude grid lines every 30 degrees
        x = (i + 90) * canvas_width / 180
        mapa.create_line(x, 0, x, canvas_height, fill="lightgray")
        if i != 0:  # Avoid overlapping label at the origin
            mapa.create_text(x, canvas_height / 2 + 10, text=str(i), font=("Helvetica", 8), fill="black")  # X-axis labels

    for i in range(-180, 181, 30):  # Latitude grid lines every 30 degrees
        y = canvas_height - ((i + 180) * canvas_height / 360)  # Invert Y-axis for canvas
        mapa.create_line(0, y, canvas_width, y, fill="lightgray")
        if i != 0:
            mapa.create_text(canvas_width / 2 - 10, y, text=str(i), font=("Helvetica", 8), fill="black")  # Y-axis labels

    # Draw the origin lines
    mapa.create_line(canvas_width / 2, 0, canvas_width / 2, canvas_height, fill="black", width=2)  # Y-axis
    mapa.create_line(0, canvas_height / 2, canvas_width, canvas_height / 2, fill="black", width=2)  # X-axis



def validate_input(event):
    if channel_select.index(tk.INSERT) == len(label_text) and event.keysym in ["BackSpace"]:
        return "break"
    if channel_select.index(tk.INSERT) < len(label_text) :
        return "break"
    # Only allow digits, backspace, or delete
    if event.char.isdigit() or event.char == " " or event.keysym in ["BackSpace", "Delete"]:
        return  # Accept the input
    else:
        return "break"  # Prevent the input



mapa = tk.Canvas(
    root, bg="white", 
    width=1, 
    height=1
)
mapa.bind("<Configure>", update_canvas_size)

port_data = tk.Entry(
    root,
    font=("Helvetica", 14),
    bd=3,
    width=1,
)

channel_select = tk.Entry(
    root, 
    font=("Helvetica", 14),
    width=1,
    bd=3
)
channel_select.insert(0,label_text)
channel_select.bind("<Key>", validate_input) #channel select only accepts numbers

button_value = tk.Button(
    root,
    text="Read/Write",
    command=value_request,
    font=("Arial", 16, "bold"),
    bg="#8D99AE",
    fg="#2B2D42",
    activebackground="#EF233C",
    activeforeground="#FFFFFF",
    relief="raised",
    bd=3,
    width=1,
    height=1
)

button_TEDS = tk.Button(
    root,
    text="TEDS",
    command=ted_request,
    font=("Arial", 16, "bold"),
    bg="#8D99AE",
    fg="#2B2D42",
    activebackground="#EF233C",
    activeforeground="#FFFFFF",
    relief="raised",
    bd=3,
    width=1,
    height=1
)

button_MetaTEDS = tk.Button(
    root,
    text="MetaTEDS",
    command=metated_request,
    font=("Arial", 16, "bold"),
    bg="#8D99AE",
    fg="#2B2D42",
    activebackground="#EF233C",
    activeforeground="#FFFFFF",
    relief="raised",
    bd=3,
    width=1,
    height=1
)

output = tk.Text(
    font=("Helvetica", 14),
    bg="#EDF2F4",
    fg="#2B2D42",
    width=1,
    height=1,
    bd=3
)

map_controler = tk.Button(
    root,
    text="Build Map",
    command=map_build,
    font=("Arial", 16, "bold"),
    bg="#8D99AE",
    fg="#2B2D42",
    activebackground="#EF233C",
    activeforeground="#FFFFFF",
    relief="raised",
    bd=3,
    width=1,
    height=1
)



mapa.grid(column=3, row=1, rowspan=4, padx=10, pady=(0,10), sticky=tk.NSEW)
port_data.grid(column=0, row=0, columnspan=4, padx=10, pady=10, sticky=tk.NSEW)
channel_select.grid(column=0, row=1, columnspan=3, padx=10, sticky=tk.NSEW)
button_value.grid(column=0,row=2, padx=10, pady=10, sticky=tk.NSEW)
button_TEDS.grid(column=1,row=2, pady=10, sticky=tk.NSEW)
button_MetaTEDS.grid(column=2,row=2, padx=10, pady=10, sticky=tk.NSEW)
output.grid(column=0, row=3, columnspan=3, padx=10, pady=5, sticky=tk.NSEW)
map_controler.grid(column=0,row=4, columnspan=3, padx=10, pady=10, sticky=tk.NSEW)

# Run the GUI event loop
root.mainloop()

