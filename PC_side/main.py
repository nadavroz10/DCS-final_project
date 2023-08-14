import tkinter as tk
import serial as ser
import time
import numpy as np
import matplotlib

matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
import keyboard
from matplotlib.widgets import Button
from tkinter.ttk import *
from matplotlib.backends.backend_tkagg import (
    FigureCanvasTkAgg, NavigationToolbar2Tk)


def object_detector_gui(s, script_mode=False, mask_distance=450):
    plt.close("all")
    # ----------------------figure 1------------------------------
    fig = plt.figure(facecolor='k')
    fig.canvas.toolbar.pack_forget()
    fig.canvas.manager.set_window_title("Radar")
    mgn = plt.get_current_fig_manager()
    mgn.window.state('zoomed')
    ax = fig.add_subplot(1, 1, 1, polar=True, facecolor='#006b70')
    ax.tick_params(axis='both', colors='w')
    r_max = mask_distance
    ax.set_ylim([0.0, r_max])
    ax.set_xlim([0.0, np.pi])
    ax.set_position([-0.05, -0.05, 1.1, 1.05])  ## What
    ax.set_rticks(np.linspace(0.0, r_max, 5))
    ax.set_thetagrids(np.linspace(0.0, 180, 37))
    angles = np.arange(0, 181, 1)
    theta = angles * (np.pi / 180)
    pols, = ax.plot([], linestyle='', marker='o', markerfacecolor='r',
                    markeredgecolor='w', markeredgewidth=1.0, markersize=6.0, alpha=0.5)

    pols2, = ax.plot([], linestyle='', marker='o', markerfacecolor='b',
                     markeredgecolor='w', markeredgewidth=1.0, markersize=12.0, alpha=0.5)

    line1 = ax.plot([], color='w', linewidth=3.0)
    fig.canvas.draw()
    axbackground = fig.canvas.copy_from_bbox(ax.bbox)
    #########################################
    # --------------------------------------------For Gui---------------------
    if not script_mode:
        s.reset_input_buffer()
        s.reset_output_buffer()
    fig.canvas.flush_events()
    # dists = np.ones((len(angles)))
    dists = np.full(len(angles), 500, dtype=int)
    dists2 = np.full(len(angles), 500, dtype=int)
    # ----------------------------------set distance to send -----------------------------------------------------------
    factor_distance = 62  # 1/ [(1/2^20) *17000]
    factor_angle = 9
    dis = mask_distance  # dist is mask distance
    raw_dis = dis * factor_distance  # 1/ [(1/2^20) *17000]
    raw_dis_low = raw_dis % 256
    raw_dis_high = raw_dis / 256
    # ------------------------------------------------------------------------
    if not script_mode:
        num_option = '1'
        send_byte(ord(num_option), s)  # move to state1

        # ----------------------------------send mask distance---------------------------------------------------------
        send_byte(int(raw_dis_high), s)
        time.sleep(1)  # Sleep for 3 seconds
        send_byte(int(raw_dis_low), s)
    # ------------------------------------------------------------------------------------------------------------------
    # print(raw_dis_low)
    # print(raw_dis_high)
    # ----------------------------------get the values of the scan------------------------------------------------------
    angles_list = []
    distances_list = []
    receiving_index = 0
    receiving_data = 0

    while not (receiving_data == 255 and receiving_index % 4 == 1):
        if s.in_waiting > 0:
            receiving_data = int.from_bytes(s.read(size=1), "big")  # received MSB byte of distance
            if receiving_index % 4 == 0:
                # print(receiving_data, receiving_index, "msb")
                distances_list.append((1 / factor_distance) * receiving_data * 256)
            elif receiving_index % 4 == 1:
                # print(receiving_data, receiving_index, "lsb")
                distances_list[len(distances_list) - 1] += (
                        receiving_data * (1 / factor_distance))  # add LSB byte to current received distance
            elif receiving_index % 4 == 2:
                # print(receiving_data, receiving_index, "angle msb")
                angles_list.append(receiving_data * 256)  # received MSB byte of angle
            else:
                # print(receiving_data, receiving_index, "angle lsb")
                angles_list[len(angles_list) - 1] += receiving_data  # add LSB byte to current received angle
                angles_list[len(angles_list) - 1] = (angles_list[len(angles_list) - 1] - 629) / factor_angle
                # print("distance:", distances_list[len(distances_list) - 1], "Angle", angles_list[len(angles_list) -
                # 1] ) ----for GUI
                dists[int(angles_list[len(angles_list) - 1])] = distances_list[len(distances_list) - 1]
                line1[0].set_data(np.repeat((angles_list[len(angles_list) - 1] * (np.pi / 180)), 2),
                                  np.linspace(0.0, r_max, 2))
                pols.set_data(theta, dists)
                fig.canvas.restore_region(axbackground)
                ax.draw_artist(pols)
                ax.draw_artist(line1[0])
                fig.canvas.blit(ax.bbox)
                # -----

            receiving_index += 1

    # ------------------------------------------------------------------------------------------------------------------
    print(distances_list, "distances_list")
    print(angles_list, "angles_list")
    # print (len(distances_list), len(angles_list))
    accepted_dots = [(distances_list[i], angles_list[i]) for i in range(0, len(angles_list))]
    print(accepted_dots)
    avg_lists = [[accepted_dots[0]]]
    for i in range(1, len(accepted_dots)):
        last_list = avg_lists[len(avg_lists) - 1]
        if abs(accepted_dots[i][0] - last_list[len(last_list) - 1][0]) < 6 and abs(
                accepted_dots[i][1] - last_list[len(last_list) - 1][1]) < 6:
            last_list.append(accepted_dots[i])
        #   print("same: ", accepted_dots[i][0])
        else:
            #  print("new: ", accepted_dots[i][0])
            avg_lists.append([accepted_dots[i]])

    def avg_list_0(l):
        return int(sum([arg[0] for arg in l]) // len(l))

    def avg_list_1(l):
        return int(sum([arg[1] for arg in l]) // len(l))

    distance_avg = list(map(avg_list_0, avg_lists))
    angle_avg = list(map(avg_list_1, avg_lists))
    # print("distances",distance_avg)
    # print ("angles", angle_avg)
    # print ("shape", dists2.shape)
    for i in range(len(angle_avg)):
        dists2[angle_avg[i]] = distance_avg[i]

    pols2.set_data(theta, dists2)
    ax.draw_artist(pols2)
    fig.canvas.blit(ax.bbox)
    print("list of lists:")
    print(avg_lists)


def telemeter_function(s, script_mode, degree_in):
    plt.close("all")
    # ----------------------figure 1 ---------------------------
    fig = plt.figure(facecolor='k')
    fig.canvas.toolbar.pack_forget()
    fig.canvas.manager.set_window_title("Radar")
    mgn = plt.get_current_fig_manager()
    mgn.window.state('zoomed')
    ax = fig.add_subplot(1, 1, 1, polar=True, facecolor='#006b70')
    ax.tick_params(axis='both', colors='w')
    r_max = 450
    ax.set_ylim([0.0, r_max])
    ax.set_xlim([0.0, np.pi])
    ax.set_position([-0.05, -0.05, 1.1, 1.05])  ## What
    ax.set_rticks(np.linspace(0.0, r_max, 5))
    ax.set_thetagrids(np.linspace(0.0, 180, 37))
    angles = np.arange(0, 181, 1)
    theta = angles * (np.pi / 180)
    pols, = ax.plot([], linestyle='', marker='o', markerfacecolor='r',
                    markeredgecolor='w', markeredgewidth=1.0, markersize=8.0, alpha=0.5)

    line1 = ax.plot([], color='w', linewidth=1.0)
    fig.canvas.draw()
    axbackground = fig.canvas.copy_from_bbox(ax.bbox)
    #########################################
    # --------------------------------------------For Gui---------------------
    s.reset_input_buffer()
    s.reset_output_buffer()
    fig.canvas.flush_events()
    dists = np.full(len(angles), 500, dtype=int)
    # ----------------------------------------get real time distace values----------------------------------------------
    factor_distance = 62  # 1/ [(1/2^20) *17000]
    distances_list = []
    receiving_index = 0
    script_receive_counter = 0
    # -----------------------------------
    if script_mode == 0:
        num_option = '2'
        send_byte(ord(num_option), s)  # move to state1
        time.sleep(0.1)
        # ----------------------------------------set angle to send
        # --------------------------------------------------------
        angle = degree_in
        factor_angle = 9
        raw_angle = 629 + factor_angle * angle  # 1/ [(1/2^20) *17000]
        raw_angle_low = raw_angle % 256
        raw_angle_high = raw_angle / 256
        # ----------------------------------------send fixed
        # angle----------------------------------------------------------
        send_byte(int(raw_angle_high), s)
        time.sleep(1)  # Sleep for 1 seconds
        send_byte(int(raw_angle_low), s)

    else:  # receive angle of script mode
        counter_angle = 0
        while counter_angle < 2:
            if s.in_waiting > 0:
                receiving_data = int.from_bytes(s.read(size=1), "big")
                if counter_angle == 0:
                    angle = receiving_data * 256
                elif counter_angle == 1:
                    angle += receiving_data
                    angle = (angle - 629) / 11
                # print("angle", angle)

                counter_angle += 1

    while True:
        if s.in_waiting > 0:
            receiving_data = int.from_bytes(s.read(size=1), "big")  # received MSB byte of distance
            if receiving_index % 2 == 0:
                distances_list.append((1 / factor_distance) * receiving_data * 256)
            elif receiving_index % 2 == 1:
                distances_list[len(distances_list) - 1] += (receiving_data * (1 / factor_distance))  # add LSB byte
                # to current received distance
                print(distances_list[len(distances_list) - 1])
            receiving_index += 1

            dists[int(angle)] = distances_list[len(distances_list) - 1]
            line1[0].set_data(np.repeat((angle * (np.pi / 180)), 2),
                              np.linspace(0.0, r_max, 2))
            pols.set_data(theta, dists)
            fig.canvas.restore_region(axbackground)
            ax.draw_artist(pols)
            ax.draw_artist(line1[0])
            fig.canvas.blit(ax.bbox)
            fig.canvas.flush_events()

            if keyboard.is_pressed('e') or script_receive_counter == 20:
                print("Exit Telemeter")

                fig.canvas.flush_events()
                send_byte(ord('0'), s)  # go to state0
                break

            if script_mode == 1:  # if in script mode
                script_receive_counter += 1


def get_init_arrays(s):
    ldr1_list = []
    ldr2_list = []
    i = 0
    j = 0
    while j < 1:
        if s.in_waiting > 0:
            r = int.from_bytes(s.read(size=1), "big")  # received MSB byte of distance
            j += 1
    while i < 40:  # config environment receiving
        if s.in_waiting > 0:
            receiving_data = int.from_bytes(s.read(size=1), "big")  # received MSB byte of distance
            if i % 4 == 0:
                #   print(receiving_data, i, "ldr1 config lsb")
                ldr1_list.append(receiving_data)
            elif i % 4 == 1:
                #  print(receiving_data, i, "ldr1 config msb")
                ldr1_list[len(ldr1_list) - 1] += receiving_data * 256  # add LSB byte to current received distance
            elif i % 4 == 2:
                # print(receiving_data, i, "ldr2  config lsb")
                ldr2_list.append(receiving_data)  # received MSB byte of angle
            else:
                # print(receiving_data, i, "ldr2 config msb")
                ldr2_list[len(ldr2_list) - 1] += receiving_data * 256  # add LSB byte to current received angle
            i += 1
    print(ldr1_list, "ldr1_list")
    print(ldr2_list, "ldr2_list")
    ldr1_init_arr = []
    ldr2_init_arr = []
    for i in range(9):
        space1 = (ldr1_list[i + 1] - ldr1_list[i]) / 5
        space2 = (ldr2_list[i + 1] - ldr2_list[i]) / 5
        for j in range(5):
            ldr1_init_arr.append(ldr1_list[i] + j * space1)
            ldr2_init_arr.append(ldr2_list[i] + j * space2)

    #  print(len(ldr1_init_arr))
    # print(len(ldr2_init_arr))
    print(ldr1_init_arr, "ldr1_init_arr", ldr2_init_arr, "ldr2_init_arr")
    return [(ldr1_init_arr[i] + ldr2_init_arr[i]) * 0.5 for i in range(45)]


def light_sources_detector(s):
    plt.close("all")
    # ----------------figure 2-----------------------
    fig2 = plt.figure(facecolor='k')
    fig2.canvas.toolbar.pack_forget()
    fig2.canvas.manager.set_window_title("Radar")
    mgn = plt.get_current_fig_manager()
    mgn.window.state('zoomed')
    ax2 = fig2.add_subplot(1, 1, 1, polar=True, facecolor='#006b70')
    ax2.tick_params(axis='both', colors='w')
    r_max2 = 45
    ax2.set_ylim([0.0, r_max2])
    ax2.set_xlim([0.0, np.pi])
    ax2.set_position([-0.05, -0.05, 1.1, 1.05])  ## What
    ax2.set_rticks(np.linspace(0.0, r_max2, 5))
    ax2.set_thetagrids(np.linspace(0.0, 180, 37))
    pols2, = ax2.plot([], linestyle='', marker='o', markerfacecolor='y',
                      markeredgecolor='w', markeredgewidth=1.0, markersize=6.0, alpha=0.5)

    line2 = ax2.plot([], color='w', linewidth=3.0)
    fig2.canvas.draw()

    axbackground = fig2.canvas.copy_from_bbox(ax2.bbox)
    # ------------------------------------------------------------------------
    s.reset_input_buffer()
    # --------------------------------------------For Gui---------------------
    fig2.canvas.flush_events()
    angles = np.arange(0, 181, 1)
    theta = angles * (np.pi / 180)
    dists = np.ones((len(angles)))
    # -----------------------------------
    num_option = '3'
    send_byte(ord(num_option), s)  # move to state1
    init_array = get_init_arrays(s)
    ldr1 = []
    ldr2 = []
    samples_list = []
    angles_list = []
    distances = []
    factor_angle = 9
    receiving_index = 0
    receiving_data = 0
    while not (receiving_data == 255 and receiving_index % 6 == 1):
        if s.in_waiting > 0:
            receiving_data = int.from_bytes(s.read(size=1), "big")  # received MSB byte of distance
            if receiving_index % 6 == 0:
                # print(receiving_data, receiving_index, "ldr1 msb data")
                ldr1.append(receiving_data * 256)
            elif receiving_index % 6 == 1:
                # print(receiving_data, receiving_index, "ldr1 lsb data")
                ldr1[len(ldr1) - 1] += (receiving_data)  # add LSB byte to current received distance
            elif receiving_index % 6 == 2:
                # print(receiving_data, receiving_index, "ldr2 msb data")
                ldr2.append(receiving_data * 256)  # received MSB byte of angle

            elif receiving_index % 6 == 3:
                # print(receiving_data, receiving_index, "ldr2 lsb data")
                ldr2[len(ldr2) - 1] += receiving_data  # add LSB byte to current received angle
                # print("ldr1:", ldr1[len(ldr1) - 1], "ldr2", ldr2[len(ldr2) - 1])
                samp = (ldr1[len(ldr1) - 1] + ldr2[len(ldr2) - 1]) / 2
                samples_list.append(samp)
                dist_index = np.array([abs((init_array[i] - samp)) for i in range(45)]).argmin()
                # print("min: ", min([(init_array[i] - samp) for i in range(45)]))
                #  print("min index: ", dist_index)
                distance = (dist_index + 1)
                distances.append(distance)

            elif receiving_index % 6 == 4:
                # print(receiving_data, receiving_index, "angle msb")
                angles_list.append(receiving_data * 256)  # received MSB byte of angle
            else:
                # print(receiving_data, receiving_index, "angle lsb")
                angles_list[len(angles_list) - 1] += receiving_data  # add LSB byte to current received angle
                angles_list[len(angles_list) - 1] = (angles_list[len(angles_list) - 1] - 629) / factor_angle
                print(angles_list[len(angles_list) - 1], " angles_list[len(angles_list) - 1]")
                # ----for GUI-------
                dists[int(angles_list[len(angles_list) - 1])] = distances[len(distances) - 1]
                line2[0].set_data(np.repeat((angles_list[len(angles_list) - 1] * (np.pi / 180)), 2),
                                  np.linspace(0.0, r_max2, 2))
                pols2.set_data(theta, dists)
                fig2.canvas.restore_region(axbackground)
                ax2.draw_artist(pols2)
                ax2.draw_artist(line2[0])
                fig2.canvas.blit(ax2.bbox)
                # ---------

            receiving_index += 1

    print(distances, "distances")
    print(angles_list, "angles_list")
    print(init_array, "init_array")
    print([(distances[i], angles_list[i]) for i in range(0, len(angles_list))])


def send_byte(byte_data, com):
    bytes_char = bytes([byte_data])
    com.write(bytes_char)


def translate_files(file_name):
    file = open(file_name, 'r')
    lines = file.readlines()
    instructions = []
    inst_translate = {
        "inc_lcd": "01",
        "dec_lcd": "02",
        "rra_lcd": "03",
        "set_delay": "04",
        "clear_lcd": "05",
        "servo_deg": "06",
        "servo_scan": "07",
        "sleep": "08"
    }

    for line in lines:
        x = line.split()
        instruction_arr = [inst_translate[x[0]]]
        if len(x) > 1:
            args = x[1].split(",")
            for arg in args:
                hex_val_int = int(arg)
                if hex_val_int < 16:
                    hex_val = "0" + hex(hex_val_int)[2:]  # [2:] to ignore 0x
                else:
                    hex_val = hex(hex_val_int)[2:]
                instruction_arr.append(hex_val)
        instruction = "".join(instruction_arr)
        instructions.append(instruction)
    return instructions


def send_file(com, file_name):
    # sets the title of the
    # Toplevel widget

    file_arr = translate_files(file_name)
    print(file_arr, "file_arr")
    for inst in file_arr:
        for i in range(0, len(inst) - 1, 2):
            command = int(inst[i:i + 2], 16)
            send_byte(command, com)
            time.sleep(0.1)
            # print(len(inst))
        for i in range(len(inst) // 2, 4):  # complete to four bytes
            send_byte(254, com)
            time.sleep(0.1)

    send_byte(255, com)  # EOF msb
    time.sleep(0.1)
    send_byte(255, com)  # EOF lsb
    time.sleep(0.1)
    receiving_data = 0
    while not (receiving_data == 255):
        if s.in_waiting > 0:
            receiving_data = int.from_bytes(s.read(size=1), "big")
            print(receiving_data, "receiving_data")
    print("done")


"""
def send_all_files(s):
    num_option = '4'
    send_byte(ord(num_option), s)  # move to state4
    send_file(s,"Script1.txt")
    send_file(s, "Script2.txt")
    send_file(s, "Script3.txt")

"""


def send_file_option(com, script_num):
    num_option = '4'
    send_byte(ord(num_option), com)  # move to state4
    time.sleep(0.1)
    send_byte(script_num, com)  # send script number
    time.sleep(0.1)
    if script_num == 1:
        send_file(com, "Script1.txt")
    elif script_num == 2:
        send_file(com, "Script2.txt")
    else:
        send_file(com, "Script3.txt")


def do_script(s, script_num):
    plt.close("all")
    num_option = '6'
    send_byte(ord(num_option), s)  # move to state4
    time.sleep(0.1)
    send_byte(script_num, s)  # send script number
    time.sleep(0.1)
    receiving_data = 0
    while not (receiving_data == 255):
        if s.in_waiting > 0:
            receiving_data = int.from_bytes(s.read(size=1), "big")
            if receiving_data == 7:
                object_detector_gui(s, True)

            ###############COMUNICATION#####################################


s = ser.Serial('COM1', baudrate=9600, bytesize=ser.EIGHTBITS,
               parity=ser.PARITY_NONE, stopbits=ser.STOPBITS_ONE,
               timeout=1)  # timeout of 1 sec where the read and write operations are blocking,
# after the timeout the program continues
# enableTX = True
# clear buffers
s.reset_input_buffer()
s.reset_output_buffer()
###############################################################


root = tk.Tk()  # create parent window


def open_script_window(com):
    # Toplevel object which will
    # be treated as a new window

    new_window = tk.Toplevel(root)

    # sets the title of the
    # Toplevel widget
    new_window.title("script")
    new_window.attributes("-topmost", True)
    new_window.geometry('+700+100')
    # sets the geometry of toplevel
    # newWindow.geometry("200x200")

    # A Label widget to show in toplevel

    do1_script = tk.Button(new_window, text="Do Script 1", width=25, command=lambda: do_script(com, 1),
                           font=("Courier New", 10), bg="#aad7f2")
    do1_script.pack()

    do2_script = tk.Button(new_window, text="Do Script 2", width=25, command=lambda: do_script(com, 2),
                           font=("Courier New", 10), bg="#aad7f2")
    do2_script.pack()

    do3_script = tk.Button(new_window, text="Do Script 3", width=25, command=lambda: do_script(com, 3),
                           font=("Courier New", 10), bg="#aad7f2")
    do3_script.pack()

    close = tk.Button(new_window, text="close", width=25, command=new_window.destroy,
                      font=("Courier New", 10), bg="#aad7f2")
    # Script_M.pack()
    close.pack()


def open_send_window(com):
    new_window = tk.Toplevel(root)

    # sets the title of the
    # Toplevel widget
    new_window.title("Send Script")
    new_window.geometry('+700+100')
    # sets the geometry of toplevel
    new_window.attributes("-topmost", True)
    script_m1 = tk.Button(new_window, text="Send File 1", width=25, command=lambda: send_file_option(com, 1),
                          font=("Courier New", 10), bg="#aad7f2")
    # Script_M.pack()
    script_m1.pack()
    script_m2 = tk.Button(new_window, text="Send File 2", width=25, command=lambda: send_file_option(com, 2),
                          font=("Courier New", 10), bg="#aad7f2")
    # Script_M.pack()
    script_m2.pack()
    script_m3 = tk.Button(new_window, text="Send File 3", width=25, command=lambda: send_file_option(com, 3),
                          font=("Courier New", 10), bg="#aad7f2")
    # Script_M.pack()
    script_m3.pack()

    close = tk.Button(new_window, text="close", width=25, command=new_window.destroy,
                      font=("Courier New", 10), bg="#aad7f2")
    close.pack()


def scan_window(com):
    scan_wind = tk.Toplevel(root)
    scan_wind.geometry('+700+100')
    # sets the title of the
    # Toplevel widget
    scan_wind.title("Servo Scan")

    # sets the geometry of toplevel
    scan_wind.attributes("-topmost", True)
    current_value = tk.StringVar(value=0)
    spin_box = tk.Spinbox(
        scan_wind,
        from_=0,
        to=450,
        width=30,
        textvariable=current_value,
        wrap=True)

    spin_box.pack()

    scan_but = tk.Button(scan_wind, text="Start", width=25,
                         command=lambda: object_detector_gui(com, False, int(current_value.get())),
                         font=("Courier New", 10), bg="#aad7f2")
    # Script_M.pack()
    scan_but.pack()

    close = tk.Button(scan_wind, text="close", width=25, command=scan_wind.destroy,
                      font=("Courier New", 10), bg="#aad7f2")
    # Script_M.pack()
    close.pack()


def telemeter_window(s):
    scan_wind = tk.Toplevel(root)
    scan_wind.geometry('+700+100')
    # sets the title of the
    # Toplevel widget
    scan_wind.title("Telemeter")

    # sets the geometry of toplevel
    # scan_wind.geometry("200x200")
    scan_wind.attributes("-topmost", True)
    current_value = tk.StringVar(value=0)
    spin_box = tk.Spinbox(
        scan_wind,
        width=30,
        from_=0,
        to=180,
        textvariable=current_value,
        wrap=True)

    spin_box.pack()

    label1 = tk.Label(scan_wind, text="Press e to stop", width=15, font=("Courier New", 15, "bold"))
    label1.pack()

    scan_but = tk.Button(scan_wind, text="Start", width=25,
                         command=lambda: telemeter_function(s, 0, int(current_value.get())),
                         font=("Courier New", 10), bg="#aad7f2")
    # Script_M.pack()
    scan_but.pack()

    close = tk.Button(scan_wind, text="close", width=25, command=scan_wind.destroy,
                      font=("Courier New", 10), bg="#aad7f2")
    # Script_M.pack()
    close.pack()


# ------------------------Gui---------------------------------------------------------------

root.geometry('+1000+50')
objects_d = tk.Button(root, text="Objects Detector System", width=30, command=lambda: scan_window(s),
                      font=("Courier New", 10), bg="#aad7f2")
# Objects_d.pack()
objects_d.pack()

telemeter = tk.Button(root, text="Telemeter", width=30, command=lambda: telemeter_window(s), font=("Courier New", 10),
                      bg="#aad7f2")
# Telemeter.pack()
telemeter.pack()

light_s = tk.Button(root, text="Light Sources Detector System", width=30, command=lambda: light_sources_detector(s),
                    font=("Courier New", 10), bg="#aad7f2")

light_s.pack()

script_butt = tk.Button(root, text="Execute Scripts", width=30, command=lambda: open_script_window(s),
                        font=("Courier New", 10), bg="#aad7f2")
script_butt.pack()
# Light_S.pack()

send_butt = tk.Button(root, text="Send Scripts", width=30, command=lambda: open_send_window(s),
                      font=("Courier New", 10),
                      bg="#aad7f2")
send_butt.pack()

plt.show(block=False)

# a button widget which will open a
# new window on button click

root.attributes("-topmost", True)
# mainloop, runs infinitely
root.mainloop()
