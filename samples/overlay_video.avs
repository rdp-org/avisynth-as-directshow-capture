a = DirectShowSource("screen-capture-recorder.grf", audio=False, framecount=1000000, fps=2)
b = DirectShowSource("usb.grf", audio=False, framecount=1000000, fps=2)
c = Overlay(a, b, 50,50)
c
