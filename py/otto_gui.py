import wx
from serial.tools import list_ports
import serial
from struct import unpack, pack


def toHex(bs):
    return ' '.join('0x%02x' % c for c in bs)


class Demo(wx.Frame):
    def __init__(self, *args, **kw):
        super(Demo, self).__init__(*args, **kw)
        self.tty = None
        self.uid = 0

        self.sc_servo = []
        self.sc_width = []
        self.sc_span = []
        self.btn_send = []
        self.cb_curve = []

        self.ttys = [d.device for d in list_ports.comports()][::-1]

        self.curves = [
            'Linear',
            'SineIn',
            'SineOut',
            'SineInOut',
            'QuadIn',
            'QuadOut',
            'QuadInOut',
            'CubicIn',
            'CubicOut',
            'CubicInOut',
            'QuarticIn',
            'QuarticOut',
            'QuarticInOut',
            'ExponentialIn',
            'ExponentialOut',
            'ExponentialInOut',
            'CircularIn',
            'CircularOut',
            'CircularInOut',
            'BackIn',
            'BackOut',
            'BackInOut',
            'ElasticIn',
            'ElasticOut',
            'ElasticInOut',
            'BounceIn',
            'BounceOut',
            'BounceInOut',
        ]

        self.init_ui()

    def init_ui(self):
        pnl = wx.Panel(self)

        vbox = wx.BoxSizer(wx.VERTICAL)
        hbox_tty = wx.BoxSizer(wx.HORIZONTAL)

        self.cb_tty = wx.ComboBox(pnl, choices=self.ttys, style=wx.CB_READONLY)

        tb_connect = wx.ToggleButton(pnl, -1, 'Connect')
        tb_connect.Bind(wx.EVT_TOGGLEBUTTON, self.OnToggleConnect)

        hbox_tty.Add(wx.StaticText(pnl, -1, 'Port'), 0, wx.ALIGN_CENTER_VERTICAL)
        hbox_tty.Add(self.cb_tty, 1, wx.ALIGN_CENTER_VERTICAL)
        hbox_tty.Add(tb_connect, 0, wx.ALIGN_CENTER_VERTICAL)

        vbox.Add(hbox_tty, 0, wx.EXPAND | wx.ALL, 4)

        # curves = ['%s (0x%02x)' % (s, i) for i, s in enumerate(self.curves)]

        for i in range(10):
            hbox_tuning = wx.BoxSizer(wx.HORIZONTAL)

            hbox_tuning.Add(wx.StaticText(pnl, -1, 'Servo #'), 0, wx.ALIGN_CENTER_VERTICAL)
            sc_servo = wx.SpinCtrl(pnl, -1)
            sc_servo.SetRange(-1, 16)
            hbox_tuning.Add(sc_servo, 0, wx.ALIGN_CENTER_VERTICAL)
            self.sc_servo.append(sc_servo)

            hbox_tuning.Add(wx.StaticText(pnl, -1, 'Width'), 0, wx.ALIGN_CENTER_VERTICAL)
            sc_width = wx.SpinCtrl(pnl, -1, '306')
            sc_width.SetRange(1, 1000)
            hbox_tuning.Add(sc_width, 0, wx.ALIGN_CENTER_VERTICAL)
            self.sc_width.append(sc_width)

            hbox_tuning.Add(wx.StaticText(pnl, -1, 'Duration'), 0, wx.ALIGN_CENTER_VERTICAL)
            sc_span = wx.SpinCtrl(pnl, -1, '100')
            sc_span.SetRange(1, 65535)
            hbox_tuning.Add(sc_span, 0, wx.ALIGN_CENTER_VERTICAL)
            self.sc_span.append(sc_span)

            hbox_tuning.Add(wx.StaticText(pnl, -1, 'Curve'), 0, wx.ALIGN_CENTER_VERTICAL)
            cb_curve = wx.ComboBox(pnl, choices=self.curves, style=wx.CB_READONLY)
            hbox_tuning.Add(cb_curve, 0, wx.ALIGN_CENTER_VERTICAL)
            self.cb_curve.append(cb_curve)

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
        if isPressed:
            self.tty = serial.Serial(self.cb_tty.GetValue(), 115200, timeout=0.1)
        else:
            self.tty.close()

    def write(self, data):
        tx = bytes([len(data) + 2, self.uid, 0xff - self.uid]) + data
        # print('tx:', toHex(tx), datetime.now())
        print('tx:', toHex(tx))
        self.tty.write(tx)
        rx_len = self.tty.read(1)
        rx_payload = self.tty.read(unpack('B', rx_len)[0])
        print('rx:', toHex(rx_len), toHex(rx_payload))


        self.uid += 1
        self.uid &= 0xff

    def OnSend(self, e):
        payload = bytes([0x01])
        btn_send = e.GetEventObject()
        i = self.btn_send.index(btn_send)

        curve = self.curves.index(self.cb_curve[i].GetValue())
        if self.sc_servo[i].GetValue() == -1:
            for i in range(4):
                payload += pack('<BHHB', i, self.sc_width[i].GetValue(), self.sc_span[i].GetValue(), curve)
        else:
            payload += pack('<BHHB', self.sc_servo[i].GetValue(), self.sc_width[i].GetValue(), self.sc_span[i].GetValue(), curve)
        self.write(payload)

if __name__ == '__main__':
    app = wx.App()
    Demo(None, title='Hunterio Servo Dongle', size=(800, 600))
    app.MainLoop()
