This program allows one to take input from an AviSynth file, and input it to your "favorite" directshow app (vlc, ffmpeg, streaming apps, etc.).

AviSynth files can stream files or live capture sources.  They can also apply "extra filters" to the output, like overlaying one image over another, etc.

Really what I wanted it for was to be able to stream multiple videos with "video inside a video" enabled.  AviSynth also has some other filters that may be useful.

Who *doesn't* want to be able to use skype with flexible/controlled input, after all? :)

= Installation =

Install "avisynth" v2.x (http://sourceforge.net/projects/avisynth2/files/AviSynth%202.5)
Install this app, download from https://github.com/rdp-org/avisynth-as-directshow-capture/downloads

Now run "program files" => "avisynth capture" => "test current avisynth config"
It should display a 'version' video.  If it does, eureka! Now proceed to "usage"

= Usage =

Now create your own file, for instance something like "test.avs" with avisynth script contents, for instance:

Version()

Or any other AVS scripts.

Now set the registry key

HKEY_CURRENT_USER\\Software\\avisynth-as-dshow-capture\\avs_filename_to_read

to be a string containig the full path to the file you created.
(or use one of the helper setup utilities provided, to set this key to be the filename).

= AviSynth =

There are lots of AviSynth tutorials around.  It seems slightly powerful than ffmpeg's (still developing) filter system.
Here is my "demo" of combining two live sources into 1: https://github.com/rdp-org/avisynth-as-directshow-capture/tree/master/samples

= Using Live Sources =

To input a live source, create a "graphedit" file (download GraphStudio, run it, insert filters -> video capture source), save.  Then in your avs file, it will look like.
directshowsource("screen-capture-recorder.GRF", audio=False, framecount=1000000, fps=10) # fps is just to force it 'down' to that fps, or 'fake it up' to that fps through duplication

= Trouble shooting =

Sometimes you need to set the fps to double what you actually want, since AviSynth is, 
timing wise, I think "just missing" frames so it always waits for a second one to come through.

Ping me if you have any feedback/feature requests you'd like me to do/add.

Also note that mplayer can already play avisynth files "natively", and ffmpeg/vlc can accept "live source input" natively, so this 
is just another tool for you in your belt.

There is also currently a known bug that if you set an fps, it isn't respected (it just streams as fast as possible).  Ping me if you want this fixed.

= Contact/feedback/issues =

http://groups.google.com/group/roger-projects
roger-projects@googlegroups.com

Source: https://github.com/rdp-org/avisynth-as-directshow-capture
Ping me for any feature requests, like easier video on video, etc.