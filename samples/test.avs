a = DirectShowSource("screen-capture-recorder.grf", audio=False, framecount=1000000, fps=10)

b = DirectShowSource("screen-capture-recorder.grf", audio=False, framecount=1000000, fps=10)

Overlay(a, b, -15,-13)