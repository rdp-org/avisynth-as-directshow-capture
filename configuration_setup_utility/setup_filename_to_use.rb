require 'win32/registry'

screen_reg = Win32::Registry::HKEY_CURRENT_USER.create "Software\\avisynth-as-dshow-capture"
require 'add_vendored_gems_to_load_path.rb' # for swinghelpers' sane
require 'jruby-swing-helpers/swing_helpers'
old = screen_reg['avs_filename_to_read'] rescue nil # yikes exception handling is *hard* ish in ruby
old = File.dirname(old) if old
filename = SwingHelpers.new_previously_existing_file_selector_and_go 'select avisynth filename for the directshow capture source to use', old
screen_reg['avs_filename_to_read'] = filename
SwingHelpers.show_blocking_message_dialog 'saved it as ' + filename + ' so now when you use it later, it will get input (capture) from that file'
screen_reg.close
