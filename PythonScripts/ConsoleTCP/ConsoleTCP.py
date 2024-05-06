import tkinter
from tkinter import ttk

import socket
import threading

tx = "null\0"
connected = True
send_semaphore = threading.Semaphore(0)

# tcp communication task
def tcp_comm_task(name):
	global connected 
	global send_semaphore
	global tx

	console_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	console_address = ('192.168.36.109', 49160)
	console_socket.connect(console_address)

	while connected: 
		send_semaphore.acquire()
		sent_bytes = console_socket.send(tx.encode())
		print('command sent')

		if sent_bytes <= 0:
			print('failed to send, disconnecting...')
			connected = False; 

# button callback functions that sets up the tx string to send to the esp32
def device_checkbutton_callback(device_id, enable):
	global send_semaphore
	global tx
	tx = 'DEVICE_' + str(device_id) + '_' + str(int(enable)) + '\0'
	print(tx)
	send_semaphore.release() 

def pwm_button_callback(label, dc_or_sp, value):
	global send_semaphore
	global tx
	tx = 'PWM_' + str(label) + '_' + str(dc_or_sp) + '_' + str(value) + '\0'
	print(tx)
	send_semaphore.release() 

def turbine_button_callback(value):
	global send_semaphore
	global tx
	tx = str(value) + '\0'
	print(tx)
	send_semaphore.release() 

def switch_mcpt():
	global mcpt_en

	if mcpt_en:
		mcpt_button.config(image = mcpt_off)
		mcpt_en = False
		# send command to set the rectifier setpoint to 70V
		pwm_button_callback(labels[0], "SETPOINT", 70)
	
	else:
		mcpt_button.config(image = mcpt_on)
		mcpt_en = True
		# send command to set the rectifier setpoint to 20V
		pwm_button_callback(labels[0], "SETPOINT", 20)

def switch_turbine():
	global turbine_en

	if turbine_en:
		turbine_button.config(image = turbine_off)
		turbine_en = False
		# turn a gpio pin off
		turbine_button_callback(0)
	else:
		turbine_button.config(image = turbine_on)
		turbine_en = True
		# turn a gpio pin on
		turbine_button_callback(1)


# if main()
if __name__ == "__main__":

	# application window
	window = tkinter.Tk()
	window.geometry('800x400')
	window.title('Back-End Console')

	# application notebook
	notebook = ttk.Notebook(master = window)

	# devices tab
	devices_tab = ttk.Frame(master = notebook)
	device_num = 7

	# note: instead of hard declaring 42 tkinter variables, i generated a list of them by looping using list(map()) which reduced 42 hard-coded tkinter variables into 2x7 + 4x7 loops
	# the issue with using a regular for loop to do this is that the iterator is passed by reference when we need it to be passed by value, so list(map()) basically performs the for loop with an iterator passed by value
	en_vars = list(map(lambda i: tkinter.BooleanVar(), range(device_num)))
	en_checkbuttons = list(map(lambda i: ttk.Checkbutton(master = devices_tab, text = 'Enable ' + str(i), variable = en_vars[i], command = lambda: device_checkbutton_callback(i, en_vars[i].get())).pack(), range(device_num)))

	# nukes
	new_window = tkinter.Toplevel(window)
	new_window.geometry('800x480')
	new_window.title('Front-End Console')

	mcpt_en = False
	turbine_en = False

	mcpt_on = tkinter.PhotoImage(file = "mcpt_on.png")
	mcpt_off = tkinter.PhotoImage(file = "mcpt_off.png")
	turbine_on = tkinter.PhotoImage(file = "turbine_on.png")
	turbine_off = tkinter.PhotoImage(file = "turbine_off.png")

	mcpt_button = tkinter.Button(new_window, image = mcpt_off, bd = 0, command = switch_mcpt)
	turbine_button = tkinter.Button(new_window, image = turbine_off, bd = 0, command = switch_turbine)

	mcpt_label = tkinter.Label(new_window, text ="Algorithm MCPT/MPPT", font = ("Helvetica", 18))
	turbine_label = tkinter.Label(new_window, text ="Turbine ON/OFF", font = ("Helvetica", 18))

	mcpt_label.place(x=80, y=50)
	mcpt_button.place(x=50, y=100)
	turbine_label.place(x=460, y=50)
	turbine_button.place(x=400, y=100)

	# pwms tab
	pwms_tab = ttk.Frame(master = notebook)
	labels = ['RECTIFIER', 'SEPIC', 'PS1', 'PS2']
	pwms_num = len(labels)

	pwm_labels = list(map(lambda i: ttk.Label(master = pwms_tab, text = labels[i]), range(pwms_num)))
	dc_vars = list(map(lambda i: tkinter.DoubleVar(), range(pwms_num)))
	dc_entries = list(map(lambda i: ttk.Entry(master = pwms_tab, textvariable = dc_vars[i]), range(pwms_num)))
	dc_buttons = list(map(lambda i: ttk.Button(master = pwms_tab, text = 'Set Duty Cycle', command = lambda: pwm_button_callback(labels[i], "DUTYCYCLE", dc_vars[i].get())), range(pwms_num)))
	sp_vars = list(map(lambda i: tkinter.DoubleVar(), range(pwms_num)))
	sp_entries = list(map(lambda i: ttk.Entry(master = pwms_tab, textvariable = sp_vars[i]), range(pwms_num)))
	sp_buttons = list(map(lambda i: ttk.Button(master = pwms_tab, text = 'Set Setpoint', command = lambda: pwm_button_callback(labels[i], "SETPOINT", sp_vars[i].get())), range(pwms_num)))
	j = 0
	for i in range(pwms_num):
		pwm_labels[i].grid(row = 0, column = j)
		dc_entries[i].grid(row = 1, column = j)
		dc_buttons[i].grid(row = 2, column = j)
		sp_entries[i].grid(row = 3, column = j)
		sp_buttons[i].grid(row = 4, column = j)
		j = j + 1
	
	# add the generated tabs and pack the application notebook
	notebook.add(child = devices_tab, text = 'Devices')
	notebook.add(child = pwms_tab, text = 'PWMs')
	notebook.pack()

	# tcp communication in a separate thread
	tcp_comm_handle = threading.Thread(target = tcp_comm_task, args = ('TCP Communication Task',) )
	tcp_comm_handle.start()

	# run
	window.mainloop()

	