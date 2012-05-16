This program allows one to take input from an AviSynth file, and input it to your "favorite" directshow app (vlc, ffmpeg, streaming utilities).

AviSynth files can stream files or live capture sources.  They can also apply "extra filters" to the output, like overlaying one image over another, etc.

= Installation =

Install "avisynth" 2.x (https://sourceforge.net/projects/avisynth2)

Now create a file, like "test.avs" with contents:

Version()

Now set the registry key (see configuration_setup_utility/setup_filename_to_use.rb for keyname) to the full path of the file you want to use.
(or use one of the helper setups provided


= Using Live Sources =

To input a live source, create a "graphedit" file (download GraphStudio, run it, insert filters -> video capture source), save.  Then in your avs file, it will look like.
directshowsource("screen-capture-recorder.GRF", audio=False, framecount=1000000, fps=10) # fps is just to force it 'down' to that fps, or 'fake it up' to that fps through duplication

= Trouble shooting =

Sometimes you need to set the fps to double what you actually want, since AviSynth is, 
timing wise, I think "just missing" frames so it always waits for a second one to come through.

Ping me if you have any feedback/feature requests you'd like me to do/add.