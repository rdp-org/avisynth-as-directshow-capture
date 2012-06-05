Want to broadcast yourself with a presentation slides *and* your webcam (face) side by side?
Record several webcams in the same video?
This app makes that possible.

Basically this allows you to take arbitrary input, like "2 webcams" or "desktop screen capture and a webcam" and combine them, into VLC.

This program allows one to take input from an AviSynth script file, and input it to your "favorite" directshow app (vlc, ffmpeg, streaming apps, etc.).

AviSynth files can stream files or live capture sources.  They can also apply "extra filters" to the output, like overlaying one image over another, etc.

Really what I wanted it for was to be able to stream multiple videos with "video inside a video" enabled.  AviSynth also has some other filters that may be useful.

Who *doesn't* want to be able to use skype with flexible/controlled input, after all? :)

= Installation =

Install "avisynth" v2.x from http://sourceforge.net/projects/avisynth2/files/AviSynth%202.5 (you can just use the default installer options when it asks you)
Download and install this device driver from https://sourceforge.net/projects/avisynth-dshow/files

Now run "program files" => "avisynth capture" => "test current avisynth config"
It should display a 'Version' small video.  If it does, eureka! Now proceed to "usage"

You can also now use the directshow capture device "avisynth-script-capture-source" in any directshow friendly app,
like ffmpeg, VLC, broadcasters, etc.

= Usage =

Now create your own file, for instance something like "test.avs" with avisynth script contents, for instance:

Version()

Or any other AVS scripts.

Now set the registry key

HKEY_CURRENT_USER\\Software\\avisynth-as-dshow-capture\\avs_filename_to_read

to be a string containig the full path to the file you created.
(or use one of the helper setup utilities provided, to set this key to be the filename).

https://github.com/rdp-org/avisynth-as-directshow-capture/tree/master/samples has a few examples, the script might even work if you install the
https://github.com/rdp/screen-capture-recorder-to-video-windows-free device.

NB that several "other programs" natively accept .avs files, so you don't need this capture at all to use the scripts (mplayer, ffmpeg).
Also note that you may be able to get "similar effects" (overlay, etc.) by using programs that natively accept directshow input, then
using their own "built in" overlay filters, etc. (in which case, you don't need to use this capture device--for instance ffmpeg can broadcast, so can VLC).
I just like being able to use aviSynth since it's so much simpler, and
can hopefully allow for cool effects and also hopefully being able to switch input dynamically, etc. (later) (ping me if you want any of this
implemented).

= AviSynth =

There are lots of AviSynth tutorials around.  It seems slightly powerful than ffmpeg's (still developing) filter system.
Here is my "demo" of combining two live sources into 1: https://github.com/rdp-org/avisynth-as-directshow-capture/tree/master/samples

= Using Live Sources =

To input a live source, create a "graphedit" file (download GraphStudio [1], run it, 
graph [menu] insert filters -> select video capture sources from dropdown, select yourself from list at right), save somewhere (like "filename.GRF").  
Then create your avs file, it will look like.
directshowsource("filename.GRF", audio=False, framecount=1000000, fps=10) # fps is just to force it 'down' to that fps, except for VLC which ignores it [ask if you want that fixed]

[1] http://blog.monogram.sk/janos/tools/monogram-graphstudio/

= TroubleShooting =

Sometimes you need to set the fps to double what you actually want, since AviSynth is, 
timing wise, I think "just missing" frames so it always waits for a second one to come through.

Ping me if you have any feedback/feature requests you'd like me to do/add.

Also note that mplayer can already play avisynth files "natively", and ffmpeg/vlc can accept "live source input" natively, so this 
is just another tool for you in your belt.

There is also currently a known bug that if you set an fps, it isn't respected (it just streams as fast as possible).  Ping me if you want this fixed.

You might need ConvertToYV12 in your script if you want to feed it to Flash Media Live Encoder (I haven't tried it).

= Contact/feedback/issues =

http://groups.google.com/group/roger-projects
roger-projects@googlegroups.com

Source: https://github.com/rdp-org/avisynth-as-directshow-capture
Ping me for any feature requests, like easier video on video, etc.

= See also =

Looks like ffdshow "might" also have something related to this project:
    "ffdshow has an AVISynth filter. Enable it and copy and paste your AVISynth script into the filter's text area. Also make sure the "Add ffdshow video source" checkbox is ticked."

http://rogerdpack.t28.net list my "virtual audio capture directshow device" and also my "screen capture directshow capture device" which may be useful to you/related.

= Future work =
Ping me if you ever want any of the following:
"easier setup" (directshow capture sources without GraphStudio, video on video preview, etc.) 
"going from one script to another dynamically/allowing for a script to change/update/going from one 'view setting' to another"
support for receiving audio
Force VLC to serve frames "in realtime" (currently it just pulls frames as quickly as possible, which isn't quite right)
Hopefully someday ffmpeg on linux will also be able to receive (avxsynth or avisynth) input, so work on this might not be unuseful eventually there.