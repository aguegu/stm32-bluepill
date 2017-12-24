import wx
from serial.tools import list_ports
import serial
from struct import unpack, pack


def toHex(bs):
    return ' '.join('0x%02x' % c for c in bs)

class Demo(wx.Frame):

    def __init__(self, *args, **kw):
        super(Demo, self).__init__(*args, **kw)
        self.init_ui()
        self.tty = None
        self.uid = 0

    def init_ui(self):
        pnl = wx.Panel(self)

        vbox = wx.BoxSizer(wx.VERTICAL)
        hbox_tty = wx.BoxSizer(wx.HORIZONTAL)

        distros = [d.device for d in list_ports.comports()][::-1]
        self.cb_tty = wx.ComboBox(pnl, choices=distros, style=wx.CB_READONLY)

        tb_connect = wx.ToggleButton(pnl, -1, 'Connect')
        tb_connect.Bind(wx.EVT_TOGGLEBUTTON, self.OnToggleConnect)

        hbox_tty.Add(wx.StaticText(pnl, -1, 'Port'), 0, wx.ALIGN_CENTER_VERTICAL)
        hbox_tty.Add(self.cb_tty, 1, wx.ALIGN_CENTER_VERTICAL)
        hbox_tty.Add(tb_connect, 0, wx.ALIGN_CENTER_VERTICAL)

        vbox.Add(hbox_tty, 0, wx.EXPAND | wx.ALL, 4)

        hbox_tuning = wx.BoxSizer(wx.HORIZONTAL)

        hbox_tuning.Add(wx.StaticText(pnl, -1, 'Servo #'), 0, wx.ALIGN_CENTER_VERTICAL)
        self.sc_servo = wx.SpinCtrl(pnl, -1)
        self.sc_servo.SetRange(-1, 16)
        hbox_tuning.Add(self.sc_servo, 0, wx.ALIGN_CENTER_VERTICAL)

        hbox_tuning.Add(wx.StaticText(pnl, -1, 'Width'), 0, wx.ALIGN_CENTER_VERTICAL)
        self.sc_width = wx.SpinCtrl(pnl, -1, '306')
        self.sc_width.SetRange(1, 1000)
        hbox_tuning.Add(self.sc_width, 0, wx.ALIGN_CENTER_VERTICAL)

        hbox_tuning.Add(wx.StaticText(pnl, -1, 'Duration'), 0, wx.ALIGN_CENTER_VERTICAL)
        self.sc_span = wx.SpinCtrl(pnl, -1, '100')
        self.sc_span.SetRange(1, 65535)
        hbox_tuning.Add(self.sc_span, 0, wx.ALIGN_CENTER_VERTICAL)

        self.btn_send = wx.Button(pnl, -1, 'Send')
        self.btn_send.Bind(wx.EVT_BUTTON, self.OnSend)
        self.btn_send.Enable(False)
        hbox_tuning.Add(self.btn_send, 0, wx.ALIGN_CENTER_VERTICAL | wx.ALIGN_RIGHT)

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
        self.btn_send.Enable(isPressed)
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
        payload = bytes([0x01])
        if self.sc_servo.GetValue() == -1:
            for i in range(4):
                payload += pack('<BHHB', i, self.sc_width.GetValue(), self.sc_span.GetValue(), 0)
        else:
            payload += pack('<BHHB', self.sc_servo.GetValue(), self.sc_width.GetValue(), self.sc_span.GetValue(), 0)
        self.write(payload)

if __name__ == '__main__':
    app = wx.App()
    Demo(None, title='Hunterio Servo Dongle', size=(800, 600))
    app.MainLoop()
