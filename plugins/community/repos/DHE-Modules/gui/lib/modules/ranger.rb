require_relative '../module'

module DHE
  class Ranger < DHE::Module
    def initialize
      super(name: 'RANGER', hp: 6, foreground: [60, 100, 15], background: [60, 100, 97])
      left = @width / 3.5 + 1.0 / 3.0
      right = @width - left

      y = 24.0
      delta_y = 16.0
      panel_buffer = delta_y + 4.0
      separator_offset = panel_buffer / 2.0 + 2.0

      medium_knob(x: left, y: y, label: 'LEVEL')
      output_port(x: right, y: y)

      y += delta_y
      cv_port(x: left, y: y)
      attenuverter(x: right, y: y)

      2.times do
        y += panel_buffer
        separator(y: y - separator_offset)
        medium_knob(x: left, y: y, label: 'LIMIT')
        polarity_toggle(x: right, y: y)

        y += delta_y
        cv_port(x: left, y: y)
        attenuverter(x: right, y: y)
      end
    end
  end
end

MODULE_TO_FILENAME[DHE::Ranger.new] = __FILE__
