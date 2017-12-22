import wx
from serial.tools import list_ports


class Demo(wx.Frame):
    def __init__(self, *args, **kw):
        super(Demo, self).__init__(*args, **kw)

        menubar = wx.MenuBar()
        fileMenu = wx.Menu()
        fitem = fileMenu.Append(wx.ID_EXIT, 'Quit', 'Quit application')
        fitem = fileMenu.Append(wx.ID_ABOUT, 'About', 'About application')
        menubar.Append(fileMenu, '&File')
        self.SetMenuBar(menubar)
        #
        # self.Bind(wx.EVT_MENU, self.OnQuit, fitem)

        pnl = wx.Panel(self)

        distros = [ d.device for d in list_ports.comports()]
        cb = wx.ComboBox(pnl, pos=(50, 30), choices=distros, style=wx.CB_READONLY)
        self.st = wx.StaticText(pnl, label='', pos=(50, 140))
        cb.Bind(wx.EVT_COMBOBOX, self.OnSelect)

        self.Centre()
        self.Show(True)

    def OnSelect(self, e):
        i = e.GetString()
        self.st.SetLabel(i)

    def OnQuit(self, e):
        self.Close()

if __name__ == '__main__':
    app = wx.App()
    Demo(None, title='Hunterio Servo Dongle', size=(800, 600))
    app.MainLoop()
    # for port in list_ports.comports():
    #     print(port.device)
    # print()
