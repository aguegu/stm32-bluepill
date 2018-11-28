import wx
from serial.tools import list_ports
import serial
from struct import unpack, pack
from functools import reduce
import time

LEN = 16
LEN_SHOW = 4
MINIMUM = 50
MAXIMUM = 600


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
        self.sld = []

        self.btn_init = []
        self.btn_mid = []
        self.btn_min = []
        self.btn_max = []
        self.btn_send = []

        self.btn_plus = []
        self.btn_minus = []

        self.ttys = [d.device for d in list_ports.comports()][::-1]

        self.init_ui()

    def init_ui(self):
        pnl = wx.Panel(self)

        vbox = wx.BoxSizer(wx.VERTICAL)
        hbox_tty = wx.BoxSizer(wx.HORIZONTAL)

        self.cb_tty = wx.ComboBox(pnl, choices=self.ttys, style=wx.CB_READONLY)

        tb_connect = wx.ToggleButton(pnl, -1, 'Connect')
        tb_connect.Bind(wx.EVT_TOGGLEBUTTON, self.OnToggleConnect)

        self.btn_reset = wx.Button(pnl, -1, 'Reset')
        self.btn_reset.Bind(wx.EVT_BUTTON, self.OnReset)
        self.btn_reset.Enable(False)

        self.btn_read = wx.Button(pnl, -1, 'Read')
        self.btn_read.Bind(wx.EVT_BUTTON, self.OnRead)
        self.btn_read.Enable(False)

        hbox_tty.Add(wx.StaticText(pnl, -1, 'Port'), 0, wx.ALIGN_CENTER_VERTICAL)
        hbox_tty.Add(self.cb_tty, 1, wx.ALIGN_CENTER_VERTICAL)
        hbox_tty.Add(tb_connect, 0, wx.ALIGN_CENTER_VERTICAL)
        hbox_tty.Add(self.btn_read, 0, wx.ALIGN_CENTER_VERTICAL)
        hbox_tty.Add(self.btn_reset, 0, wx.ALIGN_CENTER_VERTICAL)

        vbox.Add(hbox_tty, 0, wx.EXPAND | wx.ALL, 4)

        for i in range(LEN_SHOW):
            hbox_tuning = wx.BoxSizer(wx.HORIZONTAL)

            hbox_tuning.Add(wx.StaticText(pnl, -1, 'Servo #%02d' % i), 0, wx.ALIGN_CENTER_VERTICAL)

            # hbox_tuning.Add(wx.StaticText(pnl, -1, 'initial'), 0, wx.ALIGN_CENTER_VERTICAL)

            btn_init = wx.Button(pnl, -1, 'Initial', size=(48, -1))
            btn_init.Bind(wx.EVT_BUTTON, self.OnInit)
            btn_init.Enable(False)
            hbox_tuning.Add(btn_init, 0, wx.ALIGN_CENTER_VERTICAL)
            self.btn_init.append(btn_init)

            sc_init = wx.SpinCtrl(pnl, -1, size=(60, -1))
            sc_init.SetRange(MINIMUM, MAXIMUM)
            hbox_tuning.Add(sc_init, 0, wx.ALIGN_CENTER_VERTICAL)
            self.sc_init.append(sc_init)

            btn_mid = wx.Button(pnl, -1, 'Middle', size=(64, -1))
            btn_mid.Bind(wx.EVT_BUTTON, self.OnMid)
            btn_mid.Enable(False)
            hbox_tuning.Add(btn_mid, 0, wx.ALIGN_CENTER_VERTICAL)
            self.btn_mid.append(btn_mid)
            # hbox_tuning.Add(wx.StaticText(pnl, -1, 'Middle'), 0, wx.ALIGN_CENTER_VERTICAL)
            sc_mid = wx.SpinCtrl(pnl, -1, size=(60, -1))
            sc_mid.SetRange(MINIMUM, MAXIMUM)
            hbox_tuning.Add(sc_mid, 0, wx.ALIGN_CENTER_VERTICAL)
            self.sc_mid.append(sc_mid)

            # hbox_tuning.Add(wx.StaticText(pnl, -1, 'Minimun'), 0, wx.ALIGN_CENTER_VERTICAL)
            btn_min = wx.Button(pnl, -1, 'Minimum', size=(80, -1))
            btn_min.Bind(wx.EVT_BUTTON, self.OnMin)
            btn_min.Enable(False)
            hbox_tuning.Add(btn_min, 0, wx.ALIGN_CENTER_VERTICAL)
            self.btn_min.append(btn_min)

            sc_min = wx.SpinCtrl(pnl, -1, size=(60, -1))
            sc_min.SetRange(MINIMUM, MAXIMUM)
            hbox_tuning.Add(sc_min, 0, wx.ALIGN_CENTER_VERTICAL)
            self.sc_min.append(sc_min)

            # hbox_tuning.Add(wx.StaticText(pnl, -1, 'Maximum'), 0, wx.ALIGN_CENTER_VERTICAL)

            btn_max = wx.Button(pnl, -1, 'Maximum', size=(80, -1))
            btn_max.Bind(wx.EVT_BUTTON, self.OnMax)
            btn_max.Enable(False)
            hbox_tuning.Add(btn_max, 0, wx.ALIGN_CENTER_VERTICAL)
            self.btn_max.append(btn_max)

            sc_max = wx.SpinCtrl(pnl, -1, size=(60, -1))
            sc_max.SetRange(MINIMUM, MAXIMUM)
            hbox_tuning.Add(sc_max, 0, wx.ALIGN_CENTER_VERTICAL)
            self.sc_max.append(sc_max)

            btn_send = wx.Button(pnl, -1, 'Set', size=(40, -1))
            btn_send.Bind(wx.EVT_BUTTON, self.OnSend)
            btn_send.Enable(False)
            hbox_tuning.Add(btn_send, 0, wx.ALIGN_CENTER_VERTICAL)
            self.btn_send.append(btn_send)

            sld = wx.Slider(pnl, -1, size=(300, -1), style=wx.SL_HORIZONTAL | wx.SL_LABELS)
            sld.Bind(wx.EVT_SLIDER, self.OnSlide)
            sld.Enable(False)
            sld.SetRange(MINIMUM, MAXIMUM)
            hbox_tuning.Add(sld, 0, wx.ALIGN_CENTER_VERTICAL)
            self.sld.append(sld)

            btn_minus = wx.Button(pnl, -1, '-', size=(32, -1))
            btn_minus.Bind(wx.EVT_BUTTON, self.OnMinus)
            btn_minus.Enable(False)
            hbox_tuning.Add(btn_minus, 0, wx.ALIGN_CENTER_VERTICAL)
            self.btn_minus.append(btn_minus)

            btn_plus = wx.Button(pnl, -1, '+', size=(32, -1))
            btn_plus.Bind(wx.EVT_BUTTON, self.OnPlus)
            btn_plus.Enable(False)
            hbox_tuning.Add(btn_plus, 0, wx.ALIGN_CENTER_VERTICAL)
            self.btn_plus.append(btn_plus)

            vbox.Add(hbox_tuning, 0, wx.EXPAND | wx.ALL, 4)

        pnl.SetSizer(vbox)

        self.statusbar = self.CreateStatusBar(1)
        # self.statusbar.SetStatusText('Sourcekit Servo Calibrator')

        self.Centre()
        self.Show(True)

    def OnQuit(self, e):
        if self.tty:
            self.tty.close()
        self.Close()

    def enableControls(self, enabled):
        self.cb_tty.Enable(not enabled)

        for ctrl in self.btn_send + self.btn_init + self.btn_mid + self.btn_min + self.btn_max + self.btn_minus + self.btn_plus + self.sld:
            ctrl.Enable(enabled)

        self.btn_read.Enable(enabled)
        self.btn_reset.Enable(enabled)

    def OnToggleConnect(self, e):
        isPressed = e.GetEventObject().GetValue()
        tty = self.cb_tty.GetValue()

        if not tty:
            e.GetEventObject().SetValue(False)
            return

        if self.tty:
            self.statusbar.SetStatusText('%s disconnected' % tty)
            self.tty.close()
            self.tty = None

        self.enableControls(False)

        if isPressed:
            e.GetEventObject().Enable(False)
            for i in range(10):
                try:
                    self.tty = serial.Serial(tty, 115200, timeout=0.2)
                except serial.SerialException as ex:
                    self.statusbar.SetStatusText("%d try failed. %s try again in 3 seconds." % (i + 1, ex.strerror))
                    e.GetEventObject().SetValue(False)
                    time.sleep(3)
                else:
                    self.statusbar.SetStatusText('%s connected' % tty)
                    self.enableControls(True)
                    break
            e.GetEventObject().Enable(True)


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
        print('rx:', toHex(rx))
        data = unpack('<' + 'H' * LEN * 4, rx[4:])
        print(data)
        for i in range(LEN):
            if i < LEN_SHOW:
                self.sc_init[i].SetValue(data[i * 4])
                self.sc_mid[i].SetValue(data[i * 4 + 1])
                self.sc_min[i].SetValue(data[i * 4 + 2])
                self.sc_max[i].SetValue(data[i * 4 + 3])
        print(data)

    def OnSlide(self, e):
        sld = e.GetEventObject()
        i = self.sld.index(sld)
        self.write(pack('<BBHHB', 0x01, i, sld.GetValue(), 1, 0))
        rx = self.tty.read(4)
        print('rx:', toHex(rx))
        # print(e)

    def OnInit(self, e):
        btn = e.GetEventObject()
        i = self.btn_init.index(btn)
        self.sc_init[i].SetValue(self.sld[i].GetValue())

    def OnMid(self, e):
        btn = e.GetEventObject()
        i = self.btn_mid.index(btn)
        self.sc_mid[i].SetValue(self.sld[i].GetValue())

    def OnMin(self, e):
        btn = e.GetEventObject()
        i = self.btn_min.index(btn)
        self.sc_min[i].SetValue(self.sld[i].GetValue())

    def OnMax(self, e):
        btn = e.GetEventObject()
        i = self.btn_max.index(btn)
        self.sc_max[i].SetValue(self.sld[i].GetValue())

    def OnPlus(self, e):
        btn = e.GetEventObject()
        i = self.btn_plus.index(btn)
        self.sld[i].SetValue(min(self.sld[i].GetValue() + 1, MAXIMUM))
        self.write(pack('<BBHHB', 0x01, i, self.sld[i].GetValue(), 1, 0))
        rx = self.tty.read(4)
        print('rx:', toHex(rx))

    def OnMinus(self, e):
        btn = e.GetEventObject()
        i = self.btn_minus.index(btn)
        self.sld[i].SetValue(max(self.sld[i].GetValue() - 1, MINIMUM))
        self.write(pack('<BBHHB', 0x01, i, self.sld[i].GetValue(), 1, 0))
        rx = self.tty.read(4)
        print('rx:', toHex(rx))

    def OnReset(self, e):
        payload = [pack('<BHHHH', i, 306, 306, MINIMUM, MAXIMUM) for i in range(LEN)]
        self.write(reduce(lambda c, x: c + x, payload, bytearray.fromhex('03')))
        rx = self.tty.read(4)
        print('rx:', toHex(rx))


if __name__ == '__main__':
    app = wx.App()
    Demo(None, title='SourceKit Servo Dongle', size=(1000, 600))
    app.MainLoop()
