require 'win32/registry'

screen_reg = Win32::Registry::HKEY_CURRENT_USER.create "Software\\avisynth-as-dshow-capture" # LODO .keys fails?
require 'add_vendored_gems_to_load_path.rb' # for swinghelpers' sane
require 'jruby-swing-helpers/swing_helpers'
old = screen_reg['avs_filename_to_read']
old = File.dirname(old) if old
filename = SwingHelpers.new_previously_existing_file_selector_and_go 'select avisynth filename for the directshow capture source to use', old
screen_reg['avs_filename_to_read'] = filename
p 'saved it as ' + filename
screen_reg.close
SwingHelpers.hard_exit # ??
