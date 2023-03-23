import tkinter as tk
import socket

class TCPClient:
    def __init__(self, server_ip, server_port):
        self.server_ip = server_ip
        self.server_port = server_port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.server_ip, self.server_port))

    def send(self, message):
        self.sock.sendall(message.encode())

    def receive(self):
        data = self.sock.recv(1024)
        return data.decode()

class App:
    def __init__(self, master):
        self.master = master
        master.title("TCP Client")

        self.server_ip_label = tk.Label(master, text="Server IP")
        self.server_ip_label.pack()
        self.server_ip_entry = tk.Entry(master)
        self.server_ip_entry.pack()

        self.server_port_label = tk.Label(master, text="Server Port")
        self.server_port_label.pack()
        self.server_port_entry = tk.Entry(master)
        self.server_port_entry.pack()

        self.message_label = tk.Label(master, text="Message")
        self.message_label.pack()
        self.message_entry = tk.Entry(master)
        self.message_entry.pack()

        self.send_button = tk.Button(master, text="Send", command=self.send_message)
        self.send_button.pack()

        self.receive_label = tk.Label(master, text="Received Message")
        self.receive_label.pack()
        self.receive_text = tk.Text(master)
        self.receive_text.pack()

    def send_message(self):
        server_ip = self.server_ip_entry.get()
        server_port = int(self.server_port_entry.get())
        
        message = self.message_entry.get()
        self.sock.send(message)
        client = TCPClient(server_ip, server_port)
        client.send(message)
        response = client.receive()
        self.receive_text.delete("1.0", tk.END)
        self.receive_text.insert(tk.END, response)

root = tk.Tk()
app = App(root)
root.mainloop()