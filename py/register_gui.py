import wx
from serial.tools import list_ports
import serial
from struct import unpack, pack
from Cryptodome.Cipher import AES
import time
from os import path
from datetime import datetime

SECRET = bytes([0x02, 0xc8, 0x69, 0x40, 0xec, 0x17, 0xe0, 0xf8, 0xbd, 0xaa, 0xfd, 0x2b, 0xa4, 0x1c, 0xa8, 0x78])
POSTFIX = [
    0x00000000,
    0x9868ee46,
    0x5846d7dc
]

log = path.join(path.dirname(path.abspath(__file__)), 'verified.log')


def toHex(bs):
    return ' '.join('%02x' % c for c in bs)


COLOR_GREEN = (41, 145, 13)
COLOR_RED = (244, 67, 54)


class Demo(wx.Frame):
    def __init__(self, *args, **kw):
        super(Demo, self).__init__(*args, **kw)
        self.tty = None
        self.uid = 0
        self.ttys = [d.device for d in list_ports.comports()][::-1]

        self.init_ui()

    def init_ui(self):
        pnl = wx.Panel(self)
        vbox = wx.BoxSizer(wx.VERTICAL)
        hbox_tty = wx.BoxSizer(wx.HORIZONTAL)

        self.cb_tty = wx.ComboBox(pnl, choices=self.ttys, style=wx.CB_READONLY)

        tb_connect = wx.ToggleButton(pnl, -1, 'Connect')
        tb_connect.Bind(wx.EVT_TOGGLEBUTTON, self.OnToggleConnect)

        self.btn_verify = wx.Button(pnl, -1, 'Verify')
        self.btn_verify.Bind(wx.EVT_BUTTON, self.OnVerify)
        self.btn_verify.Enable(False)

        self.btn_authorize = wx.Button(pnl, -1, 'Authorize')
        self.btn_authorize.Bind(wx.EVT_BUTTON, self.OnAuthorize)
        self.btn_authorize.Enable(False)

        hbox_tty.Add(wx.StaticText(pnl, -1, 'Port'), 0, wx.ALIGN_CENTER_VERTICAL)
        hbox_tty.Add(self.cb_tty, 1, wx.ALIGN_CENTER_VERTICAL)
        hbox_tty.Add(tb_connect, 0, wx.ALIGN_CENTER_VERTICAL)
        hbox_tty.Add(self.btn_verify, 0, wx.ALIGN_CENTER_VERTICAL)
        hbox_tty.Add(self.btn_authorize, 0, wx.ALIGN_CENTER_VERTICAL)

        vbox.Add(hbox_tty, 0, wx.EXPAND | wx.ALL, 4)

        hbox_status = wx.BoxSizer(wx.HORIZONTAL)
        self.lbl_status = wx.StaticText(pnl, -1, 'Status')
        hbox_status.Add(self.lbl_status, 0, wx.ALIGN_CENTER_VERTICAL)

        vbox.Add(hbox_status, 0, wx.EXPAND | wx.ALL, 4)

        pnl.SetSizer(vbox)

        self.CreateStatusBar()
        self.Centre()
        self.Show(True)

    def OnToggleConnect(self, e):
        isPressed = e.GetEventObject().GetValue()
        self.cb_tty.Enable(not isPressed)

        self.btn_verify.Enable(isPressed)
        self.btn_authorize.Enable(isPressed)

        if isPressed:
            self.tty = serial.Serial(self.cb_tty.GetValue(), 115200, timeout=0.2)
        else:
            self.tty.close()

    def OnVerify(self, e):
        self.tty.reset_input_buffer()
        self.write(bytes([0xf3]))
        rx = self.tty.read(6)
        if len(rx) < 6:
            self.lbl_status.SetLabel('no response')
            self.lbl_status.SetForegroundColour(COLOR_RED)
        elif len(rx) > 6:
            self.lbl_status.SetLabel('wrong response: %s' % toHex(rx))
            self.lbl_status.SetForegroundColour(COLOR_RED)
        else:
            if rx[-2]:
                self.lbl_status.SetLabel('Authorized: %s' % toHex(rx))
                self.lbl_status.SetForegroundColour(COLOR_GREEN)
            else:
                self.lbl_status.SetLabel('Unauthorized: %s' % toHex(rx))
                self.lbl_status.SetForegroundColour(COLOR_RED)

    def OnAuthorize(self, e):
        error = 0
        while True:
            self.write(bytes([0xf3]))
            rx = self.tty.read(6)

            if len(rx) != 6:
                if len(rx) == 0:
                    error = 1
                    break
                error = 2
                break

            version = rx[-1]
            self.write(bytes([0xf0]))
            sid = self.tty.read(16)

            if len(sid) != 16:
                if len(sid) == 0:
                    error = 1
                    break
                error = 2
                break

            with open(log, 'a') as f:
                f.write(datetime.now().isoformat() + ' ' + sid[4:].hex() + '\n')

            cipher = AES.new(SECRET, AES.MODE_ECB)
            license = cipher.encrypt(sid[4:] + pack('<I', POSTFIX[version]))
            print(toHex(license))
            self.write(bytes([0xf2]) + license + bytes([sum(license) & 0xff]))
            rx = self.tty.read(4)
            if len(rx) != 4:
                if len(rx) == 0:
                    error = 1
                    break
                error = 2
                break

            time.sleep(0.2) # for reset
            self.write(bytes([0xf3]))
            rx = self.tty.read(6)
            if len(rx) != 6:
                if len(rx) == 0:
                    error = 1
                    break
                error = 2
                break

            break

        if error == 1:
            self.lbl_status.SetLabel('no response')
            self.lbl_status.SetForegroundColour(COLOR_RED)
        elif error == 2:
            self.lbl_status.SetLabel('wrong response: %s' % toHex(rx))
            self.lbl_status.SetForegroundColour(COLOR_RED)
        else:
            self.lbl_status.SetLabel('Authorize done.')
            self.lbl_status.SetForegroundColour(COLOR_GREEN)

        # self.lbl_status.SetLabel('Authorize')

    def write(self, data):
        tx = bytes([len(data) + 2, self.uid, 0xff - self.uid]) + data
        # print('tx:', toHex(tx), datetime.now())
        # print('tx:', toHex(tx))
        self.tty.write(tx)
        self.uid += 1
        self.uid &= 0xff


if __name__ == '__main__':
    app = wx.App()
    Demo(None, title='Hunterio Servo Board Register', size=(600, 128))
    app.MainLoop()
