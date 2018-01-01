import wx
from serial.tools import list_ports
import serial
from struct import unpack, pack

LEN = 16

def toHex(bs):
    return ' '.join('0x%02x' % c for c in bs)


class Demo(wx.Frame):
    def __init__(self, *args, **kw):
        super(Demo, self).__init__(*args, **kw)
        self.tty = None
        self.uid = 0

        self.sc_init = []
        self.sc_mid = []
        self.sc_min = []
        self.sc_max = []
        self.btn_send = []

        self.ttys = [d.device for d in list_ports.comports()][::-1]

        self.init_ui()

    def init_ui(self):
        pnl = wx.Panel(self)

        vbox = wx.BoxSizer(wx.VERTICAL)
        hbox_tty = wx.BoxSizer(wx.HORIZONTAL)

        self.cb_tty = wx.ComboBox(pnl, choices=self.ttys, style=wx.CB_READONLY)

        tb_connect = wx.ToggleButton(pnl, -1, 'Connect')
        tb_connect.Bind(wx.EVT_TOGGLEBUTTON, self.OnToggleConnect)

        self.btn_read = wx.Button(pnl, -1, 'Read')
        self.btn_read.Bind(wx.EVT_BUTTON, self.OnRead)
        self.btn_read.Enable(False)

        hbox_tty.Add(wx.StaticText(pnl, -1, 'Port'), 0, wx.ALIGN_CENTER_VERTICAL)
        hbox_tty.Add(self.cb_tty, 1, wx.ALIGN_CENTER_VERTICAL)
        hbox_tty.Add(tb_connect, 0, wx.ALIGN_CENTER_VERTICAL)
        hbox_tty.Add(self.btn_read, 0, wx.ALIGN_CENTER_VERTICAL)

        vbox.Add(hbox_tty, 0, wx.EXPAND | wx.ALL, 4)

        # curves = ['%s (0x%02x)' % (s, i) for i, s in enumerate(self.curves)]

        for i in range(LEN):
            hbox_tuning = wx.BoxSizer(wx.HORIZONTAL)

            hbox_tuning.Add(wx.StaticText(pnl, -1, 'Servo #%02d' % i), 0, wx.ALIGN_CENTER_VERTICAL)

            hbox_tuning.Add(wx.StaticText(pnl, -1, 'initial'), 0, wx.ALIGN_CENTER_VERTICAL)
            sc_init = wx.SpinCtrl(pnl, -1)
            sc_init.SetRange(1, 65535)
            hbox_tuning.Add(sc_init, 0, wx.ALIGN_CENTER_VERTICAL)
            self.sc_init.append(sc_init)

            hbox_tuning.Add(wx.StaticText(pnl, -1, 'Middle'), 0, wx.ALIGN_CENTER_VERTICAL)
            sc_mid = wx.SpinCtrl(pnl, -1)
            sc_mid.SetRange(1, 65535)
            hbox_tuning.Add(sc_mid, 0, wx.ALIGN_CENTER_VERTICAL)
            self.sc_mid.append(sc_mid)

            hbox_tuning.Add(wx.StaticText(pnl, -1, 'Minimun'), 0, wx.ALIGN_CENTER_VERTICAL)
            sc_min = wx.SpinCtrl(pnl, -1)
            sc_min.SetRange(1, 65535)
            hbox_tuning.Add(sc_min, 0, wx.ALIGN_CENTER_VERTICAL)
            self.sc_min.append(sc_min)

            hbox_tuning.Add(wx.StaticText(pnl, -1, 'Maximum'), 0, wx.ALIGN_CENTER_VERTICAL)
            sc_max = wx.SpinCtrl(pnl, -1)
            sc_max.SetRange(1, 65535)
            hbox_tuning.Add(sc_max, 0, wx.ALIGN_CENTER_VERTICAL)
            self.sc_max.append(sc_max)

            btn_send = wx.Button(pnl, -1, 'Send')
            btn_send.Bind(wx.EVT_BUTTON, self.OnSend)
            btn_send.Enable(False)
            hbox_tuning.Add(btn_send, 0, wx.ALIGN_CENTER_VERTICAL | wx.ALIGN_RIGHT)
            self.btn_send.append(btn_send)

            vbox.Add(hbox_tuning, 0, wx.EXPAND | wx.ALL, 4)

        pnl.SetSizer(vbox)

        self.CreateStatusBar()
        self.Centre()
        self.Show(True)

    def OnQuit(self, e):
        self.Close()

    def OnToggleConnect(self, e):
        isPressed = e.GetEventObject().GetValue()
        self.cb_tty.Enable(not isPressed)
        for btn_send in self.btn_send:
            btn_send.Enable(isPressed)

        self.btn_read.Enable(isPressed)

        if isPressed:
            self.tty = serial.Serial(self.cb_tty.GetValue(), 115200)
        else:
            self.tty.close()

    def write(self, data):
        tx = bytes([len(data) + 2, self.uid, 0xff - self.uid]) + data
        # print('tx:', toHex(tx), datetime.now())
        print('tx:', toHex(tx))
        self.tty.write(tx)
        self.uid += 1
        self.uid &= 0xff

    def OnSend(self, e):
        payload = bytes([0x03])
        btn_send = e.GetEventObject()
        i = self.btn_send.index(btn_send)

        payload += pack('<BHHHH', i, self.sc_init[i].GetValue(), self.sc_mid[i].GetValue(), self.sc_min[i].GetValue(), self.sc_max[i].GetValue())
        self.write(payload)
        self.tty.read(4)

    def OnRead(self, e):
        self.write(bytes([0x04]))
        rx = self.tty.read(LEN * 8 + 4)
        data = unpack('<' + 'H' * LEN * 4, rx[4:])
        print(toHex(data))
        for i in range(LEN):
            self.sc_init[i].SetValue(data[i * 4])
            self.sc_mid[i].SetValue(data[i * 4 + 1])
            self.sc_min[i].SetValue(data[i * 4 + 2])
            self.sc_max[i].SetValue(data[i * 4 + 3])
        print(data)

if __name__ == '__main__':
    app = wx.App()
    Demo(None, title='Hunterio Servo Dongle', size=(800, 600))
    app.MainLoop()
