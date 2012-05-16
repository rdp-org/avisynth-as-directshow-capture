require 'win32/registry'

screen_reg = Win32::Registry::HKEY_CURRENT_USER.create "Software\\avisynth-as-dshow-capture" # LODO .keys fails?
require 'add_vendored_gems_to_load_path.rb' # for swinghelpers' sane
require 'jruby-swing-helpers/swing_helpers'
filename = SwingHelpers.new_nonexisting_filechooser_and_go 'select avisynth filename for the directshow capture source to use'
screen_reg['filename_to_use'] = filename
p 'saved it as ' + filename