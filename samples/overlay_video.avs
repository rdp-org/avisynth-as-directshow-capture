a = DirectShowSource("screen-capture-recorder.grf", audio=False, framecount=1000000, fps=2)
b = DirectShowSource("screen-capture-recorder.grf", audio=False, framecount=1000000, fps=2)
smaller = BicubicResize(b, 100,100)
c = Overlay(a, smaller, 50, 50) # place 'smaller' over a, starting at position 50,50
c
